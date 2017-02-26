
#include "arg_parser.h"
#include "icmp_utils.h"

#define REQUESTS_COUNT 5

void ping_destination_address(struct ICMP_ARGS arguments);
int initialize_socket(uint32_t ttl);

int main(int argc, char** argv)
{
    struct ICMP_ARGS arguments;

    if (get_icmp_args(argc, argv, &arguments) == -1)
    {
        return -1;
    }
    ping_destination_address(arguments);

    return 0;
}

void ping_destination_address(struct ICMP_ARGS arguments)
{
    int socket_descriptor = 0;
    int index = 0;
    struct ECHO_REQUEST request;
    struct ECHO_REPLY reply;

    if ((socket_descriptor = initialize_socket(arguments.ttl)) == -1)
    {
        return;
    }

    for (index = 0; index < REQUESTS_COUNT; index ++)
    {
        initialize_echo_request(
            arguments.src_address,
            arguments.dest_address,
            arguments.ttl,
            &request
        );
        send_echo_request(socket_descriptor, request);
        wait_echo_reply(socket_descriptor, &reply);
        print_echo_reply(reply);
    }
    close_socket(socket_descriptor);
}

int initialize_socket(uint32_t ttl)
{
    int socket_descriptor = 0;

    if ((socket_descriptor = create_raw_icmp_socket()) == -1)
    {
        return -1;
    }

    if (bind_socket_to_all_interfaces(socket_descriptor) == -1)
    {
        return -1;
    }

    if (set_socket_ttl_option(socket_descriptor, ttl) == -1)
    {
        return -1;
    }

    if (set_socket_nonblock_mode(socket_descriptor) == -1)
    {
        return -1;
    }
    return socket_descriptor;
}
