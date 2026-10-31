# Linux Device Driver

Linux kernel drivers written from scratch, each tested inside a QEMU guest so
nothing experimental is loaded on the host.

| Driver | Subsystem | Status |
|---|---|---|
| [`vthermal/`](vthermal/) | Thermal framework (thermal zone + cooling device, platform driver) | Builds and passes its QEMU test on Ubuntu kernel 6.8 |

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

Exercises and notes that came before this repo are in
[linux_training](https://github.com/chinyakao/linux_training).
