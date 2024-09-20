/*
 * Copyright (c) 2024 Parth Thakkar
 * All rights reserved.
 *
 * File: virt_memory.c
 * 
 * Description:
 * This file implements a program to explore and analyze the virtual memory layout
 * of a process. It demonstrates various aspects of memory management including:
 * - Text (Code) segment
 * - Data segment (initialized and uninitialized)
 * - Heap segment (dynamic memory allocation)
 * - Stack segment
 * - Memory-mapped segments
 * 
 * The program uses system calls and procfs to gather and display information about
 * the process's memory layout, providing insights into how different types of data
 * are stored and managed within a process's virtual address space.
 */

// Include necessary header files for system calls, standard I/O, and error handling
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>

#define MAX_LINE 256  // Maximum length of a line read from /proc/[pid]/maps

// Enum to represent different types of memory segments
typedef enum {
    TEXT,
    DATA,
    BSS,
    HEAP,
    MMAP,
    STACK,
    VDSO,
    VVAR,
    VSYSCALL
} SegmentType;

/**
 * read_proc_dir - Read and analyze the memory map of the current process
 *
 * @param code_ptr: Pointer to a location in the code segment
 * @param data_ptr: Pointer to a location in the data segment
 * @param bss_ptr: Pointer to a location in the BSS segment
 * @param heap_ptr: Pointer to a location in the heap segment
 * @param stack_ptr: Pointer to a location in the stack segment
 * @param cmd_args_seg: Pointer to command-line arguments
 *
 * This function reads the /proc/[pid]/maps file to obtain information about
 * the process's memory layout. It then analyzes and prints details about
 * each memory segment, including its address range and size.
 */
void read_proc_dir(void *code_ptr, void *data_ptr, void *bss_ptr, void *heap_ptr, void *stack_ptr, void *cmd_args_seg)
{
    pid_t pid = getpid();  // Get the process ID
    char filename[25];
    sprintf(filename, "/proc/%d/maps", pid);  // Construct the path to the memory map file
    
    // Open the memory map file
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("fread");
        exit(EXIT_FAILURE);
    }

    // Variables to store information about each memory segment
    char buffer[256];
    long start, end;
    char perms[12];
    long unsigned offset;
    int dev_major, dev_minor;
    long unsigned inode;
    char pathname[100];

    SegmentType current_segment = TEXT;  // Start with the text segment

    // Read and process each line of the memory map file
    while (fgets(buffer, MAX_LINE, file) != NULL)
    {
        // Parse the line to extract segment information
        sscanf(buffer, "%lx-%lx %4s %lx %x:%x %lu %255s",
               &start, &end, perms, &offset, &dev_major, &dev_minor, &inode, pathname);

        const char *segment_name = "Unknown";
        void *compare_ptr = NULL;

        // Determine the segment type based on the pathname and permissions
        if (strstr(pathname, "[heap]")) {
            segment_name = "Heap";
            current_segment = HEAP;
            compare_ptr = heap_ptr;
        } else if (strstr(pathname, "[stack]")) {
            segment_name = "Stack";
            current_segment = STACK;
            compare_ptr = stack_ptr;
        } else if (strstr(pathname, "[vdso]") || strstr(pathname, "[vvar]") || strstr(pathname, "[vsyscall]")) {
            continue;  // Skip these special segments
        } else if (perms[2] == 'x' && current_segment == TEXT) {
            segment_name = "Text (Code)";
            compare_ptr = code_ptr;
        } else if (perms[1] == 'w' && current_segment == TEXT) {
            segment_name = "Data";
            current_segment = DATA;
            compare_ptr = data_ptr;
        } else if (current_segment == DATA) {
            segment_name = "BSS";
            current_segment = BSS;
            compare_ptr = bss_ptr;
        } else if (pathname[0] == '\0') {
            segment_name = "Mapped File";
            current_segment = MMAP;
        }

        // Check if the compare pointer is within the current segment
        if((unsigned long)compare_ptr > start && (unsigned long)compare_ptr < end){
            printf("Current segment %s, start %lx - %lx (size %lu bytes)\n", segment_name, start, end, end - start);
            printf("  Pointer %p is within this %s segment\n", compare_ptr, segment_name);
        }
    }

    fclose(file);
    printf("\n");
}

/**
 * stack_growth_demo - Demonstrate stack growth through recursive function calls
 *
 * @param depth: The current depth of recursion
 *
 * This function recursively calls itself to show how the stack grows
 * with each function call. It prints the address of a local variable
 * at each level of recursion.
 */
void stack_growth_demo(int depth) {
    int local_var;
    printf("   Stack in stack_growth_demo %p\n", (void *)&local_var);
    if (depth > 0) {
        stack_growth_demo(depth - 1);
    }
}

// Global variables to demonstrate different segments
int global1;                     // Uninitialized global variable (BSS segment)
int global2 = 10;                // Initialized global variable (Data segment)
char *global3 = "Parth Thakkar"; // Initialized global pointer (Data segment, points to read-only data)


/**
 * main - Entry point of the program
 *
 * @param argc: Number of command-line arguments
 * @param argv: Array of command-line argument strings
 * @return: EXIT_SUCCESS on successful execution, EXIT_FAILURE otherwise
 *
 * This function demonstrates various aspects of virtual memory layout:
 * - Prints process information
 * - Allocates memory on the heap
 * - Shows addresses of variables in different segments
 * - Demonstrates stack growth
 * - Reads and displays detailed memory map information
 */
int main(int argc, char *argv[])
{
    // Check for correct number of command-line arguments
    if (argc != 3)
    {
        printf("Usage: %s <arg1> <arg2>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Print basic process information
    printf("=== Process Information ===\n");
    printf("PID: %d\n", getpid());
    printf("Virtual Address Space Size: 0x%lX (%lu bytes)\n\n", UINTPTR_MAX, UINTPTR_MAX);

    // Heap allocations to demonstrate heap growth
    void *dynamic1 = malloc(1);
    void *dynamic2 = malloc(1);
    
    // Stack variables
    int local1 = 10;
    int local2 = 20;

    static int data = 10; // static data 

    // Print addresses of variables in different memory segments
    printf("=== Memory Segment Addresses ===\n");
    printf("1. Code Segment (Text):\n");
    printf("   Main function: %p\n\n", (void *)main);
    
    printf("2. Data Segment:\n");
    printf("   Initialized (R/W):  %p (global2)\n", (void *)&global2);
    printf("   Initialized (R/O):  %p (global3)\n", (void *)&global3);
    printf("   Static data %p \n", &data);
    printf("   Uninitialized (BSS): %p (global1)\n\n", (void *)&global1);

    
    printf("3. Heap Segment:\n");
    printf("   First allocation:  %p\n", dynamic1);
    printf("   Second allocation: %p\n", dynamic2);
    printf("   Growth direction:  %s\n\n", 
           (dynamic2 > dynamic1) ? "Upwards" : "Downwards");
    int * current_break = sbrk(0);  // Get current program break
    printf("   Current Heap end %p \n", current_break);
    dynamic1 = (void *)malloc(1);
    current_break = sbrk(0);  // Get current program break
    printf("   Current Heap end(After 1byte Malloc) %p \n", current_break);
    
    printf("4. Stack Segment:\n");
    printf("   Local variable 1: %p\n", (void *)&local1);
    printf("   Local variable 2: %p\n", (void *)&local2);
    
    printf("   Command-line arguments:\n");
    printf("     argc: %p\n", (void *)&argc);
    printf("     argv: %p\n", (void *)argv);
    stack_growth_demo(5);  // Demonstrate stack growth

    printf("\n5. Memory-Mapped Segment:\n");
    void *mmap1 = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    printf("   mmap allocation: %p \n\n", mmap1);

    // Read and display detailed memory map information
    printf("=== Detailed Memory Map from /proc ===\n");
    read_proc_dir((void *)main, &global2, &global1, dynamic1, &local1, &argc);

    // Clean up allocated memory
    free(dynamic1);
    free(dynamic2);

    return EXIT_SUCCESS;
}