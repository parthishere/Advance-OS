#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <string.h>
#include <arpa/inet.h>

#define PORT 8080 // port to connect to

int main()
{
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[1024] = {0};

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // inet_pton - convert IPv4 and IPv6 addresses from text to binary form
    /*
    This function converts the character string src into a network address structure in the af address family, then copies the network address structure to dst.  The
       af argument must be either AF_INET or AF_INET6.  dst is written in network byte order.

       The following address families are currently supported:

       AF_INET
       AF_INET6

    */
    if (inet_pton(AF_INET, "192.168.5.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    /*
    The  send()  call  may  be  used  only when the socket is in a connected state (so that the intended recipient is known).  The only difference between send() and
       write(2) is the presence of flags.  With a zero flags argument, send() is equivalent to write(2).  Also, the following call

           send(sockfd, buf, len, flags);

       is equivalent to

           sendto(sockfd, buf, len, flags, NULL, 0);

       The argument sockfd is the file descriptor of the sending socket.
    */
    send(client_fd, hello, strlen(hello), 0 /*no flags*/);

    // closing the connected socket
    close(client_fd);
    return 0;// closing the connected socket
}