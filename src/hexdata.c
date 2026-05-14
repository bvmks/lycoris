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

int hexdigval(char dig)
{
    if(dig >= '0' && dig <= '9')
        return dig - '0';
    if(dig >= 'a' && dig <= 'f')
        return dig - 'a' + 10;
    if(dig >= 'A' && dig <= 'F')
        return dig - 'A' + 10;
    return -1;
}

static int iswhitespace(int c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

int hexstr2data(unsigned char *data, int datasize, const char *str)
{
    int dest, i, half, val;
    dest = 0;
    half = 0;
    for(i = 0; str[i]; i++) {
        char c = str[i];
        int dig;
        if(iswhitespace(c))
            continue;
        dig = hexdigval(c);
        if(dig == -1)
            return -(i+1);
        val = !half ? ((dig << 4) & 0xf0) : (val | (dig & 0x0f));
        if(half) {
            if(dest < datasize)
                data[dest] = val;
            dest++;
        }
        half = !half;
    }
    return half ? -(i+1) : dest;
}

unsigned int u32_from_big_endian(const unsigned char d[4])
{
    return
        ((unsigned int)d[0] << 24) |
        ((unsigned int)d[1] << 16) |
        ((unsigned int)d[2] <<  8) |
        ((unsigned int)d[3]);
}

unsigned long long u64_from_big_endian(const unsigned char d[8])
{
    unsigned long long res;
    res = u32_from_big_endian(d);
    res <<= 32;
    res |= u32_from_big_endian(d + 4);
    return res;
}

void u32_to_big_endian(unsigned char mem[4], unsigned int num)
{
    mem[0] = (num >> 24) & 0xff;
    mem[1] = (num >> 16) & 0xff;
    mem[2] = (num >>  8) & 0xff;
    mem[3] = num         & 0xff;
}

void u64_to_big_endian(unsigned char mem[8], unsigned long long num)
{
    int i;
    for(i = 0; i < 8; i++) {
        mem[7-i] = num & 0xFF;
        num >>= 8;
    }
}



unsigned int u32_from_little_endian(const unsigned char d[4])
{
    return
        ((unsigned int)d[3] << 24) |
        ((unsigned int)d[2] << 16) |
        ((unsigned int)d[1] <<  8) |
        ((unsigned int)d[0]);
}

unsigned long long u64_from_little_endian(const unsigned char d[8])
{
    unsigned long long res;
    res = u32_from_little_endian(d + 4);
    res <<= 32;
    res |= u32_from_little_endian(d);
    return res;
}

void u32_to_little_endian(unsigned char mem[4], unsigned int num)
{
    mem[0] = num         & 0xff;
    mem[1] = (num >>  8) & 0xff;
    mem[2] = (num >> 16) & 0xff;
    mem[3] = (num >> 24) & 0xff;
}

void u64_to_little_endian(unsigned char mem[8], unsigned long long num)
{
    int i;
    for(i = 0; i < 8; i++) {
        mem[i] = num & 0xFF;
        num >>= 8;
    }
}
