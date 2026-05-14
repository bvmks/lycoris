#ifndef _MS_HEXDATA_H
#define _MS_HEXDATA_H

char hexdigit(unsigned int n);
void hexbyte2str(char str[3], int bt);

void hexdata2str(char *str, const unsigned char *data, int datalen);
const char *hexdata2a(const unsigned char *data, int datalen);


int hexstr2data(unsigned char *data, int datasize, const char *str);



unsigned int u32_from_big_endian(const unsigned char d[4]);
unsigned long long u64_from_big_endian(const unsigned char d[8]);

void u32_to_big_endian(unsigned char mem[4], unsigned int num);
void u64_to_big_endian(unsigned char mem[8], unsigned long long num);

unsigned int u32_from_little_endian(const unsigned char d[4]);
unsigned long long u64_from_little_endian(const unsigned char d[8]);

void u32_to_little_endian(unsigned char mem[4], unsigned int num);
void u64_to_little_endian(unsigned char mem[8], unsigned long long num);

#endif
