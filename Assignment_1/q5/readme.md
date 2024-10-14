# Educational Rootkit LKM for latest kernel

## Overview

This Linux Kernel Module demonstrates advanced kernel programming techniques, including system call hooking, file hiding, and kernel memory manipulation. It is intended to provide insights into kernel internals, system call mechanisms, and rootkit techniques for educational purposes.

## Features

- System call table hooking (`sys_exit`)
- File and directory hiding capabilities (via `getdents` hook)
- Dynamic symbol resolution using kprobes
- Kernel memory protection bypassing
- Demonstration of kernel-level file operations

## Technical Specifications

- **Target OS**: Linux (Kernel version: 5.x+)

## Prerequisites

- Linux operating system (Kernel version 5.x or higher recommended)
- Root access
- GCC compiler
- Linux kernel headers

## Installation

1. Compile the module:
   ```
   make
   ```

2. Load the module (requires root privileges):
   ```
   sudo insmod rootkit.ko
   make load 
   make setup
   ```

## Usage

Once loaded, the rootkit will:

- Intercept and log `sys_exit` calls
- Provide capability to hide files/directories containing "secret" in their names (when uncommented)
- Modify the behavior of certain system calls

To unload the module:
```
sudo rmmod rootkit
make unload
make teardown
```

## Code Structure

- `rootkit.c`: Main source file containing the rootkit implementation
- `Makefile`: For compiling the kernel module

Key functions:
- `custom_init`: Module initialization and system call hooking
- `custom_exit`: Module cleanup and restoring original system calls
- `fake_getdents`: Custom implementation for file hiding
- `CR0_WRITE_UNLOCK`: Macro for bypassing kernel memory write protection

## Customization

To modify the rootkit's behavior:
1. Edit the `HIDE_FILE` macro to change the file hiding pattern
2. Uncomment the `getdents` and `