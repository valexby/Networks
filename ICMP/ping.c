
#include "arg_parser.h"
#include "icmp_utils.h"

void ping_destination_address(struct ICMP_ARGS arguments);
int initialize_socket(int ttl);

int main(int argc, char** argv)
{
    struct ICMP_ARGS arguments;

    get_icmp_args(argc, argv, &arguments);
    ping_destination_address(arguments);

    return 0;
}

void ping_destination_address(struct ICMP_ARGS arguments)
{
    int socket_descriptor = 0;

    if ((socket_descriptor = initialize_socket(arguments.ttl)) == -1)
    {
        return;
    }

    
}

int initialize_socket(int ttl)
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
