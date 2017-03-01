#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <string.h>
#include <ifaddrs.h>

#include "icmp_utils.h"


void handle_socket_creating_error();
void handle_socket_bind_error();
void handle_socket_nonbloc_mode_error();
uint16_t get_process_id();
uint32_t get_current_host_address();
uint16_t calculate_checksum(void *data, int length);

static uint16_t sequence_number = 1;


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

void initialize_echo_request(uint32_t src, uint32_t dst, uint32_t ttl, struct ECHO_REQUEST* request)
{
    int index = 0;

    memset(request, 0, sizeof(struct ECHO_REQUEST));

    request -> icmp_header.type = ICMP_ECHO;
    request -> icmp_header.code = 0;
    request -> icmp_header.id = get_process_id();
    request -> icmp_header.sequence_number = sequence_number++;

    request -> ip_header.version = 4;
    request -> ip_header.header_length = IP_HEADER_LENGTH;
    request -> ip_header.type_of_service = 0;
    request -> ip_header.total_length = htons(sizeof(struct ECHO_REQUEST));
    request -> ip_header.flag_off = 0;
    request -> ip_header.ttl = (uint8_t)ttl;
    request -> ip_header.protocol = IPPROTO_ICMP;
    request -> ip_header.src_address = src ? src : get_current_host_address();
    request -> ip_header.dst_address = dst;
    request -> ip_header.checksum = calculate_checksum(&(request -> ip_header), sizeof(struct IP_HEADER));

    for (index = 0; index < FILL_DATASIZE; index ++)
    {
        request -> data[index] = '0' + index;
    }
    request -> data[index] = '\0';

    request -> icmp_header.checksum = calculate_checksum(
        &(request -> icmp_header),
        sizeof(struct ICMP_HEADER) + FILL_DATASIZE
    );
}

int send_echo_request(int socket_descriptor, struct ECHO_REQUEST request)
{
    return 0;
}

int wait_echo_reply(int socket_descriptor, struct ECHO_REPLY* reply)
{
    return 0;
}

void print_echo_reply(struct ECHO_REPLY reply)
{

}

uint16_t get_process_id()
{
    return (uint16_t)getpid();
}

uint32_t get_current_host_address()
{
    struct ifaddrs *addresses, *temp;
    struct sockaddr_in *p_address, *current_address;

    getifaddrs(&addresses);
    temp = addresses;

    while (temp)
    {
        if (temp -> ifa_addr && temp -> ifa_addr -> sa_family == AF_INET)
        {
            p_address = (struct sockaddr_in *) temp -> ifa_addr;
        }
        temp = temp -> ifa_next;
    }
    memcpy(current_address, p_address, sizeof(struct sockaddr_in));
    freeifaddrs(addresses);

    return current_address -> sin_addr.s_addr;
}

uint16_t calculate_checksum(void *data, int length)
{
    uint16_t *buffer = data;
    uint32_t checksum = 0;
    uint16_t result;

    for (checksum = 0; length > 1; length -= 2 )
    {
        checksum += *buffer++;
    }

    if (length == 1)
    {
        checksum += *(unsigned char*)buffer;
    }
    checksum = (checksum >> 16) + (checksum & 0xFFFF);
    checksum += (checksum >> 16);
    result = (uint16_t) ~checksum;

    return result;
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
