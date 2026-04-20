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

#endif
