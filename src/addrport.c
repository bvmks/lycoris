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

int str2ipport(unsigned int *ip, unsigned short *port, const char* str)
{
    static char str_ip[sizeof("255.255.255.255")], str_port[sizeof("65535")];
    if(sscanf(str, "%15[0-9.]:%5s", str_ip, str_port) != 2)
        return 0;
    if(!str2ip(ip, str_ip))
        return 0;
    if(!str2port(port, str_port))
        return 0;
    return 1;
}

int addrport_equal(const struct addrport* a, const struct addrport* b)
{
    return(a->addr == b->addr && a->port == b->port);
}

int str2ip(unsigned int *ip, const char* str)
{
    unsigned char b0, b1, b2, b3;
    if (sscanf(str, "%hhu.%hhu.%hhu.%hhu", &b0, &b1, &b2, &b3) != 4)
        return 0;

    *ip = ((unsigned int)b0 << 24) | 
          ((unsigned int)b1 << 16) | 
          ((unsigned int)b2 <<  8) | 
          (unsigned int)b3;
    return 1;
}

/* bool */
int str2port(unsigned short *port, const char* str)
{
    unsigned short p;

    if (sscanf(str, "%hu", &p) != 1) 
        return 0;
    *port = p;
    return 1;
}

void addrport2str(char *str, const struct addrport *ap)
{
    ipport2str(str, ap->addr, ap->port);
}

void str2addrport(struct addrport *dst, char *src)
{
    str2ipport(&(dst->addr), &(dst->port), src);
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


void ipport2mem(unsigned char mem[6], unsigned int ip, unsigned short port)
{
    mem[0] = (ip >> 24)  & 0xff;
    mem[1] = (ip >> 16)  & 0xff;
    mem[2] = (ip >>  8)  & 0xff;
    mem[3] = ip          & 0xff;
    mem[4] = (port >> 8) & 0xff;
    mem[5] = port        & 0xff;
}

void mem2ipport(const unsigned char mem[6],
                unsigned int *ip, unsigned short *port)
{
    *ip =
        ((unsigned int)mem[0] << 24) |
        ((unsigned int)mem[1] << 16) |
        ((unsigned int)mem[2] << 8)  |
        ((unsigned int)mem[3]);
    *port =
        ((unsigned int)mem[4] << 8)  |
        ((unsigned int)mem[5]);
}
