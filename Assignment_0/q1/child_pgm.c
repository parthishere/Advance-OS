/*
 * Copyright (c) 2024 Your Organization
 * All rights reserved.
 *
 * File: ipc_child_process.c
 * 
 * Description:
 * This file implements a child process in an Inter-Process Communication (IPC) system.
 * It uses shared memory and semaphores to communicate with a parent process and another
 * child process. The child process reads from shared memory, writes to an output file,
 * and writes its own input to shared memory in a coordinated manner using semaphores.
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
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

// Debug macro: If INFO is 1, debug messages will be printed
#define INFO 1

#if INFO == 1
#define debug printf
#else
#define debug( ...)
#endif

// Constants
#define SHM_SIZE 4096

// Semaphore names for synchronization
#define PARENT_SEM_NAME "/parentsem"
#define CHILDONE_SEM_NAME "/childonesem"
#define CHILDTWO_SEM_NAME "/childtwosem"
#define SHARED_MEM_SEM_NAME "/sharedmemsem"

// Global variables
sem_t *parent_sem, *child1_sem, *child2_sem, *shm_sem;
int shmid;
char *shared_memory;
int output_file_fd;
char *process_name;

/**
 * @brief Signal handler for termination signal
 *
 * @param signum The signal number received
 */
void terminate_signal_handler(int signum) {
    if(signum == SIGTERM){
        sem_close(parent_sem);
        exit(0);
    }
}

/**
 * @brief Main function for the child process
 *
 * @param argc Argument count
 * @param argv Argument vector
 * @return int Exit status of the program
 */
int main(int argc, char *argv[]) {
    // Check for correct number of arguments
    if(argc != 3){
        printf("Format should be ./executable <ProcessName> <PipeReadFileDescriptor> | argc %d\n", argc);
        return 1;
    }

    process_name = argv[1];
    int pipe_read_fd = atoi(argv[2]);  // Use argv[2] for pipe file descriptor
    int shared_memory_id;

    // Read shared memory ID and output file descriptor from pipe
    if(read(pipe_read_fd, &shared_memory_id, sizeof(int)) == -1 || 
       read(pipe_read_fd, &output_file_fd, sizeof(int)) == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    // Attach to shared memory
    shared_memory = shmat(shared_memory_id, NULL, 0);
    if (shared_memory == (char *)(-1)) {
        perror("shmat");
        fprintf(stderr, "errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    
    // Open semaphores
    parent_sem = sem_open(PARENT_SEM_NAME, 0);
    child1_sem = sem_open(CHILDONE_SEM_NAME, 0);
    child2_sem = sem_open(CHILDTWO_SEM_NAME, 0);
    shm_sem = sem_open(SHARED_MEM_SEM_NAME, 0);

    if (parent_sem == SEM_FAILED || child1_sem == SEM_FAILED || 
        child2_sem == SEM_FAILED || shm_sem == SEM_FAILED) {
        perror("sem_open");       
        fprintf(stderr, "errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    // Set up signal handler for termination
    signal(SIGTERM, terminate_signal_handler);

    char buffer[100];
    
    // Main communication loop
    while(1) {
        // Wait for the appropriate semaphore based on process name
        if(strcmp(process_name, "ChildOne") == 0) {
            debug("waiting for semaphore child one\n");
            sem_wait(child1_sem);
        } else {
            debug("waiting for semaphore child two\n");
            sem_wait(child2_sem);
        }

        // Read from shared memory and write to output file
        char file_write[SHM_SIZE + 20];
        int buffer_written_length = snprintf(file_write, sizeof(file_write), "%s\n", shared_memory);

        if (write(output_file_fd, file_write, buffer_written_length) == -1) {
            perror("write to file");
            exit(EXIT_FAILURE);
        }

        // Get user input
        printf("%s, enter a string: ", process_name);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        buffer[strcspn(buffer, "\n")] = 0;  // Remove newline

        // Write to shared memory
        snprintf(shared_memory, SHM_SIZE, "%s: %s", process_name, buffer);

        // Signal the next process in the communication cycle
        if(strcmp(process_name, "ChildOne") == 0) {
            sem_post(child2_sem);
            debug("Posted semaphore of child two from child one\n");
        } else {
            sem_post(parent_sem);
            debug("Posted semaphore of main from child two\n");
        }
    }    

    return EXIT_SUCCESS;
}