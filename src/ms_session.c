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
    session->lseq = 0;
    session->rmask = 0;
}

int mss_check_dup(unsigned short seq, struct ms_session* session)
{
    short diff = (short)(seq - session->lseq);
    if (diff > 0)
        return 0;
    else if (-diff >= (short)(sizeof(session->rmask) * 8))
        return -1;
    if (session->rmask & (1ULL << -diff))
        return 1;
    else
        return 0;
}

int mss_add_recvd(unsigned short seq, struct ms_session* session) 
{
    short diff = (short)(seq - session->lseq);
    unsigned long long mask;
    if (diff > 0) {
        if (diff >= (short)(sizeof(session->rmask) * 8)) {
            session->rmask = 1ULL;
        }
        else {
            session->rmask <<= diff;
            session->rmask |= 1ULL;
        }
        session->lseq = seq;
        return 0;
    }

    if (-diff >= (short)(sizeof(session->rmask) * 8))
        return -1;

    mask = (1ULL << -diff);
    if (session->rmask & mask)
        return 1;
    else {
        session->rmask |= mask;
        return 0;
    }
}
