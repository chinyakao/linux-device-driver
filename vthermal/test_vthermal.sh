#!/bin/sh
# Load vthermal, apply a heat load, and check that the step_wise governor
#   1. raises the cooling state once the passive trip is crossed,
#   2. keeps the temperature well below where it would settle uncooled, and
#   3. releases all cooling after the load is removed.
#
# step_wise reacts to the temperature trend: +1 state while rising, -1 while
# falling (in the 6.8 Ubuntu kernel). It settles where the trend flips, which
# can be above the trip point, so the check is "cooling made a difference",
# not "temperature returned to the trip point".
#
# Usage: sudo ./test_vthermal.sh [path/to/vthermal.ko]
# Works with bash or busybox sh (used in the QEMU test).

set -e

KO="${1:-./vthermal.ko}"
HEAT_LOAD_MC=60000	# ambient 35 °C + 60 °C load = 95 °C without cooling
UNCOOLED_MC=95000
MIN_COOLING_EFFECT_MC=15000
SAMPLES=30
RELEASE_TIMEOUT=30

fail() { echo "FAIL: $*"; exit 1; }

[ "$(id -u)" -eq 0 ] || fail "run as root"
grep -q " /sys/kernel/debug " /proc/mounts || mount -t debugfs none /sys/kernel/debug

insmod "$KO"
trap 'rmmod vthermal 2>/dev/null || true' EXIT

TZ=""
for z in /sys/class/thermal/thermal_zone*; do
	[ "$(cat "$z/type")" = "vthermal" ] && TZ="$z"
done
CD=""
for c in /sys/class/thermal/cooling_device*; do
	[ "$(cat "$c/type")" = "vthermal" ] && CD="$c"
done
[ -n "$TZ" ] || fail "thermal zone not found"
[ -n "$CD" ] || fail "cooling device not found"

PASSIVE=$(cat "$TZ/trip_point_0_temp")
echo "zone: $TZ  cooling: $CD  policy: $(cat "$TZ/policy")  passive trip: $PASSIVE"

echo "$HEAT_LOAD_MC" > /sys/kernel/debug/vthermal/heat_load_mc

MAX_TEMP=0
MAX_STATE=0
echo "sec temp_mC cur_state"
i=0
while [ "$i" -lt "$SAMPLES" ]; do
	sleep 1
	i=$((i + 1))
	T=$(cat "$TZ/temp")
	S=$(cat "$CD/cur_state")
	echo "$i $T $S"
	[ "$T" -gt "$MAX_TEMP" ] && MAX_TEMP=$T
	[ "$S" -gt "$MAX_STATE" ] && MAX_STATE=$S
done
FINAL_TEMP=$T

echo "max temp: $MAX_TEMP  final temp: $FINAL_TEMP  max cooling state: $MAX_STATE"
[ "$MAX_STATE" -gt 0 ] || fail "cooling device was never activated"
[ "$MAX_TEMP" -lt $((UNCOOLED_MC - MIN_COOLING_EFFECT_MC)) ] || \
	fail "cooling kept the zone less than $MIN_COOLING_EFFECT_MC m°C below the uncooled temperature"

echo "load removed, waiting for cooling to be released"
echo 0 > /sys/kernel/debug/vthermal/heat_load_mc
i=0
while [ "$(cat "$CD/cur_state")" -gt 0 ]; do
	[ "$i" -lt "$RELEASE_TIMEOUT" ] || fail "cooling state still $(cat "$CD/cur_state") after ${RELEASE_TIMEOUT}s"
	sleep 1
	i=$((i + 1))
done
echo "cooling released after ${i}s at $(cat "$TZ/temp") m°C"
echo "PASS"
