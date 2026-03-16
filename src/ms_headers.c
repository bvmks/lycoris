#include "ms_headers.h"
#include <endian.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/time.h>

unsigned long long get_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void hton_header(struct ms_header* dst, struct ms_header* src) {
    dst->type = src->type;
    dst->s_id = htons(src->s_id);
    dst->seq = htons(src->seq);
}

void ntoh_header(struct ms_header* dst, struct ms_header* src) {
    dst->type = src->type;
    dst->s_id = ntohs(src->s_id);
    dst->seq = ntohs(src->seq);
}
