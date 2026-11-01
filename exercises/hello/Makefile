KDIR ?= /lib/modules/$(shell uname -r)/build

MODULE_NAME = hello
obj-m := $(MODULE_NAME).o

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules
clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
