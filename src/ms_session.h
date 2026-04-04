#ifndef _MS_SESSION_H
#define _MS_SESSION_H

#include "ms_list.h"

enum {
    ms_session_timeout = 3,
    ms_start_seq = 0
};

struct ms_session {
    struct _ms_list recvd, sent;
    unsigned short id; /* session identifier */
};

unsigned short generate_id(unsigned int key);

void session_init(struct ms_session* session, unsigned short id);

#endif
