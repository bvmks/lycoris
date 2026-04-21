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

struct ms_peer {
    unsigned int ip;
    unsigned short port;

    int state;

    struct ms_crypto_comm_ctx comctx;

    char init_assoc;
    unsigned char id[node_id_size];
    unsigned char public_key[public_key_size];/* remote sign key*/



    unsigned long long created_at;
    unsigned long long last_rx;
    unsigned long long last_tx;
};


struct ms_peer* make_peer();
void dispose_peer(struct ms_peer* s);

void ms_peer_init(struct ms_peer* s);



struct ms_peer_el {
    struct ms_peer *peer;
    struct ms_peer_el *next;
};

struct ms_peer_collection {
    struct ms_peer_el *head;
    size_t count;
};

void ms_sess_coll_init(struct ms_peer_collection *coll);

int  ms_sess_coll_add(struct ms_peer_collection *coll, struct ms_peer *s);
void ms_sess_coll_remove(struct ms_peer_collection *coll, struct ms_peer *s);

struct ms_peer* ms_sess_coll_find(struct ms_peer_collection *coll, const struct addrport *addr);

void ms_sess_coll_cleanup(struct ms_peer_collection *coll);

int mss_sess_get_num_of_el(struct ms_peer_collection* coll);


struct ms_session {
    struct _ms_list recvd, sent;
    unsigned short id;
};

void session_init(struct ms_session* session, unsigned short id);

#endif
