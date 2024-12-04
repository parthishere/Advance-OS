#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Socket creation failed (expected)\n");
        return 1;
    }
    return 0;
}