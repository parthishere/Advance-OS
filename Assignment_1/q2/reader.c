// reader.c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int fd, bytes_read;
    char buffer[BUFFER_SIZE];
    
    fd = open("/dev/chardev", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open the device");
        return errno;
    }

    printf("Process %d started reading\n", getpid());
    
    while (1) {
        bytes_read = read(fd, buffer, BUFFER_SIZE);
        printf("Process %d read: %.*s\n", getpid(), bytes_read, buffer);
        sleep(1);  // Slow down the reading process
    }

    if (bytes_read < 0) {
        perror("Error reading the device");
    }

    close(fd);
    return 0;
}