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
        printf("Usage : %s <arg1> <arg2>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("PID %d\n", getpid());

    // heap
    void *dynamic1 = (void *)malloc(1);
    void *dynamic2 = (void *)malloc(1);
    // Stack
    int local1;
    int local2;

    printf("Size of virtual space 0x%lX (%lu) \n", UINTPTR_MAX, UINTPTR_MAX);
    // data segment main
    printf("Code segment (where main is stored): %p\n", (void *)main);

    // initialized segment -> const read only -> read write
    printf("Initialzied Data segment : %p\n", &global2);
    // initialized segment -> const read only -> read
    printf("Initialized segment with const (readonly initialized) : %p\n", &global3);

    // bss segment uninitialized data
    printf("BSS segment (UnInitialized data) %p\n", &global1);

    // malloc heap
    printf("Heap section (dynamic data) %p\n", dynamic1);
    printf("Heap section grows up (dynamic data 2) %p\n", dynamic2);

    // mmap section
    // mmap();
    // printf("mmap section %p\n", );

    // local vars stack
    printf("Stack section (local data) %p\n", &local1);
    printf("Stack section decreases (local data 2) %p\n", &local2);

    // arg and argv above all
    printf("argc and argv address %p %p \n", &argc, argv);

    // from proc directory
    read_proc_dir((void *)main, &global1, &global1, dynamic1, &local1, &argc);
    return EXIT_SUCCESS;
}