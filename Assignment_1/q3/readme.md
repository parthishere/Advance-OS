# Educational Rootkit LKM

This project is an **educational rootkit** implemented as a Linux Kernel Module (LKM). It is designed **strictly for educational and research purposes**. Unauthorized use of this software may be illegal and unethical.

## Overview

This rootkit demonstrates advanced Linux kernel programming techniques, including system call hooking, file hiding, and kernel memory manipulation. It is intended to provide insights into kernel internals and security mechanisms.

### Key Features

- System call table hooking (`sys_exit`, `getdents`, `getdents64`)
- File and directory hiding capabilities
- Dynamic symbol resolution using kprobes
- Kernel memory protection bypassing

## Technical Specifications

- **Target OS**: Linux (Kernel version: 4.19-)
- **Language**: C

## Prerequisites

- Linux operating system (Kernel version X.X or higher)
- Root access
- GCC compiler
- Linux kernel headers

## Installation

1. Clone the repository:
   ```
   git clone https://github.com/yourusername/educational-rootkit.git
   cd educational-rootkit
   ```

2. Compile the module:
   ```
   make
   ```

3. Load the module (requires root privileges):
   ```
   sudo insmod rootkit.ko 
   or 
   make setup
   ```

## Usage

Once loaded, the rootkit will:

- Intercept and log `sys_exit` calls
- Hide files/directories containing "secret" in their names
- Modify the behavior of `getdents` and `getdents64` system calls

To unload the module:
```
sudo rmmod rootkit
```

## Code Structure

- `rootkit.c`: Main source file containing the rootkit implementation
- `Makefile`: For compiling the kernel module

Key functions:
- `custom_init`: Module initialization
- `custom_exit`: Module cleanup
- `fake_getdents`: Custom implementation of getdents for file hiding
- `make_rw` / `make_ro`: Functions to manipulate memory protection



## Learning Objectives

- Understanding Linux kernel module development
- Exploring system call mechanisms
- Learning about kernel memory management
- Studying rootkit techniques for better defense strategies


