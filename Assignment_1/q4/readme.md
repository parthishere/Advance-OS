# Kprobe-based Kernel Instrumentation LKM

## Overview

This Linux Kernel Module (LKM) demonstrates the use of kprobes for kernel function instrumentation and debugging. It provides a powerful tool for kernel developers and researchers to gain insights into kernel function behavior, particularly focusing on the `kernel_clone` function involved in the fork system call.

## Features

- Instruments the `kernel_clone` function (easily adaptable to other kernel functions)
- Captures and displays register states before and after function execution
- Prints stack traces for comprehensive debugging
- Demonstrates use of pre and post handlers in kprobes

## Technical Specifications

- **Target OS**: Linux (Kernel version: 5.x+)
- **Key Kernel Feature**: Kprobes

## Prerequisites

- Linux operating system (Kernel version 5.x or higher kprobes are added in later)


## Installation

1. Compile the module:
   ```
   make
   ```

2. Load the module (requires root privileges):
   ```
   sudo insmod kprobe_instrumentation.ko
   make setup 
   make load
   ```
3. Cleanup
    ```
    make clean
    make teardown
    make unload
    ```
## Usage

Once loaded, the module will automatically instrument the `kernel_clone` function. To observe its output:

1. Monitor kernel messages:
   ```
   sudo dmesg -w
   ```

2. Trigger fork operations (e.g., run programs, open new terminals)

3. Observe the detailed output in the kernel log

To unload the module:
```
sudo rmmod kprobe_instrumentation
make unload
make teardown
```

## Code Structure

- `kprobe_instrumentation.c`: Main source file containing the LKM implementation
- `Makefile`: For compiling the kernel module

Key functions:
- `pre_handler`: Executes before the probed instruction
- `post_handler`: Executes after the probed instruction
- `kprobe_init`: Module initialization
- `kprobe_exit`: Module cleanup

## Customization

To instrument a different kernel function:
1. Modify the `symbol_name` in the `my_kprobe` structure
2. Adjust the handlers if necessary to capture function-specific information

## Learning Objectives

- Understanding Linux kernel module development
- Exploring kernel function instrumentation techniques
- Analyzing kernel function behavior and performance
- Gaining insights into the fork system call implementation



