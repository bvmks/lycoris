#include <stdlib.h>

#include "ms_session.h"

unsigned short generate_id(unsigned int key)
{
    unsigned short ip_key = (unsigned short)(key ^ (key >> 16));
    return ip_key ^ (unsigned short)rand();
}

void mss_init_session(struct ms_session* session, unsigned int skey)
{
    session->id = generate_id(skey);
    session->mask_conf.mask = 0;
    session->mask_conf.last_seq = 0;
    session->mask_recvd.mask = 0;
    session->mask_recvd.last_seq = 0;
}

int ms_mask_check(unsigned short seq, struct ms_mask* mask)
{
    short diff = (short)(seq - mask->last_seq);
    if (diff > 0)
        return 0;
    else if (-diff >= (short)(sizeof(mask->mask) * 8))
        return -1;
    if (mask->mask & (1ULL << -diff))
        return 1;
    else
        return 0;
}

int ms_mask_add(unsigned short seq, struct ms_mask* mask) 
{
    short diff = (short)(seq - mask->last_seq);
    unsigned long long m;
    if (diff > 0) {
        if (diff >= (short)(sizeof(mask->mask) * 8)) {
            mask->mask = 1ULL;
        }
        else {
            mask->mask <<= diff;
            mask->mask |= 1ULL;
        }
        mask->last_seq = seq;
        return 0;
    }

    if (-diff >= (short)(sizeof(mask->mask) * 8))
        return -1;

    m = (1ULL << -diff);
    if (mask->mask & m)
        return 1;
    else {
        mask->mask |= m;
        return 0;
    }
}
