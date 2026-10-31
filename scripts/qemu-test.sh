#!/bin/bash
# Build a module and run its test inside a throwaway QEMU guest, so no
# out-of-tree code is loaded on the host kernel.
#
# Usage: scripts/qemu-test.sh <module-dir> [kernel-version]
#   e.g. scripts/qemu-test.sh vthermal 6.8.0-147-generic
#
# Needs (Ubuntu): qemu-system-x86 busybox-static cpio
#                 linux-image-<ver> linux-headers-<ver>
# The module dir must contain a Makefile and test_<module>.sh.

set -euo pipefail

MOD_DIR="$(realpath "${1:?usage: $0 <module-dir> [kernel-version]}")"
KVER="${2:-$(uname -r)}"
MOD="$(basename "$MOD_DIR")"
KERNEL="/boot/vmlinuz-$KVER"
KDIR="/usr/src/linux-headers-$KVER"

[ -r "$KERNEL" ] || { echo "missing $KERNEL (install linux-image-$KVER)"; exit 1; }
[ -d "$KDIR" ] || { echo "missing $KDIR (install linux-headers-$KVER)"; exit 1; }

make -C "$MOD_DIR" KDIR="$KDIR"

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
ROOT="$WORK/root"
mkdir -p "$ROOT"/{bin,proc,sys,dev,tmp}
cp "$(command -v busybox)" "$ROOT/bin/"
for a in sh mount insmod rmmod cat grep sleep echo id poweroff dmesg; do
	ln -s busybox "$ROOT/bin/$a"
done
cp "$MOD_DIR/$MOD.ko" "$MOD_DIR/test_$MOD.sh" "$ROOT/"

cat > "$ROOT/init" <<INIT
#!/bin/sh
mount -t proc none /proc
mount -t sysfs none /sys
mount -t devtmpfs none /dev
echo "=== kernel \$(cat /proc/sys/kernel/osrelease)"
if sh /test_$MOD.sh /$MOD.ko; then echo "=== RESULT: PASS"; else echo "=== RESULT: FAIL"; fi
dmesg | grep -iE "$MOD|bug|warn|oops"
poweroff -f
INIT
chmod +x "$ROOT/init"
(cd "$ROOT" && find . | cpio -o -H newc 2>/dev/null | gzip) > "$WORK/initrd.gz"

ACCEL=()
[ -w /dev/kvm ] && ACCEL=(-enable-kvm -cpu host)

qemu-system-x86_64 "${ACCEL[@]}" -m 512 -nographic -no-reboot \
	-kernel "$KERNEL" -initrd "$WORK/initrd.gz" \
	-append "console=ttyS0 quiet panic=-1" | tee "$WORK/console.log" | sed -n '/=== kernel/,$p'

grep -q "=== RESULT: PASS" "$WORK/console.log"
