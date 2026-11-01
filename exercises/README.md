# Exercises

Small kernel module exercises from when I started learning driver development.
They were moved here from
[linux-platform-enablement](https://github.com/chinyakao/linux-platform-enablement) with their git
history. Both build against Ubuntu kernel 6.8 headers:

```bash
make -C <dir> KDIR=/usr/src/linux-headers-6.8.0-147-generic
```

| Folder | What it is |
|---|---|
| [`hello/`](hello/) | Minimal loadable module. |
| [`char_device/`](char_device/) | Character device with read/write and a user space test program (`run.sh`). |

New exercises start from [`README.template`](README.template).
