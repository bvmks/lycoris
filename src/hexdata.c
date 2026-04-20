#include "hexdata.h"

char hexdigit(unsigned int n)
{
    if(n <= 9)
        return n + '0';
    if(n <= 15)
        return n - 10 + 'a';
    return '?'; /* should never happen */
}

void hexbyte2str(char str[3], int bt)
{
    str[0] = hexdigit((bt >> 4) & 0x0f);
    str[1] = hexdigit(bt        & 0x0f);
    str[2] = 0;
}

void hexdata2str(char *str, const unsigned char *data, int datalen)
{
    int i;
    char *p = str;
    for(i = 0; i < datalen; i++) {
        hexbyte2str(p, data[i]);
        p += 2;
    }
}

const char *hexdata2a(const unsigned char *data, int datalen)
{
    static char buf[4096];
    if(datalen * 2 + 1 > sizeof(buf))
        datalen = (sizeof(buf) - 1) / 2;
    hexdata2str(buf, data, datalen);
    return buf;
}
