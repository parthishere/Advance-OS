# Virtual Memory Layout Analyzer


## Features

- Displays process information (PID and virtual address space size)
- Shows addresses of variables in different memory segments
- Demonstrates heap allocation and growth
- Illustrates stack growth through recursive function calls
- Provides a detailed memory map by reading from `/proc/[pid]/maps`

## Sample Output and Analysis

### Process Information

- PID: 11678
- Virtual Address Space Size: 0xFFFFFFFFFFFFFFFF (18,446,744,073,709,551,615 bytes)

### Memory Segment Addresses

1. **Code Segment (Text)**:
   - Main function: 0x604fe9127771

2. **Data Segment**:
   - Initialized (R/W): 0x604fe912a010 (global2)
   - Initialized (R/O): 0x604fe912a018 (global3)
   - Uninitialized (BSS): 0x604fe912a024 (global1)

3. **Heap Segment**:
   - First allocation: 0x604feafa12b0
   - Second allocation: 0x604feafa12d0
   - Growth direction: Upwards
   - Current Heap end: 0x604feafc1000

4. **Stack Segment**:
   - Local variables: 0x7ffd49de59b8, 0x7ffd49de59bc
   - Command-line arguments:
     - argc: 0x7ffd49de59ac
     - argv: 0x7ffd49de5b08
   - Stack growth demonstration (recursive function calls):
     Addresses from 0x7ffd49de5984 to 0x7ffd49de5894

5. **Memory-Mapped Segment**:
   - mmap allocation: 0x7ffd49de5b08

### Detailed Memory Map

Current segment Text (Code), start 604fe9127000 - 604fe9128000 (size 4096 bytes)
Pointer 0x604fe9127771 is within this Text (Code) segment
Current segment Data, start 604fe912a000 - 604fe912b000 (size 4096 bytes)
Pointer 0x604fe912a010 is within this Data segment
Current segment Heap, start 604feafa0000 - 604feafc1000 (size 135168 bytes)
Pointer 0x604feafa12b0 is within this Heap segment
Current segment Stack, start 7ffd49dc7000 - 7ffd49de8000 (size 135168 bytes)
Pointer 0x7ffd49de59b8 is within this Stack segment

## Analysis and Answers to Questions

1. **What is the size of your process' virtual address space?**
   
   The virtual address space size is 0xFFFFFFFFFFFFFFFF (18,446,744,073,709,551,615 bytes or 16 exbibytes). This indicates a 64-bit address space, which is the maximum theoretical addressable memory for a 64-bit system.

2. **Identify the locations where argv and argc are stored.**
   
   - argc is stored at address 0x7ffd49de59ac
   - argv is stored at address 0x7ffd49de5b08
   
   Both are located in the stack segment, which is expected as they are function parameters for the main function.

3. **What are the addresses of code, static data, stack and heap?**
   
   - Code (Text): Starts at 0x604fe9127000
   - Static Data: Starts at 0x604fe912a000 (includes initialized and uninitialized data)
   - Heap: Starts at 0x604feafa0000
   - Stack: Starts at 0x7ffd49dc7000 (grows downwards)

4. **In what directions do the stack and the heap grow? Explain your answer by using the virtual memory layouts at different execution phases.**
   
   - Heap grows upwards (from lower to higher addresses):
     - First allocation: 0x604feafa12b0
     - Second allocation: 0x604feafa12d0
     This shows that subsequent allocations have higher addresses.

   - Stack grows downwards (from higher to lower addresses):
     - The recursive function calls show addresses decreasing:
       0x7ffd49de5984 -> 0x7ffd49de5954 -> 0x7ffd49de5924 -> ...
     This demonstrates that as new stack frames are added, they use lower addresses.

5. **Your dynamic memory (from malloc()) may not start at the very beginning of the heap. Explain why this is so.**
   
   The first malloc allocation (0x604feafa12b0) doesn't start at the beginning of the heap segment (0x604feafa0000). This gap is due to several factors:
   
   a) Heap management overhead: The memory allocator (e.g., ptmalloc in glibc) needs space for its own data structures to manage allocated and free blocks.
   
   b) Alignment requirements: Memory allocations are often aligned to certain boundaries for performance reasons.
   
   c) Small allocation optimization: Many allocators have optimizations for small allocations, which might be handled differently than larger ones.
   
   d) Security features: Some systems implement security measures like heap canaries or guard pages, which require additional space.

## Memory Map Explanation

The detailed memory map provides insights into how the process's virtual memory is organized:

1. **Text (Code) Segment**: 
   - Range: 0x604fe9127000 - 0x604fe9128000 
   - Size: 4,096 bytes (4 KB)
   - Contains the executable code of the program.

2. **Data Segment**: 
   - Range: 0x604fe912a000 - 0x604fe912b000
   - Size: 4,096 bytes (4 KB)
   - Stores initialized and uninitialized (BSS) global and static variables.

3. **Heap Segment**: 
   - Range: 0x604feafa0000 - 0x604feafc1000
   - Size: 135,168 bytes (132 KB)
   - Used for dynamic memory allocation (malloc, new, etc.).

4. **Stack Segment**: 
   - Range: 0x7ffd49dc7000 - 0x7ffd49de8000
   - Size: 135,168 bytes (132 KB)
   - Used for local variables, function call management, and program stack.

5. **Memory-Mapped Segment**:
   - 
   - Used for shared libraries, memory-mapped files, and certain types of IPC.

