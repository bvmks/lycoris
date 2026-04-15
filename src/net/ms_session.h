#ifndef _MS_SESSION_H
#define _MS_SESSION_H

#include "ms_list.h"

#include "addrport.h"
#include "ms_crypto_ctx.h"
#include "ms_nonce.h"

enum {
    ms_session_timeout = 3,
    ms_start_nonce = 0,


    mss_tcp,
    mss_udp,

    mss_server,
    mss_client,
};

enum ms_us_state {
    ms_us_stub,
    ms_us_init,
    ms_us_handshake,
    ms_us_active,
    ms_us_cancelling,
    ms_us_terminated,
};

struct ms_udp_session {
    int sockfd;
    struct addrport remote_addr;
    enum ms_us_state state;

    int side; /* a.k.a we are server or client */
    struct ms_handshake_ctx* hctx;

    unsigned char rx_key[rx_key_size];
    unsigned char tx_key[tx_key_size];
    struct ms_nonce nonce;

    unsigned long created_at;
    unsigned long last_activity;
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
