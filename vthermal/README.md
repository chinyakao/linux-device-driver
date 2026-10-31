# vthermal - Virtual Thermal Zone and Cooling Device

A platform driver that registers a thermal zone and a cooling device with the
Linux thermal framework. The temperature comes from a small heat model instead
of a sensor, so the thermal core and its governors can be exercised on any
machine, including a QEMU guest.

## How it works

```
 debugfs heat_load_mc ──► heat model ──► get_temp() ──► thermal core (polls zone)
                              ▲                              │
                              │                     step_wise governor
                              │                              │
                         cur_state ◄──── set_cur_state() ◄───┘
```

- **Heat model:** `target = ambient + heat_load - cur_state × cool_per_state`.
  The temperature moves 1/64 of the way to the target every 100 ms (time
  constant about 6 s), standing in for thermal mass. It advances with time, not
  with the number of reads, so reading `temp` from sysfs does not change it.
- **Trips:** passive at 60 °C (bound to the cooling device), critical at 95 °C.
  The critical handler only logs, because the default would power the machine off.
- **Cooling device:** 10 states, each worth 5 °C of cooling.
- **Governor:** `step_wise`; polling every 250 ms while cooling, every 1 s otherwise.
- **Device model:** there is no ACPI or device tree node for a virtual device, so
  the module creates a platform device and the driver core matches it to the
  driver by name, like a real platform device.

Ambient temperature, both trip points, and cooling per state are module
parameters (`modinfo vthermal.ko`).

## Build and test

```bash
make                     # against the running kernel
make KDIR=/usr/src/linux-headers-6.8.0-147-generic
make check               # checkpatch.pl
../scripts/qemu-test.sh vthermal 6.8.0-147-generic
```

On a test machine (not your daily one) you can also run
`sudo ./test_vthermal.sh` directly. The zone shows up under
`/sys/class/thermal/` with type `vthermal`, so
[`thermal_stress_test.sh`](https://github.com/chinyakao/linux_training/blob/main/thermal_stress_test.sh)
records it alongside the real zones.

The test applies a 60 °C heat load for 30 s, then removes it, and checks that:

1. the cooling device was activated,
2. the zone stayed at least 15 °C below the 95 °C it would reach uncooled, and
3. the cooling state returned to 0 after the load was removed.

Result on Ubuntu kernel 6.8.0-147-generic in QEMU:

```
sec temp_mC cur_state
3 58164 0
4 63863 1
5 66100 5
...
30 67557 6
max temp: 67557  final temp: 67557  max cooling state: 6
load removed, waiting for cooling to be released
cooling released after 2s at 58739 m°C
PASS
```

## What I learned from the first runs

The first version of the test expected the temperature to come back to the
60 °C trip point. It never did: it settled around 67 °C with the cooling state
bouncing between 5 and 6. Turning on the thermal core's debug messages
(`dyndbg="file drivers/thermal/* +p"`) showed why. In this kernel `step_wise`
only looks at the trend while above the trip: one state up while the
temperature is rising, one state down while it is falling. It finds the level
where the temperature stops rising, not the level that brings it back to the
trip point. The test now checks that cooling made a clear difference instead.

Two model bugs showed up the same way:

- In the first model the temperature stepped on every `get_temp()` call, so
  reading sysfs heated or cooled the zone. It now steps with elapsed time.
- With a near-instant model, one cooling step was enough to make the
  temperature fall, so `step_wise` never added more. Adding thermal mass (the
  slow approach) fixed it.

## Kernel compatibility

Written and tested against the 6.8 thermal API
(`thermal_zone_device_register_with_trips()` with a trip mask, `.bind` /
`.unbind` callbacks, `.remove_new`). Later kernels changed the binding and
registration API, so building on a newer kernel needs a port. That is the next
step.
