#include <stdio.h>
#include <string.h>
#include "ping_utils.h"

void ip_to_string(unsigned long ip, char *string, size_t size) {
    if (size < 16) {
        return;
    }

    memset(string, 0, size);
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;
    sprintf(string, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
}

unsigned short checksum(void *data, int length) {
    unsigned short *buffer = data;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; length > 1; length -= 2) {
        sum += *buffer++;
    }

    if (length == 1) {
        sum += *(unsigned char*)buffer;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = (unsigned short) ~sum;

    return result;
}
