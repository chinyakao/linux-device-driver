# Kernel Driver

Small kernel module exercises. Larger driver projects written from scratch live in
[linux-device-driver](https://github.com/chinyakao/linux-device-driver).

## Status

| Folder | Status | Notes |
|---|---|---|
| [`hello/`](hello/) | Done | Minimal loadable module. |
| [`char_device/`](char_device/) | Done | Character device with read/write and a user space test program. |
| [`dell-wmi-sysman/`](dell-wmi-sysman/) | Done (study) | Upstream driver rebuilt out of tree to study and add debug output. Not my code; see its README. |
| [`usb_device/`](usb_device/) | Planned | Not started. |
| [`pci_device/`](pci_device/) | Planned | Not started. |

New exercises start from [`README.template`](README.template).

## Planned structure
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
