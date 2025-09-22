#!/bin/bash
set -e

sudo insmod virt_char.ko
major=$(dmesg | grep "virt_char loaded" | tail -1 | grep -oP 'major \K[0-9]+')

sudo mknod /dev/virt_char c $major 0
sudo chmod 666 /dev/virt_char

./test_char

sudo rmmod virt_char
sudo rm /dev/virt_char
