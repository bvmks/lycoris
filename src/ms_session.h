#ifndef _MS_SESSION_H
#define _MS_SESSION_H

enum {
    ms_session_timeout = 3,
    ms_start_seq = 0
};

struct ms_session {
    struct ms_mask {
        unsigned long long mask; /* mask for last 64 packages*/
        unsigned short last_seq; /* sequence number of last package */
    } mask_conf, mask_recvd;
    unsigned short id; /* session identifyer */
};

unsigned short generate_id(unsigned int key);

void mss_init_session(struct ms_session* session, unsigned int skey);

/* 
 * checks mask if seq is duplicate 
 * if so returns 1, otherwise 0, on error returns -1
 * error means message have sequence num older than 64, and cannot be checked
 */
int ms_mask_check(unsigned short seq, struct ms_mask* mask);

/*
 * marks sequence num in mask
 * if it`s already marked returns 1, otherwise 0, on error returns -1
 * error means message have sequence num older than 64, and cannot be marked
 */
int ms_mask_add(unsigned short seq, struct ms_mask* mask);

#endif
