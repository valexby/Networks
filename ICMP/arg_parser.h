/*
src_address - source host address (optional)
dst_address - destination address (requred)
ttl - time to live (optional)
*/
struct ECHO_ARGS
{
    uint32_t src_address;
    uint32_t dest_address;
    int ttl;
};

int get_echo_args(int args_count, char** argv, struct ECHO_ARGS* result);
