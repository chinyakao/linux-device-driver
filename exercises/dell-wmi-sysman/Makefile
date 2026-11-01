PWD := $(shell pwd) 
KVERSION := $(shell uname -r)
KERNEL_DIR = /usr/src/linux-headers-$(KVERSION)/

obj-$(CONFIG_DELL_WMI_SYSMAN)  += dell-wmi-sysman.o
dell-wmi-sysman-objs := 	sysman.o		\
				enum-attributes.o	\
				int-attributes.o	\
				string-attributes.o	\
				passobj-attributes.o	\
				biosattr-interface.o	\
				passwordattr-interface.o

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules
clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
