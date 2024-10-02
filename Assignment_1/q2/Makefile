# Name of the module
MODULE_NAME := chardev

# Object file(s) to be built
obj-m := $(MODULE_NAME).o

# Kernel source directory
KDIR := /lib/modules/$(shell uname -r)/build

# Current directory
PWD := $(shell pwd)

# Device file details
DEV_NAME := chardev
MINOR_NUM := 0
DEV_PATH := /dev/$(DEV_NAME)

# Default target
all: module

# Build the module
module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Clean up
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f $(DEV_PATH)

# Install the module
install:
	sudo $(MAKE) -C $(KDIR) M=$(PWD) modules_install
	sudo depmod -a

# Remove the module
uninstall:
	sudo rm -f /lib/modules/$(shell uname -r)/extra/$(MODULE_NAME).ko
	sudo depmod -a


MAJOR_NUM = $(shell cat /proc/devices | grep $(MODULE_NAME) | sed 's/\([0-9]*\).*/\1/')


# Example usage in create_node target
create_node:
	if [ ! -e $(DEV_PATH) ]; then \
		sudo mknod $(DEV_PATH) c $(MAJOR_NUM) $(MINOR_NUM); \
		sudo chmod 666 $(DEV_PATH); \
		echo "Device node created: $(DEV_PATH) with major number $(MAJOR_NUM)"; \
	else \
		echo "Device node already exists: $(DEV_PATH)"; \
	fi

# Check if module is loaded and get major number
check:
	@if lsmod | grep -q $(MODULE_NAME); then \
		echo "Module $(MODULE_NAME) is loaded"; \
		MAJOR_NUM=$$($(call get_major_num)); \
		if [ -n "$$MAJOR_NUM" ]; then \
			echo "Major number: $$MAJOR_NUM"; \
		else \
			echo "Major number not found in /proc/devices"; \
		fi; \
	else \
		echo "Module $(MODULE_NAME) is not loaded"; \
	fi

# Load the module
load:
	sudo insmod $(MODULE_NAME).ko
	@sleep 1  # Give a moment for dmesg to update

# Unload the module
unload:
	sudo rmmod $(MODULE_NAME)

# Check if module is loaded
check:
	@if lsmod | grep -q $(MODULE_NAME); then \
		echo "Module $(MODULE_NAME) is loaded"; \
		MAJOR_NUM=$$($(call get_major_num)); \
		if [ -n "$$MAJOR_NUM" ]; then \
			echo "Major number: $$MAJOR_NUM"; \
		else \
			echo "Major number not found in dmesg"; \
		fi; \
	else \
		echo "Module $(MODULE_NAME) is not loaded"; \
	fi

# Show information about the module
info:
	@if [ -f $(MODULE_NAME).ko ]; then \
		sudo modinfo $(MODULE_NAME).ko; \
	else \
		echo "Module file $(MODULE_NAME).ko not found"; \
	fi

# Full setup: build, load, create node
setup: module load create_node

# Full cleanup: delete node, unload, clean
teardown: delete_node unload clean

# Phony targets
.PHONY: all clean install uninstall create_node delete_node load unload check info setup teardown