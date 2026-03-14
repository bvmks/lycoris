#ifndef _MS_SESSION_H
#define _MS_SESSION_H

#include "ms_headers.h"

enum ms_const {
    ms_session_timeout = 3,
    ms_start_seq = 0
};

struct ms_session {
    /* session identifyer */
    unsigned short id;
    /* sequence number of last received package */
    unsigned short lseq;
    /* mask for last 64 received packages*/
    unsigned long long rmask;
};

unsigned short generate_id(unsigned int key);

void mss_init_session(struct ms_session* session, unsigned int skey);

/* 
 * checks if message is duplicate within session 
 * if so returns 1, otherwise 0, on error returns -1
 * error means message have sequence num older than 64, and cannot be checked
 */
int mss_check_dup(unsigned short seq, struct ms_session* session);

/*
 * marks sequence num of message as received in given session
 * if it`s already marked returns 1, otherwise 0, on error returns -1
 * error means message have sequence num older than 64, and cannot be marked
 */
int mss_mark_recvd(unsigned short seq, struct ms_session* session);

#endif
