## Project structure
```
- your-linux-distro
    - linux-6.0.10
    - arch
    -... all linux folders
    - vfs (create folder called vfs)
        - hello.c (create hello program)
        - root.cpio.gz (this is virtual file system)
```

## Install build dependencies
```
sudo apt-get install git fakeroot build-essential ncurses-dev xz-utils libssl-dev bc flex libelf-dev bison
```

## Download Linux's source code
```
wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.0.10.tar.xz
tar xvf linux-6.0.10.tar.xz
cd linux-6.0.10
```

## Check the downloaded package

## Create a new folder
create the folder "vfs" and the file "hello-kernel.c" under the project structure 

```
- linux-6.0.10
    - vfs
        - hello-kernel.c
```

put the following code in the "hello-kernel.c"

```
#include <stdio.h>

int main() {
    printf("Hello, Kernel!\n");
    sleep(99999999999999999);
}
```

## Compile hello kernel
```
gcc --static hello-kernel.c -o init
```

## Create virtual file system
```
find . | cpio -o -H newc | gzip > root.cpio.gz
```

## Boot kernel (assuming above project structure)
```
qemu-system-x86_64 -nographic -no-reboot -kernel arch/x86/boot/bzImage -initrd vfs/root.cpio.gz -append "panic=1 console=ttyS0"
```

###### REF
- https://www.kernel.org/
- https://github.com/maksimKorzh/cmk-linux
- https://www.youtube.com/watch?v=1gEFYoGUFxM
