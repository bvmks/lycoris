#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "crc32.h"

int ms_crc32_check(char *buf, int buf_len)
{
    return CRC32_VALID != crc32(buf, buf_len);
}


