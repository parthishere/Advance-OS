# Temperature Driver Module

A Linux kernel module that implements a character device driver with interrupt handling and sysfs interface for temperature monitoring.

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Usage](#usage)
- [Technical Details](#technical-details)
- [API Reference](#api-reference)
- [Troubleshooting](#troubleshooting)

## Overview

This driver implements a character device for temperature monitoring with the following capabilities:
- Character device interface (/dev/temperature_device)
- Sysfs interface for temperature value access
- Interrupt handling (both GPIO and software interrupts)
- Tasklet-based bottom-half processing
- Device class integration

## Features

- **Character Device Operations**
  - Read operation triggers interrupt
  - Write operation for future extensions
  - Open/Close operations with proper logging

- **Interrupt Handling**
  - Software interrupt mode (IRQ 11)
  - Optional GPIO interrupt mode (configurable)
  - Tasklet-based deferred processing

- **Sysfs Interface**
  - Read/Write temperature values
  - Located at `/sys/kernel/temperature_sysfs/temperature_value`

- **Device Class**
  - Integrated with Linux device model
  - Proper device registration and cleanup

## Prerequisites

- Linux kernel headers
- Build essentials (gcc, make)
- Root privileges for module operations

```bash
# Install required packages
sudo apt-get update
sudo apt-get install build-essential linux-headers-$(uname -r)
```

## Installation

1. **Clone the repository**
```bash
git clone [repository-url]
cd temperature-driver
```

2. **Build the module**
```bash
make
```

3. **Install the module**
```bash
sudo insmod temperature_driver.ko
```

4. **Verify installation**
```bash
# Check if module is loaded
lsmod | grep temperature_driver

# Check device creation
ls -l /dev/temperature_device

# Check sysfs entry
ls -l /sys/kernel/temperature_sysfs/
```

## Usage

### Basic Operations

1. **Read from device**
```bash
sudo cat /dev/temperature_device
```

2. **Write to device**
```bash
echo "test" | sudo tee /dev/temperature_device
```

3. **Access sysfs interface**
```bash
# Read temperature value
cat /sys/kernel/temperature_sysfs/temperature_value

# Write temperature value
echo "25" | sudo tee /sys/kernel/temperature_sysfs/temperature_value
```

### Interrupt Testing

1. **Software Interrupt Mode**
- Reading from device triggers IRQ 11
- Check kernel logs for interrupt processing

2. **GPIO Interrupt Mode** (if enabled)
- Configure with GPIO_INTERRUPT=1
- Triggers on GPIO pin rising edge

## Technical Details

### Driver Architecture

1. **Character Device Component**
```c
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = temperature_dev_read,
    .write = temperature_dev_write,
    .open = temperature_dev_open,
    .release = temperature_dev_release,
};
```

2. **Interrupt Handling**
```c
static irqreturn_t irq_handler(int irq, void *dev_id) {
    tasklet_schedule(&my_tasklet_struct);
    return IRQ_HANDLED;
}
```

3. **Sysfs Interface**
```c
static struct kobj_attribute temperature_sysfs_attr = 
    __ATTR(temperature_value, 0660, sysfs_show, sysfs_store);
```

### Memory Model

- Character device major number: Dynamic allocation
- Temperature value: Static integer storage
- Tasklet: Static declaration

### Resource Management

- Proper cleanup in module exit
- Error handling with goto labels
- Resource release in reverse order

## API Reference

### Character Device Interface

- **Open**: Initialize device
- **Read**: Trigger interrupt
- **Write**: Store data (placeholder)
- **Release**: Cleanup resources

### Sysfs Interface

- **Show**: Read temperature value
- **Store**: Write temperature value

## Troubleshooting

### Common Issues

1. **Module Loading Fails**
- Check kernel logs: `dmesg | tail`

2. **Interrupt Not Triggering**
- BIOS setting for IOMMU

### Debug Commands

```bash
# Check module status
sudo dmesg -w

# Monitor interrupts
cat /proc/interrupts | grep temperature

# Check device permissions
ls -l /dev/temperature_device
```


---
Created by: Parth Thakkar  
Version: 1.0