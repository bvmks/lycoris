#ifndef _MS_IPPORT_H
#define _MS_IPPORT_H

enum {
    addrport_str_max_len = sizeof("255.255.255.255:65535")
};

struct addrport { /* host byte order */
    unsigned int    addr;
    unsigned short  port;
};

void ipport2str(char *str, unsigned int ip, unsigned short port);

const char *ipport2a(unsigned int ip, unsigned short port);

const char *addrport2a(const struct addrport *ap);

void addrport2str(char *str, const struct addrport *ap);

#endif
