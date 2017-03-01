#ifndef LANSS_UTILS_H
#define LANSS_UTILS_H

void ip_to_string(unsigned long ip, char *string, size_t size);
unsigned short checksum(void *data, int length);

#endif
