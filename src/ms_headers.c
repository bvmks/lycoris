#include "ms_headers.h"
#include <endian.h>
#include <string.h>
#include <sys/time.h>

unsigned long long get_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

