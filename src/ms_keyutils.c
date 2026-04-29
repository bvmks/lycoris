#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ms_keyutils.h"

int get_random(void *buf, int len)
{
    int fd, rc, cnt;
    fd = open("/dev/urandom", O_RDONLY);
    if(fd == -1)
        return 0;
    cnt = 0;
    while(cnt < len) {
        rc = read(fd, ((char*)buf) + cnt, len - cnt);
        if(rc < 1) {
            close(fd);
            return 0;
        }
        cnt += rc;
    }
    close(fd);
    return 1;
}

#define RAND_STEP \
    u = u * 2862933555777941757LL + 7046029254386353087LL; \
    v ^= v >> 17; \
    v ^= v << 31; \
    v ^= v >> 8;  \
    w = 4294957665U*(w & 0xffffffff) + (w >> 32);

static unsigned long long get_rand_longlong()
{
    static int seeded = 0;
    static unsigned long long u, v, w;
    unsigned long long x;

    if(!seeded) {
        unsigned long long start;
        int res;
        res = get_random(&start, sizeof(start));
        if(!res)  /* fallback for paranoia; we don't want to fail here */
            start = time(NULL) ^ (getpid() << 16);

        v = 4101842887655102017LL;
        w = 1;
        u = start ^ v; 
        RAND_STEP
        v = u; 
        RAND_STEP
        w = v; 
        /* RAND_STEP */
        seeded = 1;
    }

    RAND_STEP

    x = u ^ (u << 21);                   
    x ^= x >> 35;
    x ^= x << 4;
    return (x + v) ^ w;
}

#undef RAND_STEP


void fill_noise(unsigned char *mem, int len)
{
    enum { buf_items = 256 };
    static unsigned long long buf[buf_items];

    int items_count, partlen, i;

    items_count = (len + sizeof(*buf) - 1) / sizeof(*buf);
    partlen = len;
    if(items_count > buf_items) {
        items_count = buf_items;
        partlen = sizeof(buf);
    }
    for(i = 0; i < items_count; i++)
        buf[i] = get_rand_longlong();
    memcpy(mem, buf, partlen);

    if(partlen < len)
        fill_noise(mem + partlen, len - partlen);
}


int rand_from_range(int first, int last)
{
    unsigned long n;
    n = get_rand_longlong();
    return first + (n % (last - first + 1));
}
