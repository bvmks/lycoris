#include <endian.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/time.h>

#include "ms_headers.h"

unsigned long long get_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void header_init(struct ms_header* h)
{
    h->type.type = mst_err;
    h->type.opt = msc_err;
    h->s_id = 0;
    h->seq = 0;
}

void hton_header(struct ms_header* dst, const struct ms_header* src)
{
    dst->type = src->type;
    dst->s_id = htons(src->s_id);
    dst->seq = htons(src->seq);
}

void ntoh_header(struct ms_header* dst, const struct ms_header* src)
{
    dst->type = src->type;
    dst->s_id = ntohs(src->s_id);
    dst->seq = ntohs(src->seq);
}
