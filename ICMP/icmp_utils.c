#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>

#include "icmp_utils.h"

void handle_socket_creating_error();
void handle_socket_bind_error();
void handle_socket_nonbloc_mode_error();


int create_raw_icmp_socket()
{
    int socket_descriptor = 0;

    if ((socket_descriptor = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
    {
        handle_socket_creating_error();
        return -1;
    }
    return socket_descriptor;
}

int bind_socket_to_all_interfaces(int socket_descriptor)
{
    struct sockaddr_in socket_address;

    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons(0);

    if (bind(socket_descriptor, (const struct sockaddr*)&socket_address, sizeof(socket_address)) == -1)
    {
        handle_socket_bind_error();
        return -1;
    }
    return 0;
}

int set_socket_ttl_option(int socket_descriptor, int ttl)
{
    if (setsockopt(socket_descriptor, SOL_IP, IP_TTL, &ttl, sizeof(ttl)) == -1)
    {
        perror("set socket option failed: ttl\n");
        return -1;
    }
    return 0;
}

int set_socket_nonblock_mode(int socket_descriptor)
{
    int socket_flags = 0;

    socket_flags = fcntl(socket_descriptor, F_GETFL);
    socket_flags |= O_NONBLOCK;

    if (fcntl(socket_descriptor, F_SETFL, socket_flags) == -1)
    {
        handle_socket_nonbloc_mode_error();
        return -1;
    }
    return 0;
}

void close_socket(int socket_descriptor)
{
    if (close(socket_descriptor) == -1)
    {
        perror("socket closing error.\n");
    }
}

void handle_socket_creating_error()
{
    int error_number = errno;

    perror("socket creating failed:\n");

    switch(error_number)
    {
        case EACCES:
            perror("Permission to create a socket of the specified type and/or protocol is denied.\n");
            break;
        case EAFNOSUPPORT:
            perror("The implementation does not support the specified address family.\n");
            break;
        case EPROTONOSUPPORT:
            perror("The protocol type or the specified protocol is not supported within this domain.\n");
            break;
        default:
            perror("Uncknown error.\n");
            break;
    }
}

void handle_socket_bind_error()
{
    int error_number = errno;

    perror("socket binding failed:\n");

    switch(error_number)
    {
        case EACCES:
            perror("The address is protected, and the user is not the superuser.\n");
            break;
        case EADDRINUSE:
            perror("The given address is already in use.\n");
            break;
        default:
            perror("Unknown error.\n");
            break;
    }
}

void handle_socket_nonbloc_mode_error()
{
    int error_number = errno;

    perror("set socket nonblock mode failed:\n");

    switch(error_number)
    {
        case EACCES:
            perror("Operation is prohibited by locks held by other processes.\n");
            break;
        default:
            perror("Unknown error.\n");
            break;
    }
}
