#include "ms_headers.h"
#include <endian.h>
#include <sys/time.h>

int  ms_pack_header(struct ms_msg_header* src, void* dst)
{
    char* ptr = dst;
    *((_MS_TYPE_T*)ptr) = htobe16(src->type);
    ptr += sizeof(_MS_TYPE_T);
    *((_LIBLYC_TMST_T*)ptr) = htobe64(src->timestamp);
    ptr += sizeof(_LIBLYC_TMST_T);
    return (ptr - (char*)dst);
}

int  ms_unpack_header(void* src, struct ms_msg_header* dst)
{
    char* ptr = src;
    dst->type = be16toh(*((_MS_TYPE_T*)ptr));
    ptr += sizeof(_MS_TYPE_T);
    dst->timestamp = be64toh(*((_LIBLYC_TMST_T*)ptr));
    ptr += sizeof(_LIBLYC_TMST_T);
    return (ptr - (char*)src);
}

_LIBLYC_TMST_T get_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

