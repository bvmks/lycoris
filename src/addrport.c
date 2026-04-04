#include <stdio.h>
#include <stdlib.h>

#include "addrport.h"

void ipport2str(char *str, unsigned int ip, unsigned short port)
{
    sprintf(str, "%d.%d.%d.%d:%d",
                 (int) ((ip >> 24) & 0xff),
                 (int) ((ip >> 16) & 0xff),
                 (int) ((ip >> 8)  & 0xff),
                 (int) (ip         & 0xff),
                 port);
}

void str2ipport(unsigned int *ip, unsigned short *port, char* str)
{
    static char str_ip[sizeof("255.255.255.255")], str_port[sizeof("65535")];
    sscanf(str, "%s:%s", str_ip, str_port);
    str2ip(ip, str_ip);
    str2port(port, str_port);
}

void str2ip(unsigned int *ip, const char* str)
{
    struct in_addr* i;
    inet_aton(str, i);
    *ip = ntohl(i->s_addr);
}

void str2port(unsigned short *port, const char* str)
{
    *port = atoi(str);
}

void addrport2str(char *str, const struct addrport *ap)
{
    ipport2str(str, ap->addr, ap->port);
}

void str2addrport(struct addrport *dst, char *src)
{
    str2ipport(&dst->addr, &dst->port, src);
}

const char *ipport2a(unsigned int ip, unsigned short port)
{
    static char str[addrport_str_max_len];
    ipport2str(str, ip, port);
    return str;
}

const char *addrport2a(const struct addrport *ap)
{
    return ipport2a(ap->addr, ap->port);
}


void addrport2sockaddr_in(struct sockaddr_in* so, const struct addrport* addr)
{
    so->sin_family = AF_INET;
    so->sin_addr.s_addr = htonl(addr->addr);
    so->sin_port = htons(addr->port);
}

void sockaddr_in2addrport(struct addrport* addr, const struct sockaddr_in* so) 
{
    addr->addr = ntohl(so->sin_addr.s_addr);
    addr->port = ntohs(so->sin_port);
}


