#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */

#include <string.h>

#define PORT 8080

int main()
{
    int sockfd;
    int new_socket;

    /*
    struct sockaddr_in {
        sa_family_t    sin_family; // address family: AF_INET/ /
        in_port_t      sin_port;   // port in network byte order //
        struct in_addr sin_addr;   // internet address //
    };

    // Internet address //
    struct in_addr {
        uint32_t       s_addr;     // address in network byte order //
    };

    */
    struct sockaddr_in myaddr;
    socklen_t addrlen = sizeof(myaddr);
    int opt = 1; // boolean value // 1 means set the values 

    // keeping protols zero means it will choose one automatically
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("error");
        exit(EXIT_FAILURE);
    }

    /*
    * SYNOPSIS
       #include <sys/socket.h>

       int getsockopt(int sockfd, int level, int optname,
                      void optval[restrict *.optlen],
                      socklen_t *restrict optlen);
       int setsockopt(int sockfd, int level, int optname,
                      const void optval[.optlen],
                      socklen_t optlen);

    DESCRIPTION
       getsockopt()  and  setsockopt() manipulate options for the socket referred to by the file descriptor sockfd.  Options may exist at multiple protocol levels; they
       are always present at the uppermost socket level.

       When manipulating socket options, the level at which the option resides and the name of the option must be specified.  To manipulate options at the  sockets  API
       level, level is specified as SOL_SOCKET.  To manipulate options at any other level the protocol number of the appropriate protocol controlling the option is sup‐
       plied.  For example, to indicate that an option is to be interpreted by the TCP protocol, level should be set to the protocol number of TCP; see getprotoent(3).

       The  arguments optval and optlen are used to access option values for setsockopt().  For getsockopt() they identify a buffer in which the value for the requested
       option(s) are to be returned.  For getsockopt(), optlen is a value-result argument, initially containing the size of the buffer pointed to by optval,  and  modi‐
       fied on return to indicate the actual size of the value returned.  If no option value is to be supplied or returned, optval may be NULL.

       Optname and any specified options are passed uninterpreted to the appropriate protocol module for interpretation.  The include file <sys/socket.h> contains defi‐
       nitions for socket level options, described below.  Options at other protocol levels vary in format and name; consult the appropriate entries in section 4 of the
       manual.

       Most socket-level options utilize an int argument for optval.  For setsockopt(), the argument should be nonzero to enable a boolean option, or zero if the option
       is to be disabled.

       For a description of the available socket options see socket(7) and the appropriate protocol man pages.

    */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsocket");
        exit(EXIT_FAILURE);
    }

    /*
    sin_family is always set to AF_INET.  This is required; in Linux 2.2 most networking functions return EINVAL when this setting is missing.  sin_port contains the
       port in network byte order.  The port numbers below 1024 are called privileged ports (or sometimes: reserved ports).  Only a  privileged  process  (on  Linux:  a
       process that has the CAP_NET_BIND_SERVICE capability in the user namespace governing its network namespace) may bind(2) to these sockets.  Note that the raw IPv4
       protocol as such has no concept of a port, they are implemented only by higher protocols like tcp(7) and udp(7).

       sin_addr  is the IP host address.  The s_addr member of struct in_addr contains the host interface address in network byte order.  in_addr should be assigned one
       of the INADDR_* values (e.g., INADDR_LOOPBACK) using htonl(3) or set using the inet_aton(3), inet_addr(3), inet_makeaddr(3) library functions  or  directly  with
       the name resolver (see gethostbyname(3)).

       IPv4 addresses are divided into unicast, broadcast, and multicast addresses.  Unicast addresses specify a single interface of a host, broadcast addresses specify
       all  hosts  on a network, and multicast addresses address all hosts in a multicast group.  Datagrams to broadcast addresses can be sent or received only when the
       SO_BROADCAST socket flag is set.  In the current implementation, connection-oriented sockets are allowed to use only unicast addresses.

       Note that the address and the port are always stored in network byte order.  In particular, this means that you need to call htons(3) on the number that  is  as‐
       signed to a port.  All address/port manipulation functions in the standard library work in network byte order.


    Special and reserved addresses
       There are several special addresses:

       INADDR_LOOPBACK (127.0.0.1)
              always refers to the local host via the loopback device;

       INADDR_ANY (0.0.0.0)
              means any address for socket binding;

       INADDR_BROADCAST (255.255.255.255)
              has  the  same effect on bind(2) as INADDR_ANY for historical reasons.  A packet addressed to INADDR_BROADCAST through a socket which has SO_BROADCAST set
              will be broadcast to all hosts on the local network segment, as long as the link is broadcast-capable.

       Highest-numbered address
       Lowest-numbered address
              On any locally-attached non-point-to-point IP subnet with a link type that supports broadcasts, the highest-numbered address (e.g., the .255 address on  a
              subnet  with  netmask 255.255.255.0) is designated as a broadcast address.  It cannot usefully be assigned to an individual interface, and can only be ad‐
              dressed with a socket on which the SO_BROADCAST option has been set.  Internet standards have  historically  also  reserved  the  lowest-numbered  address
              (e.g., the .0 address on a subnet with netmask 255.255.255.0) for broadcast, though they call it "obsolete" for this purpose.  (Some sources also refer to
              this as the "network address.")  Since Linux 5.14, it is treated as an ordinary unicast address and can be assigned to an interface.

       Internet standards have traditionally also reserved various addresses for particular uses, though Linux no longer treats some of these specially.

    */
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = INADDR_ANY;
    myaddr.sin_port = htons(PORT); // 8080 port

    if (bind(sockfd, (const struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)  
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // max 3 connection queue
    if (listen(sockfd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    /*
    To accept connections, the following steps are performed:

           (1)  A socket is created with socket(2).

           (2)  The socket is bound to a local address using bind(2), so  that
                other sockets may be connect(2)ed to it.

           (3)  A willingness to accept incoming connections and a queue limit
                for incoming connections are specified with listen().

           (4)  Connections are accepted with accept(2).

    */
    if ((new_socket = accept(sockfd, (struct sockaddr *)&myaddr, &addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];

    // file descriptor, buffer to read to, max read bytes
    ssize_t valread = read(new_socket, buffer,  1024 - 1);
    printf("%s\n", buffer);

    // closing the connected socket
    close(new_socket);
    // closing the listening socket
    close(sockfd);

    return 0;
}

