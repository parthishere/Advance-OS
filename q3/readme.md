# Virtual Memory Layout Analysis in Linux Processes

## Project Overview

1. Parent process
2. Child process before and after exec()
3. Independent execution of the sample program

Two main files are used for this analysis:

1. `parent_child_vml.c`: Demonstrates parent-child process memory layouts (output for parent process is redirected in output.txt)
2. `sample_program.c`: Used as the target for exec() and for independent execution(memory map for this process running individually is provided in address_space_for_child_separate)

## File Descriptions

### 1. parent_child_vml.c

This file contains the main program that:
- Prints its own (parent) memory map
- Creates a child process using fork()
- In the child process:
  - Prints the child's memory map before exec()
  - Executes sample_program using exec()
- Waits for the child process to complete

### 2. sample_program.c

A simple program that:
- Is executed by the child process after exec()
- Is run independently to compare memory layouts

## Key Questions Addressed

1. How does the virtual memory layout of the child process compare to the virtual memory layout of the parent process before the child process executes exec()?
2. How does the virtual memory layout of the child process compare to the virtual memory layout of another process that runs the other C program from the command line?

## Detailed Observations

### 1. Parent vs Child before exec()

- Memory layouts are nearly identical:
  - Main program: 0x6003b6dd3000 - 0x6003b6dd8000 (20 KB)
  - Heap: 0x6003b7c2e000 - 0x6003b7c4f000 (135 KB)
  - Shared libraries (e.g., libc.so.6): 0x782abd000000 - 0x782abd212000 (2.1 MB)
  - Stack: 0x7ffd87184000 - 0x7ffd871a5000 (135 KB)
- Special segments ([vvar], [vdso], [vsyscall]) have identical addresses
- This similarity is due to fork() creating a copy-on-write duplicate of the parent's address space

### 2. Child before exec() vs Child after exec()

- Main program changes:
  - Before: 0x6003b6dd3000 - 0x6003b6dd8000
  - After:  0x61f37cb75000 - 0x61f37cb7a000
- Heap segment changes:
  - Before: 0x6003b7c2e000 - 0x6003b7c4f000
  - After:  0x61f37d596000 - 0x61f37d5b7000
- Shared libraries loaded at different addresses:
  - libc.so.6 before: 0x782abd000000 - 0x782abd212000
  - libc.so.6 after:  0x7449b4600000 - 0x7449b4812000
- Stack segment changes:
  - Before: 0x7ffd87184000 - 0x7ffd871a5000
  - After:  0x7ffc72b4a000 - 0x7ffc72b6b000
- Overall structure (order of segments) remains similar
- Total size of segments remains consistent (e.g., main program remains 20 KB)

### 3. Child after exec() vs Sample program running independently

- Main program addresses differ:
  - Exec'd child: 0x61f37cb75000 - 0x61f37cb7a000
  - Independent: 0x598e40c46000 - 0x598e40c4b000
- Shared libraries at different locations:
  - Exec'd child libc.so.6: 0x7449b4600000 - 0x7449b4812000
  - Independent libc.so.6: 0x765f49400000 - 0x765f49612000
- Heap, stack, and other segments have different address ranges
- Despite address differences, segment sizes and overall structure are very similar

## General Observations

1. Address Space Layout Randomization (ASLR):
   - Evident in all scenarios, as the same program has different address ranges when run in different contexts
   - Enhances security by making it harder for attackers to predict memory addresses

2. Consistent Segment Sizes:
   - Despite address changes, the size of corresponding segments remains consistent
   - Example: Main program consistently occupies 20 KB across all scenarios

3. Permissions Consistency:
   - Segment permissions (read, write, execute) remain consistent across different runs
   - Ensures proper memory protection and execution rights

4. Special Segments:
   - [vvar], [vdso], and [vsyscall] present in all cases
   - Addresses may vary, but their presence and purpose remain constant

5. Memory Layout Structure:
   - Order of segments (text, data, heap, libraries, stack) consistent across all scenarios
   - Follows the typical virtual memory layout of a Linux process

## Conclusions

1. Initial Similarity: The child process initially shares an almost identical memory layout with the parent, demonstrating the efficiency of fork() in process creation.

2. Exec() Impact: Executing a new program via exec() completely replaces the child's memory layout while maintaining a similar overall structure. This showcases the flexibility of the Linux memory management system.

3. ASLR Effects: The memory layout of a program executed via exec() is structurally similar to the same program run independently, but with different addresses due to ASLR. This highlights the balance between consistency and security in memory management.

4. Structural Consistency: Despite address randomization, the overall structure and sizes of memory segments remain consistent.