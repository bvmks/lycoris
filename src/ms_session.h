#ifndef _MS_SESSION_H
#define _MS_SESSION_H

#include "ms_headers.h"

enum ms_const {
    ms_session_timeout = 3,
    ms_start_seq = 1
};

struct ms_session {
    unsigned short id;
    unsigned short lseq;
    unsigned long  rmask;
};

unsigned short generate_id();

void mss_init_session(struct ms_session* );

/* 
 * checks if message is duplicate within session 
 * if so returns non zero
 */
int mss_check_dup(struct ms_header*, struct ms_session*);

/*
 * marks sequence num of message as received and returns 0
 * if it`s already marked returns 1
 */
int mss_mark_recvd(struct ms_header*, struct ms_session*);

#endif
