#include <time.h>
#include <stdint.h>

unsigned long long get_timestapm_ms() 
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        /*
            very bad but should never happen, i hope
        */
        return 0;
    }
    return (unsigned long long)ts.tv_sec * 1000 + (unsigned long long)ts.tv_nsec / 1000000;
}
