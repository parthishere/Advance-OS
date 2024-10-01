#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <wait.h>

#define MAX_LINE 256

/*
 * This function reads and prints the memory map of the current process.
 * It's used to compare the virtual memory layouts of parent and child processes.
 */
void read_proc_dir()
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
    long start, end;
    char perms[12];
    long unsigned offset;
    int dev_major, dev_minor;
    long unsigned inode;
    char pathname[100];

    while (fgets(buffer, MAX_LINE, file) != NULL)
    {
        sscanf(buffer, "%lx-%lx %4s %lx %x:%x %lu %255s",
               &start, &end, perms, &offset, &dev_major, &dev_minor, &inode, pathname);

        // Print detailed information about each memory segment
        printf("1. Address range: 0x%lx - 0x%lx (Size: %lu bytes)\n", start, end, end - start);
        printf("2. Permissions: %s\n", perms);
        printf("3. File offset: 0x%lx\n", offset);
        printf("4. Device: %d:%d\n", dev_major, dev_minor);
        printf("5. Inode: %lu\n", inode);

        if (pathname[0] != '\0')
        {
            printf("6. Pathname: %s\n\n", pathname);
        }
        else
        {
            printf("6. Pathname: [none]\n\n");
        }
    }

    fclose(file);
    printf("\n");
}

/*
 * This function demonstrates the comparison of virtual memory layouts
 * between parent and child processes before and after exec().
 */
void foo()
{
    printf("Parent Memory map ... \n");
    read_proc_dir();
    printf(" ============================= \n");
    printf(" ============================= \n\n");

    int childPID = fork();

    if (childPID == 0)
    {
        // Child process
        printf("Child Memory map ... \n");
        read_proc_dir();

        // Execute another program
        // This will replace the child's memory layout with the new program's layout
        printf(" ============================= \n");
        printf(" ============================= \n\n");

        printf("Child Memory map after exec ... \n");
        execlp("./sample_program", "sample_program", NULL);

        // If execlp fails, we'll reach this point
        perror("execlp");
        exit(EXIT_FAILURE);
    }
    else
    {
        // Parent process
        wait(NULL); // Wait for the child to finish
    }
}

int main()
{
    foo();
    return EXIT_SUCCESS;
}