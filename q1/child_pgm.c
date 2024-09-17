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

#define INFO 1

#if INFO == 1
#define debug printf
#else
#define debug( ...)
#endif

#define SHM_SIZE 4096

#define PARENT_SEM_NAME "/parentsem"
#define CHILDONE_SEM_NAME "/childonesem"
#define CHILDTWO_SEM_NAME "/childtwosem"
#define SHARED_MEM_SEM_NAME "/sharedmemsem"

sem_t *parent_sem, *child1_sem, *child2_sem, *shm_sem;
int shmid;
char * shared_memory;
int output_file_fd;
char * process_name;

void terminate_signal_handler(int signum) {
    if(signum == SIGTERM){
        sem_close(parent_sem);
        exit(0);
    }
}

int main(int argc, char * argv[]){
    if(argc != 3){
        printf("Format should be ./executable <ProcessName> <PipeReadFileDescriptor> | argc %d\n", argc);
        return 1;
    }
    process_name = argv[1];
    int pipe_read_fd = atoi(argv[2]);  // Use argv[2] for pipe file descriptor
    int shared_memory_id;

    if(read(pipe_read_fd, &shared_memory_id, sizeof(int)) == -1 || read(pipe_read_fd, &output_file_fd, sizeof(int)) == -1){
        perror("read");
        exit(EXIT_FAILURE);
    }
    printf("output file fd recieved %d\n", output_file_fd);
    write(output_file_fd, "testing", 8);


    shared_memory = shmat(shared_memory_id, NULL, 0);
    if (shared_memory == (char *)(-1)) {
        perror("shmat");
        fprintf(stderr, "errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    
    parent_sem = sem_open(PARENT_SEM_NAME, 0);
    child1_sem = sem_open(CHILDONE_SEM_NAME, 0);
    child2_sem = sem_open(CHILDTWO_SEM_NAME, 0);
    shm_sem = sem_open(SHARED_MEM_SEM_NAME, 0);

    if (parent_sem == SEM_FAILED || child1_sem == SEM_FAILED || child2_sem == SEM_FAILED || shm_sem == SEM_FAILED)
    {
        perror("sem_open");       
        fprintf(stderr, "errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    signal(SIGTERM, terminate_signal_handler);
    char buffer[100];
    
    while(1){
        sleep(1);

        if(strcmp(process_name, "ChildOne") == 0){
            debug("waiting for semphore child one\n");
            sem_wait(child1_sem);
        }
        else{
            debug("waiting for semphore child two\n");
            sem_wait(child2_sem);
        }

        
        strncpy(shared_memory, buffer, SHM_SIZE);// read from shared memory

        //determine lenght of shared memory
        int length_data_in_shm = strnlen(shared_memory, SHM_SIZE);

        char file_write[length_data_in_shm+20];
        int buffer_written_length = snprintf(file_write, sizeof(file_write),"%s: %s\n", process_name, shared_memory);
        if(buffer_written_length != 0 && buffer_written_length != -1){
            write(output_file_fd, file_write, buffer_written_length);
        }
        
        
        fgets(buffer, sizeof(buffer), stdin); // read from terminal
        buffer[strcspn(buffer, "\n")] = 0;

        printf("You entered: %s , read from process name :%s", buffer, process_name); // write to shared memory
        sprintf(shared_memory, "%s: %s", argv[1], buffer);


        if(strcmp(process_name, "ChildOne") == 0){
            sem_post(child2_sem);
            debug("Posted semaphore of child two from child one\n");
        }
        else{
            sem_post(parent_sem);
            debug("Posted semaphore of main from child two\n");
        }

    }    


    return EXIT_SUCCESS;
}