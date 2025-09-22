obj-m += virt_char.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules
	gcc test_char.c -o test_char

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f test_char
