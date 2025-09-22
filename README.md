# Kerenl Drvier
## Structure
```
kernel-driver/
├── foundation/
│   ├── kernel_modules/
│   ├── char_driver_intro/
│   ├── char_driver_ioctl/
│   ├── usb_driver/
│   ├── pci_driver/
│   ├── block_driver/
│   ├── net_driver/
│   └── ...
├── laptop_devices/
│   ├── graphics_drm/
│   ├── audio_alsa/
│   ├── input_evdev/
│   ├── backlight/
│   └── ...
├── advanced_features/
│   ├── wireless_mac80211/
│   ├── camera_v4l2/
│   ├── power_management/
│   ├── firmware_loading/
│   └── ...
├── common_libs_and_tools/
│   ├── test_utils/
│   ├── tracing_scripts/
│   └── qemu_configs/
└── README.md

```
---

### Week 1: Kernel Module Basics

- [ ]  Install kernel headers and toolchains
- [ ]  Write and compile a hello_world module
- [ ]  Load/unload with `insmod` and `rmmod`
- [ ]  Inspect logs via `dmesg`
- [ ]  Exercise: Modify the message output

### Week 2: Char Driver Intro

- [ ]  Implement a basic char device using `register_chrdev`
- [ ]  Implement `open`, `read`, `write`
- [ ]  Understand user/kernel buffer operations
- [ ]  Exercise: Write a user-space test program

### Week 3: Char Driver with IOCTL

- [ ]  Implement ioctl handling in driver
- [ ]  Define command macros with `_IO`, `_IOR`, `_IOW`
- [ ]  Exercise: Create user app to send/receive data via ioctl

### Week 4: USB Driver

- [ ]  Use `usb_driver` and `usb_device_id`
- [ ]  Implement `probe` and `disconnect`
- [ ]  Send/receive USB bulk messages
- [ ]  Exercise: Detect specific USB device via your driver

### Week 5: PCI Driver

- [ ]  Use `pci_register_driver`
- [ ]  Map BARs using `ioremap`
- [ ]  Perform read/write with `readl/writel`
- [ ]  Exercise: Simulate a PCI device using QEMU

### Week 6: Block Driver

- [ ]  Implement a virtual block device
- [ ]  Understand `request_queue` and `gendisk`
- [ ]  Format and mount the device using `mkfs`, `mount`
- [ ]  Exercise: Create a RAM disk module

### Week 7: Net Driver

- [ ]  Register `net_device`
- [ ]  Implement `ndo_start_xmit`
- [ ]  Use `ping` or `tcpdump` to test packets
- [ ]  Exercise: Build a loopback-like net driver
