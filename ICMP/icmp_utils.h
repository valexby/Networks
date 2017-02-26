#if !defined(_STD_INT_)

#define _STD_INT_
#include <stdint.h>

#endif

// Structures

#define IP_HEADER_LENGTH 5
#define FILL_DATASIZE 256

struct IP_HEADER
{
    uint8_t version:4, header_length:4;
    uint8_t type_of_service;
    uint16_t total_length;
    uint16_t id;
    uint16_t flag_off;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t src_address;
    uint32_t dst_address;
};

struct ICMP_HEADER
{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t sequence_number;
};

struct ECHO_REQUEST
{
    struct IP_HEADER ip_header;
    struct ICMP_HEADER icmp_header;
    char data[FILL_DATASIZE];
};

struct ECHO_REPLY
{
    struct IP_HEADER ip_header;
    struct ICMP_HEADER icmp_header;
    char data[FILL_DATASIZE];
};

//Functions

/*
Socket for icpm packages exchange, returns socket descriptor or -1.
Needs root access for create that socket.
*/
int create_raw_icmp_socket();

/*
Closes socket descriptor as file descriptor
*/
void close_socket(int socket_descriptor);

/*
Binds a socket to all awailable interfaces
*/
int bind_socket_to_all_interfaces(int socket_descriptor);

/*
Sets ttl value for socket.
*/
int set_socket_ttl_option(int socket_descriptor, int ttl);

/*
Sets non-block mode
*/
int set_socket_nonblock_mode(int socket_descriptor);

/*
Sets all needed fields of echo request
*/
void initialize_echo_request(uint32_t src, uint32_t dst, uint32_t ttl, struct ECHO_REQUEST* request);

/*
Sends a echo request
*/
int send_echo_request(int socket_descriptor, struct ECHO_REQUEST request);


/*
Waits for echo reply. Sets reply by passed address;
*/
int wait_echo_reply(int socket_descriptor, struct ECHO_REPLY* reply);


/*
Prints a echo request result
*/
void print_echo_reply(struct ECHO_REPLY reply);
