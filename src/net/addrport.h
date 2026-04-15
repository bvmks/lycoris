#ifndef _MS_IPPORT_H
#define _MS_IPPORT_H

#include <arpa/inet.h>

enum {
    addrport_str_max_len = sizeof("255.255.255.255:65535")
};

struct addrport { /* host byte order */
    unsigned int    addr;
    unsigned short  port;
};

int addrport_equal(const struct addrport* a, const struct addrport* b);

void ipport2str(char *str, unsigned int ip, unsigned short port);

void str2ipport(unsigned int *ip, unsigned short *port, const char* str);

void str2ip(unsigned int *ip, const char* str);

void str2port(unsigned short *port, const char* str);

const char *ipport2a(unsigned int ip, unsigned short port);

const char *addrport2a(const struct addrport *ap);

void addrport2str(char *str, const struct addrport *ap);

void str2addrport(struct addrport *dst, char *src);

void addrport2sockaddr_in(struct sockaddr_in* so, const struct addrport* addr);

void sockaddr_in2addrport(struct addrport* addr, const struct sockaddr_in* so);

#endif
