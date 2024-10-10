# Name of the module
MODULE_NAME := kprobe_at_read

# Object file(s) to be built
obj-m := $(MODULE_NAME).o

# Kernel source directory
KDIR := /lib/modules/$(shell uname -r)/build

# Current directory
PWD := $(shell pwd)

# Default target
all: module load

# Build the module
module: 
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Clean up
clean:
	- sudo rmmod $(MODULE_NAME)
	- sudo rm -f /lib/modules/$(shell uname -r)/extra/$(MODULE_NAME).ko
	- sudo depmod -a
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	-rm -f $(TARGETS)

# Install the module
install:
	- sudo $(MAKE) -C $(KDIR) M=$(PWD) modules_install
	- sudo depmod -a


# Load the module
load:
	- sudo insmod $(MODULE_NAME).ko
	@sleep 1  # Give a moment for dmesg to update

# Unload the module
unload:
	- sudo rmmod $(MODULE_NAME)



