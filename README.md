# Linux Device Driver

Linux kernel drivers written from scratch, each tested inside a QEMU guest so
nothing experimental is loaded on the host.

| Driver | Subsystem | Status |
|---|---|---|
| [`vthermal/`](vthermal/) | Thermal framework (thermal zone + cooling device, platform driver) | Builds and passes its QEMU test on Ubuntu kernel 6.8 |
| [`exercises/`](exercises/) | Basics: hello module, character device, out-of-tree build of an upstream driver | Learning exercises |

## Testing

```bash
sudo apt install qemu-system-x86 busybox-static cpio \
    linux-image-6.8.0-147-generic linux-headers-6.8.0-147-generic
scripts/qemu-test.sh vthermal 6.8.0-147-generic
```

[`scripts/qemu-test.sh`](scripts/qemu-test.sh) builds the module against the
given kernel's headers, packs it with busybox into an initramfs, boots that
kernel in QEMU, runs `test_<module>.sh`, and exits non-zero if the test fails.
It uses KVM when `/dev/kvm` is available and falls back to emulation otherwise.

## Roadmap

- Port `vthermal` to the newer thermal binding API (6.13+).
- Character device v2: misc device, `ioctl`, `poll`, sysfs attributes, KUnit tests.
- A `firmware_attributes` style sysfs driver, written from scratch.
- CI that builds every module against several kernel versions and runs checkpatch.

Later areas, following the plan from my earlier notes: USB and PCI drivers,
then laptop subsystems (DRM, ALSA, input, backlight) and power management.

My notes on kernel builds, thermal, power, and graphics are
in [client-linux-notes](https://github.com/chinyakao/client-linux-notes).
