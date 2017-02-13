/*
Socket for icpm packages exchange, returns socket descriptor or -1.
Needs root access for create that socket.
*/
int create_raw_icmp_socket();

/*
Bind socket to all awailable interfaces
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
