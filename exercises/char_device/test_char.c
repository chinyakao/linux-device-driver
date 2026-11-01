#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEVFILE "/dev/virt_char"

int main() {
    int fd = open(DEVFILE, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    const char *msg = "Hello from user space!";
    write(fd, msg, strlen(msg));

    char buf[128] = {0};
    lseek(fd, 0, SEEK_SET);
    read(fd, buf, sizeof(buf));
    printf("Read from device: %s\n", buf);

    close(fd);
    return 0;
}
