// SPDX-License-Identifier: GPL-2.0
/*
 * vthermal - a virtual thermal zone with a virtual cooling device
 *
 * The zone's temperature comes from a simple heat model instead of a real
 * sensor, so the Linux thermal framework can be exercised on any machine
 * (including a QEMU guest):
 *
 *   target = ambient + heat_load - cooling_state * cool_per_state
 *   temp  += (target - temp) / 64         every 100 ms of elapsed time
 *
 * The slow approach (time constant about 6 s) stands in for thermal mass.
 * It matters for the step_wise governor, which only adds cooling while the
 * temperature is still rising: with a plant that reacts instantly, the
 * temperature would stop rising after the first step and settle above the
 * trip point.
 *
 * The model advances with wall-clock time, not with the number of reads,
 * so reading the temperature from sysfs does not change it.
 *
 * Writing a heat load through debugfs heats the zone up. When the
 * temperature crosses the passive trip point, the step_wise governor raises
 * the cooling device's state, which lowers the target and brings the
 * temperature back down. Everything is visible from user space under
 * /sys/class/thermal/ like a real platform thermal driver.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/debugfs.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/thermal.h>

#define VTHERMAL_NAME		"vthermal"
#define VTHERMAL_MAX_STATE	10
#define VTHERMAL_PASSIVE_DELAY	250	/* ms, polling while cooling */
#define VTHERMAL_POLLING_DELAY	1000	/* ms, polling while idle */
#define VTHERMAL_MODEL_STEP_MS	100

/* All temperatures are in millidegrees Celsius, like the thermal core. */
static int ambient_mc = 35000;
module_param(ambient_mc, int, 0444);
MODULE_PARM_DESC(ambient_mc, "Ambient temperature in m°C (default 35000)");

static int passive_mc = 60000;
module_param(passive_mc, int, 0444);
MODULE_PARM_DESC(passive_mc, "Passive trip point in m°C (default 60000)");

static int critical_mc = 95000;
module_param(critical_mc, int, 0444);
MODULE_PARM_DESC(critical_mc, "Critical trip point in m°C (default 95000)");

static int cool_per_state_mc = 5000;
module_param(cool_per_state_mc, int, 0444);
MODULE_PARM_DESC(cool_per_state_mc, "Cooling per cooling state step in m°C (default 5000)");

enum vthermal_trip_id {
	VTHERMAL_TRIP_PASSIVE,
	VTHERMAL_TRIP_CRITICAL,
	VTHERMAL_NUM_TRIPS,
};

struct vthermal {
	struct mutex lock;		/* protects the model below */
	int temp_mc;
	unsigned long last_update;	/* jiffies of the last model step */
	u32 heat_load_mc;		/* written from debugfs */
	unsigned long cur_state;	/* cooling device state */

	struct thermal_trip trips[VTHERMAL_NUM_TRIPS];
	struct thermal_zone_device *tzd;
	struct thermal_cooling_device *cdev;
	struct dentry *debugfs;
};

/* ---- thermal zone ---------------------------------------------------- */

/* Advance the heat model to the current time. Caller holds vt->lock. */
static void vthermal_update_model(struct vthermal *vt)
{
	unsigned long steps = jiffies_to_msecs(jiffies - vt->last_update) /
			      VTHERMAL_MODEL_STEP_MS;
	int target;

	if (!steps)
		return;
	vt->last_update += msecs_to_jiffies(steps * VTHERMAL_MODEL_STEP_MS);

	target = ambient_mc + (int)vt->heat_load_mc -
		 (int)vt->cur_state * cool_per_state_mc;
	if (target < ambient_mc)
		target = ambient_mc;

	/* Cap the catch-up so a long idle gap cannot loop for long. */
	steps = min(steps, 1000UL);
	while (steps--)
		vt->temp_mc += (target - vt->temp_mc) / 64;
}

static int vthermal_get_temp(struct thermal_zone_device *tzd, int *temp)
{
	struct vthermal *vt = thermal_zone_device_priv(tzd);

	mutex_lock(&vt->lock);
	vthermal_update_model(vt);
	*temp = vt->temp_mc;
	mutex_unlock(&vt->lock);

	return 0;
}

static int vthermal_bind(struct thermal_zone_device *tzd,
			 struct thermal_cooling_device *cdev)
{
	struct vthermal *vt = thermal_zone_device_priv(tzd);

	/* Only bind our own cooling device, and only to the passive trip. */
	if (cdev != vt->cdev)
		return 0;

	return thermal_zone_bind_cooling_device(tzd, VTHERMAL_TRIP_PASSIVE, cdev,
						THERMAL_NO_LIMIT,
						THERMAL_NO_LIMIT,
						THERMAL_WEIGHT_DEFAULT);
}

static int vthermal_unbind(struct thermal_zone_device *tzd,
			   struct thermal_cooling_device *cdev)
{
	struct vthermal *vt = thermal_zone_device_priv(tzd);

	if (cdev != vt->cdev)
		return 0;

	return thermal_zone_unbind_cooling_device(tzd, VTHERMAL_TRIP_PASSIVE,
						  cdev);
}

static void vthermal_critical(struct thermal_zone_device *tzd)
{
	/*
	 * The default handler would shut the machine down. This zone is
	 * virtual, so only report it.
	 */
	dev_emerg(&tzd->device, "critical temperature reached (virtual, not shutting down)\n");
}

static struct thermal_zone_device_ops vthermal_tz_ops = {
	.get_temp = vthermal_get_temp,
	.bind = vthermal_bind,
	.unbind = vthermal_unbind,
	.critical = vthermal_critical,
};

/* ---- cooling device -------------------------------------------------- */

static int vthermal_get_max_state(struct thermal_cooling_device *cdev,
				  unsigned long *state)
{
	*state = VTHERMAL_MAX_STATE;
	return 0;
}

static int vthermal_get_cur_state(struct thermal_cooling_device *cdev,
				  unsigned long *state)
{
	struct vthermal *vt = cdev->devdata;

	mutex_lock(&vt->lock);
	*state = vt->cur_state;
	mutex_unlock(&vt->lock);

	return 0;
}

static int vthermal_set_cur_state(struct thermal_cooling_device *cdev,
				  unsigned long state)
{
	struct vthermal *vt = cdev->devdata;

	if (state > VTHERMAL_MAX_STATE)
		return -EINVAL;

	mutex_lock(&vt->lock);
	vthermal_update_model(vt);
	vt->cur_state = state;
	mutex_unlock(&vt->lock);

	return 0;
}

static const struct thermal_cooling_device_ops vthermal_cdev_ops = {
	.get_max_state = vthermal_get_max_state,
	.get_cur_state = vthermal_get_cur_state,
	.set_cur_state = vthermal_set_cur_state,
};

/* ---- debugfs: heat load input ---------------------------------------- */

static int vthermal_heat_load_get(void *data, u64 *val)
{
	struct vthermal *vt = data;

	mutex_lock(&vt->lock);
	*val = vt->heat_load_mc;
	mutex_unlock(&vt->lock);

	return 0;
}

static int vthermal_heat_load_set(void *data, u64 val)
{
	struct vthermal *vt = data;

	if (val > 100000)
		return -EINVAL;

	mutex_lock(&vt->lock);
	vthermal_update_model(vt);
	vt->heat_load_mc = val;
	mutex_unlock(&vt->lock);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(vthermal_heat_load_fops, vthermal_heat_load_get,
			 vthermal_heat_load_set, "%llu\n");

/* ---- platform driver ------------------------------------------------- */

static int vthermal_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct thermal_zone_params tzp = {
		.governor_name = "step_wise",
		.no_hwmon = true,
	};
	struct vthermal *vt;
	int ret;

	if (passive_mc >= critical_mc) {
		dev_err(dev, "passive trip must be below critical trip\n");
		return -EINVAL;
	}

	vt = devm_kzalloc(dev, sizeof(*vt), GFP_KERNEL);
	if (!vt)
		return -ENOMEM;

	mutex_init(&vt->lock);
	vt->temp_mc = ambient_mc;
	vt->last_update = jiffies;
	vt->trips[VTHERMAL_TRIP_PASSIVE] = (struct thermal_trip) {
		.temperature = passive_mc,
		.hysteresis = 2000,
		.type = THERMAL_TRIP_PASSIVE,
	};
	vt->trips[VTHERMAL_TRIP_CRITICAL] = (struct thermal_trip) {
		.temperature = critical_mc,
		.type = THERMAL_TRIP_CRITICAL,
	};
	platform_set_drvdata(pdev, vt);

	/* Register the cooling device first so the zone can bind it. */
	vt->cdev = thermal_cooling_device_register(VTHERMAL_NAME, vt,
						   &vthermal_cdev_ops);
	if (IS_ERR(vt->cdev))
		return dev_err_probe(dev, PTR_ERR(vt->cdev),
				     "failed to register cooling device\n");

	vt->tzd = thermal_zone_device_register_with_trips(VTHERMAL_NAME,
							  vt->trips,
							  VTHERMAL_NUM_TRIPS,
							  0, vt,
							  &vthermal_tz_ops,
							  &tzp,
							  VTHERMAL_PASSIVE_DELAY,
							  VTHERMAL_POLLING_DELAY);
	if (IS_ERR(vt->tzd)) {
		ret = dev_err_probe(dev, PTR_ERR(vt->tzd),
				    "failed to register thermal zone\n");
		goto err_cdev;
	}

	ret = thermal_zone_device_enable(vt->tzd);
	if (ret) {
		dev_err(dev, "failed to enable thermal zone: %d\n", ret);
		goto err_tzd;
	}

	vt->debugfs = debugfs_create_dir(VTHERMAL_NAME, NULL);
	debugfs_create_file_unsafe("heat_load_mc", 0644, vt->debugfs, vt,
				   &vthermal_heat_load_fops);

	dev_info(dev, "registered %s (passive %d m°C, critical %d m°C)\n",
		 dev_name(&vt->tzd->device), passive_mc, critical_mc);
	return 0;

err_tzd:
	thermal_zone_device_unregister(vt->tzd);
err_cdev:
	thermal_cooling_device_unregister(vt->cdev);
	return ret;
}

static void vthermal_remove(struct platform_device *pdev)
{
	struct vthermal *vt = platform_get_drvdata(pdev);

	debugfs_remove_recursive(vt->debugfs);
	thermal_zone_device_unregister(vt->tzd);
	thermal_cooling_device_unregister(vt->cdev);
	mutex_destroy(&vt->lock);
}

static struct platform_driver vthermal_driver = {
	.probe = vthermal_probe,
	.remove_new = vthermal_remove,
	.driver = {
		.name = VTHERMAL_NAME,
	},
};

/*
 * There is no firmware description (ACPI/DT) for a virtual device, so the
 * module creates its own platform device and the driver core matches it to
 * the driver by name, the same way a real platform device would be matched.
 */
static struct platform_device *vthermal_pdev;

static int __init vthermal_init(void)
{
	int ret;

	ret = platform_driver_register(&vthermal_driver);
	if (ret)
		return ret;

	vthermal_pdev = platform_device_register_simple(VTHERMAL_NAME,
							PLATFORM_DEVID_NONE,
							NULL, 0);
	if (IS_ERR(vthermal_pdev)) {
		platform_driver_unregister(&vthermal_driver);
		return PTR_ERR(vthermal_pdev);
	}

	return 0;
}

static void __exit vthermal_exit(void)
{
	platform_device_unregister(vthermal_pdev);
	platform_driver_unregister(&vthermal_driver);
}

module_init(vthermal_init);
module_exit(vthermal_exit);

MODULE_AUTHOR("Riley Kao <riley.kao.33@gmail.com>");
MODULE_DESCRIPTION("Virtual thermal zone and cooling device for exercising the thermal framework");
MODULE_LICENSE("GPL");
