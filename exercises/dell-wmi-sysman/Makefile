KDIR ?= /lib/modules/$(shell uname -r)/build

obj-$(CONFIG_DELL_WMI_SYSMAN)  += dell-wmi-sysman.o
dell-wmi-sysman-objs := 	sysman.o		\
				enum-attributes.o	\
				int-attributes.o	\
				string-attributes.o	\
				passobj-attributes.o	\
				biosattr-interface.o	\
				passwordattr-interface.o

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules
clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
