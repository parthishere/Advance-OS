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

#define SHM_SIZE 4096
#define SEM_NAME "/mysemaphore"

sem_t * sem;
int shmid;
char * shared_memory;

void terminate_signal_handler(int signum) {
    if(signum == SIGTERM){
        sem_close(sem);
        exit(0);
    }
}

int main(int argc, char * argv[]){
    if(argc != 3){
        printf("Format should be ./executable <ProcessName> <PipeReadFileDescriptor> | argc %d\n", argc);
        return 1;
    }

     printf("Process Name : %s\n", argv[1]);  // Use argv[1] for process name
    int pipe_read_fd = atoi(argv[2]);  // Use argv[2] for pipe file descriptor
    int shared_memory_id;
    
    if(read(pipe_read_fd, &shared_memory_id, sizeof(int)) == -1){
        perror("read");
        exit(EXIT_FAILURE);
    }
    
    
    printf("Received shared memory id: %d\n", shared_memory_id);

    shared_memory = shmat(shared_memory_id, NULL, 0);
    if (shared_memory == (char *)(-1)) {
        perror("shmat");
        fprintf(stderr, "errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    
    sem = sem_open(SEM_NAME, 0);

    signal(SIGTERM, terminate_signal_handler);

    while(1){
        sleep(1);

        sem_wait(sem);
        // sprintf(shared_memory);
        printf("%s \n", shared_memory);
        // sem_post(sem);

    }

    // int shmid = shmget(SHARED_MEMORY_KEY, 0, IPC_CREAT | 0666);
    // if (shmid == -1)
    // {
    //     perror("shmget");
    //     fprintf(stderr, "errno: %d\n", errno);
    //     exit(EXIT_FAILURE);
    // }

    // char *shm = shmat(shmid, NULL, 0);
    // if (shm == NULL)
    // {
    //     perror("shmat");
    //     fprintf(stderr, "errno: %d\n", errno);
    //     exit(EXIT_FAILURE);
    // }

    // sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    // if (sem == SEM_FAILED)
    // {
    //     perror("sem_open");
    //     fprintf(stderr, "errno: %d\n", errno);
    //     exit(EXIT_FAILURE);
    // }

    return EXIT_SUCCESS;
}