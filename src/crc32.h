#ifndef _LYC_CRC32_H
#define _LYC_CRC32_H

#define CRC32_POLY  0xEDB88320ul
#define CRC32_START 0xFFFFFFFFul
#define CRC32_VALID 0x2144df1cul

/*
*   calculates crc32 hash
*/
unsigned int crc32(char* buf, int buf_len);

#endif
