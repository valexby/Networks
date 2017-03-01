#if !defined(_STD_INT_)

#define _STD_INT_
#include <stdint.h>

#endif
/*
src_address - source host address (optional)
dst_address - destination address (requred)
ttl - time to live (optional)
*/
struct ICMP_ARGS
{
    uint32_t src_address;
    uint32_t dest_address;
    uint32_t ttl;
};

int get_icmp_args(int args_count, char** argv, struct ICMP_ARGS* result);
