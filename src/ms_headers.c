#include <endian.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "ms_headers.h"

unsigned long long get_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


static int _get_ctrl_h_size(unsigned char o)
{
    switch (o) {  
        case msc_e_req: return ms_ereq_header_size;
        case msc_e_rep: return ms_erep_header_size;
        default:        return ms_gen_header_size;
    }
}



int _get_header_size(unsigned char t, unsigned char o)
{
    switch (t) {  
        case mst_ctrl:   return _get_ctrl_h_size(o);
        case mst_post:   return ms_post_header_size;
        case mst_stream: return ms_stream_header_size;
        default:         return ms_gen_header_size;
    }

}

int get_header_size(const struct ms_header* h)
{
    return _get_header_size(h->type.t, h->type.o);
}


void header_copy(struct ms_header** dst, const struct ms_header* src)
{
    if(*dst == src) 
        return;
    unsigned long size = get_header_size(src);
    header_free(*dst);
    *dst = malloc(size);
    memcpy(*dst, src, size);
}

void header_move(struct ms_header** dst, struct ms_header* src) {
    if(*dst == src)
        return;
    *dst = src;
}

void header_free(struct ms_header* h)
{
    free(h);
}

struct ms_header* header_make(unsigned char type, unsigned char opt)
{
    struct ms_header* h = malloc(_get_header_size(type, opt));
    header_init(h);
    return h;
}

void header_init(struct ms_header* h)
{
    h->type.t = mst_err;
    h->type.o = msc_err;
}


static int _gen2mem(void* dst, const struct ms_header* src) {
    unsigned char* out = (unsigned char*)dst;
    out[0] = src->type.t;
    out[1] = src->type.o;
    return 2; /* sizeof(struct ms_header_type) */
}

static int _post2mem(void* dst, const struct ms_header* src)
{
    int payload = 0;
    const struct ms_post_header* h = (const struct ms_post_header*)src;

    payload += _gen2mem(dst, src);

    unsigned short sid_net = htons(h->s_id);
    unsigned short seq_net = htons(h->seq);

    memcpy((char*)dst + payload, &sid_net, 2);
    payload += 2;
    memcpy((char*)dst + payload, &seq_net, 2);
    payload += 2;

    return payload;
}

static int _ctrl2mem(void* dst, const struct ms_header* src)
{
    /*
    *   ADD LATER!
    */
    return 0;
}

static int _stream2mem(void* dst, const struct ms_header* src)
{
    /*
    *   ADD LATER!
    */
    return 0;
}

int header2mem(void* dst, const struct ms_header* src)
{
    int payload = 0;
    switch (src->type.t) {
        case mst_ctrl: {
            payload += _ctrl2mem(dst + payload, src);
            break;
        } 
        case mst_post: {
            payload += _post2mem(dst + payload, src);
            break;
        }
        case mst_stream: {
            payload += _stream2mem(dst + payload, src);
            break;
        }
    }

    return payload;
}

static int _mem2post(struct ms_header* dst, const void* src) {
    struct ms_post_header* h = (struct ms_post_header*)dst;
    const unsigned char* in = (const unsigned char*)src;
    
    unsigned short sid_net, seq_net;
    memcpy(&sid_net, in + 2, 2);
    memcpy(&seq_net, in + 4, 2);

    h->s_id = ntohs(sid_net);
    h->seq  = ntohs(seq_net);
    
    return sizeof(struct ms_header) + 2 * sizeof(unsigned short); 
}

int mem2header(struct ms_header* dst, const void* src)
{
    unsigned char* in = (unsigned char*)src;
    
    dst->type.t = in[0];
    dst->type.o = in[1];

    switch (dst->type.t) {
        case mst_post:    return _mem2post(dst, src);
        case mst_ctrl:    return sizeof(struct ms_header);
        case mst_stream:  return sizeof(struct ms_header);
        default:          return sizeof(struct ms_header);
    }
}


const char* ms_pc(unsigned char ctrl)
{
    if(ctrl > ms_msg_ctrl_types_num)
        return "unknown";
    #define _MS_T_IN(name) #name,
    static const char* list[] = {_MS_CTRL_TYPES_LIST};
    #undef _MS_T_IN
    return list[ctrl];
}

const char* ms_pt(unsigned char type)
{
    if(type > ms_msg_types_num)
        return "unknown";
    #define _MS_T_IN(name) #name,
    static const char* list[] = {_MS_TYPES_LIST};
    #undef _MS_T_IN
    return list[type];
}

char* ms_type2a(struct ms_header_type type)
{
    static char buf[ptype_buf_len];
    if(type.t == mst_ctrl) {
        sprintf(buf, "%s: %s", ms_pt(type.t), ms_pc(type.o));
    }
    else {
        sprintf(buf, "%s", ms_pt(type.t));
    }
    return buf;
}
