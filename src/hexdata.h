#ifndef _MS_HEXDATA_H
#define _MS_HEXDATA_H

char hexdigit(unsigned int n);
void hexbyte2str(char str[3], int bt);
void hexdata2str(char *str, const unsigned char *data, int datalen);

    /* returns internal static buffer, 4096 bytes in size; if the
       buffer's length is insufficient, the hex data is truncated
       accordingly (to 4095 chars, which is odd, heh)
     */
const char *hexdata2a(const unsigned char *data, int datalen);



unsigned int u32_from_big_endian(const unsigned char d[4]);
unsigned long long u64_from_big_endian(const unsigned char d[8]);

void u32_to_big_endian(unsigned char mem[4], unsigned int num);
void u64_to_big_endian(unsigned char mem[8], unsigned long long num);

unsigned int u32_from_little_endian(const unsigned char d[4]);
unsigned long long u64_from_little_endian(const unsigned char d[8]);

void u32_to_little_endian(unsigned char mem[4], unsigned int num);
void u64_to_little_endian(unsigned char mem[8], unsigned long long num);

#endif
