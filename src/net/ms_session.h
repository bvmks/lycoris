#ifndef _MS_SESSION_H
#define _MS_SESSION_H

#include "ms_list.h"

#include "addrport.h"
#include "ms_comm_ctx.h"
#include "ms_nonce.h"

enum {
    ms_session_timeout = 3,
    ms_start_nonce = 0,


    mss_tcp,
    mss_udp,

    mss_server,
    mss_client,
};


enum mss_authtype {
    mss_trust, /* we just exchange kex keys, and believe all good*/
    mss_pass,  /* we use some shared pass (or like global id), to sign kex keys */
    mss_known, /* we have previously established connection(with 1 or 2 auth type)
                  and exchange locals ids witch each other, so now we can require 
                  this type of auth
                */
    mss_cert,  /* Probably won't be implemented, this when known node signed us 
                  some id, so we can auth with it
                */
};

enum mss_state {
    ms_us_stub,
    ms_us_init,
    ms_us_handshake,
    ms_us_active,
    ms_us_cancelling,
    ms_us_terminated,
};

struct ms_udp_session {
    struct addrport remote_addr;
    enum mss_state state;
    char side; /*we are on server or client side*/
    enum mss_authtype auth_type;
    struct ms_crypto_comm_ctx comctx;

    unsigned long long created_at;
    unsigned long long last_rx;
    unsigned long long last_tx;
};



struct ms_session {
    struct _ms_list recvd, sent;
    unsigned short id;
};


struct ms_udp_session* make_udp_session();
void dispose_udp_session(struct ms_udp_session* s);

void ms_udp_session_init(struct ms_udp_session* s);


void session_init(struct ms_session* session, unsigned short id);

#endif
