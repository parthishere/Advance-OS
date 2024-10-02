# Name of the module
MODULE_NAME := chardev

# Object file(s) to be built
obj-m := $(MODULE_NAME).o

# If the module consists of multiple source files,
# $(MODULE_NAME)-objs := file1.o file2.o file3.o

# Kernel source directory
KDIR := /lib/modules/$(shell uname -r)/build

# Current directory
PWD := $(shell pwd)

# Default target
all: module

# Build the module
module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Clean up
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

# Install the module
install:
	sudo $(MAKE) -C $(KDIR) M=$(PWD) modules_install
	sudo depmod -a

# Remove the module
uninstall:
	sudo rm -f /lib/modules/$(shell uname -r)/extra/$(MODULE_NAME).ko
	sudo depmod -a

create_node:
	sudo mknod /dev/chardev c 508 0
	sudo chmod 666 /dev/chardev

# Load the module
load:
	sudo insmod $(MODULE_NAME).ko

# Unload the module
unload:
	sudo rmmod $(MODULE_NAME)

# Check if module is loaded
check:
	sudo lsmod | grep $(MODULE_NAME)

# Show information about the module
info:
	sudo modinfo $(MODULE_NAME).ko

# Phony targets
.PHONY: all clean install uninstall load unload check info