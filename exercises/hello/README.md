# Build Kernel Module - hello
Kernel version: Ubuntu OEM 5.14 (x86_64)
## Create Module - hello.c
## Compile Module - Makefile
After creating the Makefile, compile it in terminal

```
make
```

It will create following files

```
hello.c
.hello.ko.cmd
hello.mod.c
hello.mod.o
hello.o
Makefile
.modules.order.cmd
.Module.symvers.cmd
hello.ko
hello.mod
.hello.mod.cmd
.hello.mod.o.cmd
hello.o.cmd
modules.order
Module.symvers
```

## Install Module
```
sudo insmod hello.ko
```

## Check Module Installation
```
sudo lsmod | grep "hello"
```

It will show

```
hello 16384 0
```

## Remove Module
```
sudo rmmod hello.ko
```

## Check Sys Log
```
dmesg
```

It will show

```
[ 3866.709249] Hello world !
[ 3887.090724] Bye !
```

###### REF
- https://jerrynest.io/how-to-write-a-linux-kernel-module/