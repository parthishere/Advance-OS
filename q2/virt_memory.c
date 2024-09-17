// virtual memroy laout
// symbol table
// page tables
// sbrk
// mmap

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#define MAX_LINE 256

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

void read_proc_dir(void *code_ptr, void *data_ptr, void *bss_ptr, void *heap_ptr, void *stack_ptr, void *cmd_args_seg)
{
    pid_t pid = getpid();
    char filename[25];
    sprintf(filename, "/proc/%d/maps", pid);
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("fread");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    long start;
    long end;
    char perms[12];
    long unsigned offset;
    int dev_major;
    int dev_minor;
    long unsigned inode;
    char pathname[100];

    SegmentType current_segment = TEXT;

    while (fgets(buffer, MAX_LINE, file) != NULL)
    {
        sscanf(buffer, "%lx-%lx %4s %lx %x:%x %lu %255s",
               &start, &end, perms, &offset, &dev_major, &dev_minor, &inode, pathname);

        const char *segment_name = "Unknown";
        void *compare_ptr = NULL;

        if (strstr(pathname, "[heap]")) {
            segment_name = "Heap";
            current_segment = HEAP;
            compare_ptr = heap_ptr;
        } else if (strstr(pathname, "[stack]")) {
            segment_name = "Stack";
            current_segment = STACK;
            compare_ptr = stack_ptr;
        } else if (strstr(pathname, "[vdso]")) {
            continue;
        } else if (strstr(pathname, "[vvar]")) {
            continue;
        } else if (strstr(pathname, "[vsyscall]")) {
           continue;
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

        
        if((unsigned long)compare_ptr > start && (unsigned long)compare_ptr < end){
        printf("Current segment %s, start %lx - %lx (size %lu bytes)\n", segment_name, start, end, end - start);
            printf("  Pointer %p is within this %s segment\n", compare_ptr, segment_name);
        }
    }

    fclose(file);
    printf("\n");
}


// Global will go to Data segment
int global1;                     // BSS
int global2 = 10;                // Initialized rw
char *global3 = "Parth Thakkar"; // Initialized r

// craete some function to print out proc directory memory map

int main(int argc, char *argv[])
{
    
    if (argc != 3)
    {
        printf("Usage: %s <arg1> <arg2>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("=== Process Information ===\n");
    printf("PID: %d\n", getpid());
    printf("Virtual Address Space Size: 0x%lX (%lu bytes)\n\n", UINTPTR_MAX, UINTPTR_MAX);

    // Heap allocations
    void *dynamic1 = malloc(1);
    void *dynamic2 = malloc(1);
    
    // Stack variables
    int local1 = 10;
    int local2 = 20;

    printf("=== Memory Segment Addresses ===\n");
    printf("1. Code Segment (Text):\n");
    printf("   Main function: %p\n\n", (void *)main);

    printf("2. Data Segment:\n");
    printf("   Initialized (R/W):  %p (global2)\n", (void *)&global2);
    printf("   Initialized (R/O):  %p (global3)\n", (void *)&global3);
    printf("   Uninitialized (BSS): %p (global1)\n\n", (void *)&global1);

    printf("3. Heap Segment:\n");
    printf("   First allocation:  %p\n", dynamic1);
    printf("   Second allocation: %p\n", dynamic2);
    printf("   Growth direction:  %s\n\n", 
           (dynamic2 > dynamic1) ? "Upwards" : "Downwards");

    printf("4. Stack Segment:\n");
    printf("   Local variable 1: %p\n", (void *)&local1);
    printf("   Local variable 2: %p\n", (void *)&local2);
    printf("   Growth direction: %s\n", 
           ((signed long)&local2 < (signed long)&local1) ? "Downwards" : "Upwards");
    printf("   Command-line arguments:\n");
    printf("     argc: %p\n", (void *)&argc);
    printf("     argv: %p\n\n", (void *)argv);

    printf("5. Memory-Mapped Segment:\n");
    printf("   (No mmap allocation in this example)\n\n");

    printf("=== Detailed Memory Map from /proc ===\n");
    read_proc_dir((void *)main, &global2, &global1, dynamic1, &local1, &argc);

    // Clean up
    free(dynamic1);
    free(dynamic2);

    return EXIT_SUCCESS;
}