obj-m += vthermal.o

KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean

check:
	$(KDIR)/scripts/checkpatch.pl --no-tree -f vthermal.c
