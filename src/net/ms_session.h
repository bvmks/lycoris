#ifndef _MS_SESSION_H
#define _MS_SESSION_H

#include "ms_list.h"

enum {
    ms_session_timeout = 3,
    ms_start_seq = 0
};


enum ms_session_state {
    mss_init,
    mss_handshake,
    mss_cancelling,
    mss_terminated,
};

struct ms_session {
    unsigned int session_id;
    enum ms_session_state state;
    struct node_info* node;

    struct addrport remote_addr;

    unsigned char eph_priv[32];
    unsigned char eph_pub[32];
    unsigned char remote_eph_pub[32];

    unsigned char shared_secret[32];

    unsigned long last_remote_nonce;
    unsigned long local_nonce_counter;

    unsigned long created_at;
    unsigned long last_activity;
};

/*
struct ms_session {
    struct _ms_list recvd, sent;
    unsigned short id;
};
*/

/*
 * used to generate random number 
 * based on key specified
*/
unsigned short generate_id(unsigned int key);

/*
*  used to generate session
*/
void session_init(struct ms_session* session, unsigned short id);

#endif
