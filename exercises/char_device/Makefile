obj-m += virt_char.o

KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules
	gcc test_char.c -o test_char

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
	rm -f test_char
