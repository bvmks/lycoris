#ifndef __CRC32_H
#define __CRC32_H

#define CRC32_POLY  0xEDB88320ul
#define CRC32_START 0xFFFFFFFFul
#define CRC32_VALID 0x2144df1cul

/*
*   calculates crc32 hash
*/
unsigned int crc32(const char* buf, int buf_len);

#endif
