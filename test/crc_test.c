#include "../src/crc32.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char buf[1024];

int main(int argc, char *argv[])
{
    srand(time(NULL));
    unsigned int i, crc, crc2;
    for(i = 0; i < 1020; i++) {
        buf[i] = rand() % 125 + 1;
    }
    *((unsigned int*)(buf+1020)) = 0;
    crc = crc32(buf, 1020);
    *((unsigned int*)(buf+1020)) = crc;
    crc2 = crc32(buf, 1024);
    printf("%x\n", crc2);
}
