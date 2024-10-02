// writer.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    int fd, bytes_written;
    char buffer[100];
    
    fd = open("/dev/chardev", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open the device");
        return errno;
    }

    printf("Process %d started writing\n", getpid());
    int i;
    while (1) {
        snprintf(buffer, sizeof(buffer), "Message %d from process %d\n", i, getpid());
        bytes_written = write(fd, buffer, strlen(buffer));
        if (bytes_written < 0) {
            perror("Error writing to the device");
            
        }
        printf("Process %d wrote: %s", getpid(), buffer);
        sleep(1);  // Slow down the writing process
        i++;
    }

    close(fd);
    return 0;
}