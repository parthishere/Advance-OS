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

#define SHM_SIZE 1024
#define SHARED_MEMORY_KEY 0x1234
#define SEM_NAME "/mysemaphore"

int shmid;
int semid;
char *shared_memory;
pid_t parent_pid, child1_pid, child2_pid;
sem_t *sem;
FILE *output_file;
int shmid_to_send;

int main(int argc, char *argv[])
{
    int pipe1[2], pipe2[2];

    parent_pid = getpid();

    printf("Parent Process Running %d\n", parent_pid);

    shmid = shmget(SHARED_MEMORY_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("shmget");
        fprintf(stderr, "errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    shared_memory = shmat(shmid, NULL, 0);
    if (shared_memory == (char *)(-1))
    {
        perror("shmat");
        fprintf(stderr, "errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED)
    {
        perror("sem_open");
        fprintf(stderr, "errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    if (pipe(pipe1) == -1)
    {
        perror("pipe");
        fprintf(stderr, "errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    if (pipe(pipe2) == -1)
    {
        perror("pipe");
        fprintf(stderr, "errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    output_file = fopen("assignment zero output", "w");
    if (output_file == NULL)
    {
        perror("fopen");
        fprintf(stderr, "errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    int ChildOnePID = fork();
    if (ChildOnePID == 0)
    {
        // Child
        close(pipe1[1]);

        char pipe_read_fd_string[20];
        sprintf(pipe_read_fd_string, "%d", pipe1[0]);
        execl("./child_pgm", "child_pgm", "ChildOne", pipe_read_fd_string, NULL);
        perror("execl");
    }

    int ChildTwoPID = fork();
    if (ChildTwoPID == 0)
    {
        // Child
        close(pipe2[1]);

        char pipe_read_fd_string[20];
        // Sending file descriptor as child will have the pipes but it wont have file descriptor.
        sprintf(pipe_read_fd_string, "%d", pipe2[0]);
        execl("./child_pgm", "child_pgm", "ChildTwo", pipe_read_fd_string, NULL);
        perror("execl");
    }
    // Parent
    close(pipe1[0]); // read side close
    close(pipe2[0]); // read side close

    shmid_to_send = shmid;
    if (write(pipe1[1], &shmid_to_send, sizeof(int)) == -1 ||
        write(pipe2[1], &shmid_to_send, sizeof(int)) == -1) {
        perror("write");
        exit(EXIT_FAILURE);
    }


    char buffer[100];
    read(STDIN_FILENO, buffer, sizeof(buffer));
    buffer[strcspn(buffer, "\n")] = 0;
    printf("written %s", buffer);
    sem_wait(sem);
    sprintf(shared_memory, "Parent: %s", buffer);
    sem_post(sem);

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // sleep(1);
        buffer[strcspn(buffer, "\n")] = 0;
        if (strcmp(buffer, "quit") == 0) break;
        

        sem_wait(sem);
        // read(STDIN_FILENO, buffer, sizeof(buffer));
        sprintf(shared_memory, "Parent: %s", buffer);
        sem_post(sem);

        printf("Enter a message (or 'quit' to exit): ");
    }

     // Send termination signal to children
    kill(ChildOnePID, SIGTERM);
    kill(ChildTwoPID, SIGTERM);

    // Wait for children to exit
    waitpid(child1_pid, NULL, 0);
    waitpid(child2_pid, NULL, 0);

    printf("Clearning up resources... \n");
    sem_close(sem);
    sem_unlink(SEM_NAME);
    shmdt(shared_memory);
    shmctl(shmid, IPC_RMID, NULL);

    return EXIT_SUCCESS;
}