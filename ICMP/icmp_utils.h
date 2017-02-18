#include <netinet/in.h>

// Structures

#define FILL_DATASIZE 256

struct IP_HEADER
{
    u_char header_length;
    u_char type_of_service;
    short total_length;
    short id;
    short flag_off;
    u_char ttl;
    u_char protocol;
    u_char ckecksum;
    struct in_addr src_address;
    struct in_addr dst_address;
};

struct ICMP_HEADER
{
    u_char type;
    u_char code;
    u_short checksum;
    u_short id;
    u_short seq;
};

struct ECHO_REQUEST
{
    struct IP_HEADER ip_header;
    struct ICMP_HEADER icmp_header;
    int time_stamp;
    char data[FILL_DATASIZE];
};

struct ECHO_REPLY
{
    struct IP_HEADER ip_header;
    struct ICMP_HEADER icmp_header;
    int time_stamp;
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
void initialize_echo_request(uint32_t src, uint32_t dst, int ttl, struct ECHO_REQUEST* request);

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
