#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "arg_parser.h"

#define SOURCE_ADDRESS_KEY "-s"
#define DESTINATION_ADDRESS_KEY "-d"
#define TTL_KEY "-t"


uint32_t ip_to_number(char* ip_address)
{
    struct in_addr address;

    if (inet_aton(ip_address, &address) == 0)
    {
        perror("ip address is invalid");
        return 0;
    }
    return address.s_addr;
}

int get_echo_args(int args_count, char** argv, struct ECHO_ARGS* result)
{
    int index = 0;
    int exist_next_arg = 0;
    struct in_addr src_address;
    struct in_addr dest_address;

    if (args_count == 0 || argv == NULL || result == NULL) {
        return -1;
    }

    for (index = 0; index < args_count - 1; index++)
    {
        if (strcmp(argv[index], SOURCE_ADDRESS_KEY) == 0)
        {
            result -> src_address = ip_to_number(argv[index + 1]);
        }
        else if (strcmp(argv[index], DESTINATION_ADDRESS_KEY) == 0)
        {
            result -> dest_address = ip_to_number(argv[index + 1]);
        }
        else if (strcmp(argv[index], TTL_KEY) == 0)
        {
            result -> ttl = atoi(argv[index + 1]);
        }
    }
}
