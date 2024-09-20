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

// Constants for shared memory and buffer sizes
#define SHM_SIZE 1024
#define MAX_BUFFER 100
#define SHARED_MEMORY_KEY 0x1234

// Semaphore names for synchronization
#define PARENT_SEM_NAME "/parentsem"
#define CHILDONE_SEM_NAME "/childonesem"
#define CHILDTWO_SEM_NAME "/childtwosem"
#define SHARED_MEM_SEM_NAME "/sharedmemsem"

// Global variables for IPC resources
int shmid;
int semid;
char *shared_memory;
pid_t parent_pid, child1_pid, child2_pid;
sem_t *parent_sem, *child1_sem, *child2_sem, *shm_sem;
int output_file_fd;
int shmid_to_send;

/**
 * @brief Main function to set up and run the IPC system
 *
 * @param argc Argument count (unused)
 * @param argv Argument vector (unused)
 * @return int Exit status of the program
 */
int main(int argc, char *argv[])
{
    int pipe1[2], pipe2[2];

    parent_pid = getpid();

    printf("Parent Process Running %d\n", parent_pid);

    // Create shared memory segment
    shmid = shmget(SHARED_MEMORY_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("shmget");
        fprintf(stderr, "errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    // Attach shared memory segment
    shared_memory = shmat(shmid, NULL, 0);
    if (shared_memory == (char *)(-1))
    {
        perror("shmat");
        fprintf(stderr, "errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    // Initialize semaphores for synchronization
    parent_sem = sem_open(PARENT_SEM_NAME, O_CREAT, 0666, 0);
    child1_sem = sem_open(CHILDONE_SEM_NAME, O_CREAT, 0666, 0);
    child2_sem = sem_open(CHILDTWO_SEM_NAME, O_CREAT, 0666, 0);
    shm_sem = sem_open(SHARED_MEM_SEM_NAME, O_CREAT, 0666, 0);

    if (parent_sem == SEM_FAILED || child1_sem == SEM_FAILED || child2_sem == SEM_FAILED || shm_sem == SEM_FAILED)
    {
        perror("sem_open");
        fprintf(stderr, "errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    // Create pipes for communication with child processes
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1)
    {
        perror("pipe");
        fprintf(stderr, "errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    // Open output file
    output_file_fd = open("assignment zero output", O_RDWR, 0666);
    if (output_file_fd == -1)
    {
        perror("fopen");
        fprintf(stderr, "errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    printf("output file fd created %d\n", output_file_fd);

    // Fork first child process
    int ChildOnePID = fork();
    if (ChildOnePID == 0)
    {
        // Child One process
        close(pipe1[1]);  // Close write end of pipe

        char pipe_read_fd_string[20];
        sprintf(pipe_read_fd_string, "%d", pipe1[0]);
        execl("./child_pgm", "child_pgm", "ChildOne", pipe_read_fd_string, NULL);
        perror("execl");
    }

    // Fork second child process
    int ChildTwoPID = fork();
    if (ChildTwoPID == 0)
    {
        // Child Two process
        close(pipe2[1]);  // Close write end of pipe

        char pipe_read_fd_string[20];
        sprintf(pipe_read_fd_string, "%d", pipe2[0]);
        execl("./child_pgm", "child_pgm", "ChildTwo", pipe_read_fd_string, NULL);
        perror("execl");
    }

    // Parent process
    close(pipe1[0]);  // Close read end of pipe1
    close(pipe2[0]);  // Close read end of pipe2

    shmid_to_send = shmid;

    // Send shared memory ID and output file descriptor to child processes
    if (write(pipe1[1], &shmid_to_send, sizeof(int)) == -1 ||
        write(pipe2[1], &shmid_to_send, sizeof(int)) == -1 ||
        write(pipe1[1], &output_file_fd, sizeof(int)) == -1 ||
        write(pipe2[1], &output_file_fd, sizeof(int)) == -1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }

    char buffer[100];
    printf("Parent, enter initial string: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0;  // Remove newline character
    printf("You entered: %s", buffer);
    sprintf(shared_memory, "Parent: %s", buffer);

    // Main communication loop
    sem_post(child1_sem);  // Signal Child One to start
    while (1)
    {
        sem_wait(parent_sem);  // Wait for parent's turn
        
        // Read from shared memory and write to output file
        char temp[SHM_SIZE];
        strncpy(temp, shared_memory, SHM_SIZE);
        dprintf(output_file_fd, "%s\n", temp);

        // Get user input
        printf("Enter string (TERMINATE to exit): ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;  // Remove newline character

        if (strcmp(buffer, "TERMINATE") == 0)
            break;

        // Write user input to shared memory
        snprintf(shared_memory, SHM_SIZE, "Parent: %s", buffer);
    
        sem_post(child1_sem);  // Signal Child One to continue
    }

    // Terminate child processes
    kill(ChildOnePID, SIGTERM);
    kill(ChildTwoPID, SIGTERM);

    // Wait for child processes to exit
    waitpid(child1_pid, NULL, 0);
    waitpid(child2_pid, NULL, 0);

    // Clean up resources
    printf("Cleaning up resources... \n");
    sem_close(parent_sem);
    sem_unlink(PARENT_SEM_NAME);
    sem_close(child1_sem);
    sem_unlink(CHILDONE_SEM_NAME);
    sem_close(child2_sem);
    sem_unlink(CHILDTWO_SEM_NAME);
    sem_close(shm_sem);
    sem_unlink(SHARED_MEM_SEM_NAME);
    shmdt(shared_memory);
    shmctl(shmid, IPC_RMID, NULL);
    close(output_file_fd);

    return EXIT_SUCCESS;
}