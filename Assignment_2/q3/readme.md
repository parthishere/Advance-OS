# installing seccomp by
sudo apt install libseccomp-dev
# Secure Program Execution Sandbox
Advanced Operating Systems Assignment 2

## Overview
This project implements a secure execution environment using chroot and seccomp to safely run untrusted programs. The sandbox provides restricted filesystem access and limited system call capabilities, demonstrating fundamental security isolation techniques.

## Components

### 1. File Access Program (`userspace.c`)
- Simple program demonstrating file operations
- Features:
  - File reading and display
  - File content appending
  - Error handling

### 2. Security Implementation
Three different approaches for system call filtering:
1. Basic Seccomp (prctl)
2. Libseccomp API
3. eBPF Filters

## Technical Implementation

### 1. Basic File Operations
```c
// Example of allowed operations
test_file_fd = open("test.txt", 0666);
read(test_file_fd, &ch, 1);
write(test_file_fd, buffer_to_write, 99);
```

### 2. Seccomp Filtering Methods

#### a) Basic Seccomp (Mode Strict)
```c
void normal_seccomp() {
    prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT);
}
```
- Allows only `read()`, `write()`, `exit()`
- Simplest implementation
- No customization options

#### b) Libseccomp Implementation
```c
void libseccomp_setup(int fd) {
    ctx = seccomp_init(SCMP_ACT_KILL);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), ...);
    seccomp_load(ctx);
}
```
- Fine-grained control
- Easier to maintain
- Comprehensive rule system

#### c) eBPF Implementation
```c
struct sock_filter filter[] = {
    BPF_STMT(BPF_LD | BPF_W | BPF_ABS, ...),
    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, ...),
    ...
};
```
- Low-level control
- Custom filter rules
- Performance optimized

## Security Features

### 1. Filesystem Restrictions
- Limited access to specific files
- Read-only access where possible
- Controlled write permissions

### 2. System Call Filtering
- Whitelist approach
- Only essential calls allowed
- Size-based write restrictions

### 3. Process Isolation
- No new privileges
- Limited resource access
- Controlled execution environment

## Installation

### Prerequisites
```bash
# Required packages
sudo apt-get install gcc
sudo apt-get install libseccomp-dev
```

### Building
```bash
# Compile the program
gcc -o sandbox_program userspace.c -lseccomp

# Compile seccomp filter
gcc -o seccomp_filter seccomp_filter.c -lseccomp
```

## Usage

### 1. Basic Program Execution
```bash
./sandbox_program 1 1  # Run with basic seccomp
./sandbox_program 1 2  # Run with libseccomp
./sandbox_program 1 3  # Run with eBPF
```

### 2. Testing Different Scenarios
```bash
# Test unauthorized syscall
./sandbox_program 1 <filter_type>

# Test file creation
./sandbox_program 2 <filter_type>

# Test accessing restricted file
./sandbox_program 3 <filter_type>
```

## Testing Results

### 1. Unauthorized System Call
- **Expected**: Process termination
- **Result**: SECCOMP_RET_KILL
- **Log**: Kernel audit log entry

### 2. File Creation Attempt
- **Expected**: Permission denied
- **Result**: Operation not permitted
- **Error**: EPERM

### 3. Restricted File Access
- **Expected**: Access denied
- **Result**: No such file or directory
- **Error**: ENOENT

## Implementation Details

### 1. File Operations
- Open file descriptor tracking
- Buffer size limitations
- Error handling implementation

### 2. Seccomp Rules
```c
// Write size restriction
BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
         (offsetof(struct seccomp_data, args[2])));
BPF_JUMP(BPF_JMP | BPF_JGE | BPF_K, 100, 0, 1);
```

### 3. Security Measures
- Architecture validation
- Privilege dropping
- Resource limiting


### Directory Structure
```
.
├── src/
│   ├── userspace.c
│   └── seccomp_filter.c
├── include/
│   └── headers/
├── tests/
├── docs/
└── README.md
```

For detailed implementation questions or issues contact @: ParthRajeshkumar.Thakkar@colorado.edu.
