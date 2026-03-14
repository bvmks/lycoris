#include <stdio.h>

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

void addrport2str(char *str, const struct addrport *ap)
{
    ipport2str(str, ap->addr, ap->port);
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

