# Name of the module
MODULE_NAME := work_queue_task

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


BUILD_DIR := .
SRCS := reader.c writer.c test.c
TARGETS := $(SRCS:%.c=$(BUILD_DIR)/%)

CC := gcc
LIBS := -lm -lpthread

# Default target
all: module $(TARGETS)

# Build the module
module: 
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Clean up
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	-rm -f $(DEV_PATH)
	-rm -f $(TARGETS)

# Install the module
install:
	sudo $(MAKE) -C $(KDIR) M=$(PWD) modules_install
	sudo depmod -a

# Remove the module
uninstall:
	sudo rm -f /lib/modules/$(shell uname -r)/extra/$(MODULE_NAME).ko
	sudo depmod -a


MAJOR_NUM = $(shell cat /proc/devices | grep $(MODULE_NAME) | sed 's/\([0-9]*\).*/\1/')




# Load the module
load:
	sudo insmod $(MODULE_NAME).ko
	@sleep 1  # Give a moment for dmesg to update

# Unload the module
unload:
	sudo rmmod $(MODULE_NAME)


# Show information about the module
info:
	@if [ -f $(MODULE_NAME).ko ]; then \
		sudo modinfo $(MODULE_NAME).ko; \
	else \
		echo "Module file $(MODULE_NAME).ko not found"; \
	fi


$(BUILD_DIR)/%: %.c
	@mkdir -p $(BUILD_DIR)
	gcc $(CFLAGS) $< -o $@ $(LIBS)

# Full setup: build, load, create node
setup: module load $(TARGETS)

# Full cleanup: delete node, unload, clean
teardown: delete_node unload clean 

# Phony targets
.PHONY: all clean install uninstall create_node delete_node load unload check info setup teardown
