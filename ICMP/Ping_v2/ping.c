#ifdef __linux__

#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>
#include <ifaddrs.h>

#elif _WIN32

#include<stdio.h>
#include<winsock2.h>
#include <ws2tcpip.h>

#endif

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "ping_utils.h"
#include "ping_types.h"


int sockfd;

int initialize_socket() {

#ifdef _WIN32

    WSADATA wsa;
    printf("Winsock initializing started.\n");

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Winsock initializing failed. Error Code: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    printf("Winsock initialized.\n");

    if((sockfd = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, 0, 0, 0)) == INVALID_SOCKET) {
        printf("Socket creating failed. Error code: %d\n" , WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    printf("Socket was created.\n");
    return sockfd;

#elif __linux__

    struct protoent *protocol = getprotobyname("ICMP");

    if ((sockfd = socket(PF_INET, SOCK_RAW, protocol -> p_proto)) < 0) {
        perror("Socket creating failed.\n");
        exit(EXIT_FAILURE);
    }
    printf("Socket was created.\n");
    return sockfd;

#else

    return 0;

#endif

}

void set_socket_options() {

#ifdef __linux__

    int ttl = DEFAULT_TTL;
    int ip_header_include = 1;

    if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, (const char *) &ttl, sizeof(ttl)) != 0) {
        perror("Error: TTL option was not setted.\n");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, (const char *) &ip_header_include, sizeof(ip_header_include)) != 0) {
        perror("Error: IP header included option was not setted.\n");
        exit(EXIT_FAILURE);
    }

    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) != 0) {
        perror("Error: nonblocking I/O mode was not setted.\n");
        exit(EXIT_FAILURE);
    }

#elif _WIN32

    int ttl = DEFAULT_TTL;

    if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, (const char*)&ttl, sizeof(ttl)) == SOCKET_ERROR) {
        printf("Error: TTL option was not setted. Reason: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

#endif

}

void close_socket() {

#ifdef __linux__

    close(sockfd);


#elif _WIN32

    closesocket(sockfd);

#endif

    printf("Socket was closed.\n");
}

void initialize_request(
    struct ping_request *request,
    const struct sockaddr_in *dst_addr,
    const struct sockaddr_in *src_addr,
    int* sequence_number)
{
    int index;
    memset(request, 0, sizeof(struct ping_request));

#ifdef __linux__

    request -> ip.tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr));
    request -> ip.ihl = 5;
    request -> ip.version = 4;
    request -> ip.ttl = 64;
    request -> ip.tos = 0;
    request -> ip.frag_off = 0;
    request -> ip.protocol = IPPROTO_ICMP;
    request -> ip.saddr = src_addr -> sin_addr.s_addr;
    request -> ip.daddr = dst_addr -> sin_addr.s_addr;
    request -> ip.check = checksum(&request -> ip, sizeof(struct iphdr));

#endif

    for (index = 0; index < sizeof(request -> message) - 1; index++) {
        request -> message[index] = (char) (index + '0');
    }
    request -> message[index] = '\0';

    request -> icmp.type = ICMP_ECHO;
    request -> icmp.un.echo.id = getpid();
    request -> icmp.un.echo.sequence = (*sequence_number)++;
    request -> icmp.checksum = checksum(request, sizeof(struct ping_request));
}


void ping(struct sockaddr_in *dst_address, struct sockaddr_in *src_address, long max_packets_count) {
    int sequence_number = 1;
    long send_packets = 0;
    long received_packets = 0;
    struct ping_request request;
    struct ping_response response;
    struct sockaddr_in response_address;
    char response_address_string[20];

    while (send_packets < max_packets_count || received_packets < max_packets_count) {
        int response_address_length = sizeof(response_address);
        int send_result = 0;
        int recieve_result = 0;

        if (send_packets < max_packets_count) {
            initialize_request(&request, dst_address, src_address, &sequence_number);

            send_result = sendto(
                sockfd,
                (const char*) &request,
                sizeof(request),
                0,
                (struct sockaddr*) dst_address,
                sizeof(*dst_address)
            );

            if (send_result > 0) {
                send_packets++;

                if (send_packets % 10 == 0) {
                    printf("Successfully sended packets count: %ld\n", send_packets);
                }
            } else {
                perror("Ping request sending error.\n");
            }
        }

        recieve_result = recvfrom(
            sockfd,
            (char*) &response,
            sizeof(struct ping_response),
            0,
            (struct sockaddr*) &response_address,
            (socklen_t*) &response_address_length
        );

        if (recieve_result > 0 && response.icmp.type == ICMP_ECHOREPLY) {
            ip_to_string(response.ip.saddr, response_address_string, sizeof(response_address_string));
            printf(
                "Reply from %s (icmp_seq=%d, ttl=%d, id=%d)\n",
                response_address_string,
                response.icmp.un.echo.sequence,
                response.ip.ttl,
                response.ip.id
            );
            received_packets++;
        }
        sleep(1);
    }

    printf(
        "Packets: Sent = %ld, Recieved = %ld, Lost = %ld\n",
        send_packets,
        received_packets,
        send_packets - received_packets
    );
}

void initialize_address(char *host_name, struct sockaddr_in *address) {
    struct hostent *host;

    host = gethostbyname(host_name);
    memset(address, 0, sizeof(struct sockaddr_in));
    address -> sin_family = host -> h_addrtype;
    address -> sin_port = 0;
    memcpy(&(address->sin_addr), host -> h_addr, host -> h_length);
}

void initilize_current_host_address(struct sockaddr_in *address) {
    memset(address, 0, sizeof(struct sockaddr_in));
    address -> sin_family = AF_INET;
    address -> sin_addr.s_addr = INADDR_ANY;
    address -> sin_port = 0;
}

void parse_arguments(int argc, char *argv[], struct ping_arguments* arguments) {
    if (argc < 2 || argc > 4) {
        printf("Invalid arguments count.\n");
        printf("Command usage example: %s <target_address> [max_packets_count] [source_address]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (argc >= 2) {
        initialize_address(argv[1], &(arguments -> dest_addr));
    }

    if (argc >= 3) {
        arguments -> max_packets_count = atoi(argv[2]);
    } else {
        arguments -> max_packets_count = LONG_MAX;
    }

    if (argc == 4) {
        initialize_address(argv[3], &(arguments -> src_addr));
    } else {
        initilize_current_host_address(&(arguments -> src_addr));
    }
}

int main(int argc, char *argv[]) {
    struct ping_arguments arguments;

    initialize_socket();
    set_socket_options();
    parse_arguments(argc, argv, &arguments);
    ping(&arguments.dest_addr, &arguments.src_addr, arguments.max_packets_count);
    close_socket();

    return 0;
}
