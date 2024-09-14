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

#define PARENT_SEM_NAME "/parentsem"
#define CHILDONE_SEM_NAME "/childonesem"
#define CHILDTWO_SEM_NAME "/childtwosem"
#define SHARED_MEM_SEM_NAME "/sharedmemsem"

int shmid;
int semid;
char *shared_memory;
pid_t parent_pid, child1_pid, child2_pid;
sem_t *parent_sem, *child1_sem, *child2_sem, *shm_sem;
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

    parent_sem = sem_open(PARENT_SEM_NAME, O_CREAT, 0666, 1);
    child1_sem = sem_open(CHILDONE_SEM_NAME, O_CREAT, 0666, 0);
    child2_sem = sem_open(CHILDTWO_SEM_NAME, O_CREAT, 0666, 0);
    shm_sem = sem_open(SHARED_MEM_SEM_NAME, O_CREAT, 0666, 0);

    if (parent_sem == SEM_FAILED || child1_sem == SEM_FAILED || child2_sem == SEM_FAILED || shm_sem == SEM_FAILED)
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
        write(pipe2[1], &shmid_to_send, sizeof(int)) == -1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }

    // char buffer[100];
    // read(STDIN_FILENO, buffer, sizeof(buffer));
    // buffer[strcspn(buffer, "\n")] = 0;

    // sem_wait(shm_sem);
    // sprintf(shared_memory, "Parent: %s", buffer);
    // sem_post(shm_sem);
    int a, b, c, d;
    
    sem_getvalue(parent_sem, &a);
    sem_getvalue(child1_sem, &b);
    sem_getvalue(child2_sem, &c);
    sem_getvalue(shm_sem, &d);
   
    printf("initial value for each semaphore %d %d %d %d\n", a, b, c, d);
    sem_post(parent_sem);
    while (1)
    {
        sleep(1);

        sem_wait(parent_sem);

        // buffer[strcspn(buffer, "\n")] = 0;
        // if (strcmp(buffer, "quit") == 0) break;
        printf("parent \n");
        // read(STDIN_FILENO, buffer, sizeof(buffer));
        // sprintf(shared_memory, "Parent: %s", buffer);
        

        sem_post(child1_sem);
    }

    // Send termination signal to children
    kill(ChildOnePID, SIGTERM);
    kill(ChildTwoPID, SIGTERM);

    // Wait for children to exit
    waitpid(child1_pid, NULL, 0);
    waitpid(child2_pid, NULL, 0);

    printf("Clearning up resources... \n");
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

    return EXIT_SUCCESS;
}