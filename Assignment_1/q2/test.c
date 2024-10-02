/*
 * Copyright (c) 2024 Parth Thakkar
 * All rights reserved.
 *
 * File: parent_pgm.c
 * 
 * Description:
 * This file implements an inter-process communication (IPC) system using shared memory,
 * semaphores, and pipes. It creates a parent process and two child processes that communicate
 * with each other through shared memory. The system uses semaphores for synchronization and
 * pipes for initial setup communication. The parent process reads user input and writes it to
 * shared memory, while child processes read from and write to the shared memory in a coordinated manner.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>



/**
 * @brief Main function to set up and run the IPC system
 *
 * @param argc Argument count (unused)
 * @param argv Argument vector (unused)
 * @return int Exit status of the program
 */
int main(int argc, char *argv[])
{
    
    pid_t parent_pid = getpid();

    printf("Parent Process Running %d\n", parent_pid);

    

    // Open output file
    int custom_file_fd = open("/dev/chardev", O_RDWR, 0666);
    if (custom_file_fd == -1)
    {
        perror("fopen");
        fprintf(stderr, "errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    printf("output file fd created %d\n", custom_file_fd);

    // Fork first child process
    // int ChildOnePID = fork();
    // if (ChildOnePID == 0)
    // {
    // }

    // Fork second child process
    // int ChildTwoPID = fork();
    // if (ChildTwoPID == 0)
    // {
    // }


    char file_buf[100];
    if (write(custom_file_fd, file_buf, sizeof(file_buf)) == -1) {
        perror("write to file");
        exit(EXIT_FAILURE);
    }


    
    if (read(custom_file_fd, file_buf, sizeof(file_buf)) == -1) {
        perror("read from file");
        exit(EXIT_FAILURE);
    }


    // Terminate child processes
    // kill(ChildOnePID, SIGTERM);
    // kill(ChildTwoPID, SIGTERM);

    // Wait for child processes to exit
    // waitpid(child1_pid, NULL, 0);
    // waitpid(child2_pid, NULL, 0);
   
    

    return EXIT_SUCCESS;
}