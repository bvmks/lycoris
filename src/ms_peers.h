#ifndef _ms_peerION_H
#define _ms_peerION_H

#include "ms_comm_ctx.h"
#include "addrcol.h"

struct ms_peer {
    struct ms_peer_collection *the_master;
    struct addr_item *the_item;

    unsigned int ip;
    unsigned short port;

    struct ms_crypto_comm_ctx comctx;

    char init_assoc;
    int assoc_status;
    unsigned char id[node_id_size];
    unsigned char public_key[public_key_size];/* remote sign key*/
    unsigned long long last_cookie;

    unsigned long long created_at;
    unsigned long long last_rx;
    unsigned long long last_tx;
};

struct peercoll_item {
    struct ms_peer *peer;
    struct peercoll_item *next;
};

struct ms_peer_collection {
    struct ms_node_cfg *the_conf;
    struct ms_udp_receiver *the_rx;
    struct addr_collection cooldown, peers;
    struct peercoll_item *permpeer_first;
    unsigned long long count;
};

enum ms_peer_assoc_status {
    msas_none,
    msas_not_needed,
    msas_gave_up,
    msas_echo_req_sent,
    msas_assoc_req_sent,
    msas_assoc_fini_sent,
    msas_established,
    msas_terminated,
};



void ms_peer_getaddr(const struct ms_peer* p, unsigned int* ip, unsigned short* port);

void ms_peer_set_cookie(struct ms_peer* p, unsigned long long cookie);
unsigned long long ms_peer_get_cookie(struct ms_peer* p);

const unsigned char* ms_peer_get_id(struct ms_peer* p);
const unsigned char* ms_peer_get_kex(struct ms_peer* p);

int peer_set_kex_public(struct ms_peer_collection *coll, struct ms_peer *peer,
                        const unsigned char *kex_public, int signchecked);

void ms_peer_fill_nounce(struct ms_peer* p, unsigned char* n);

int ms_peer_should_init_assoc(const struct ms_peer* p);
int ms_peer_assoc_status(const struct ms_peer* p);
void ms_peer_set_assoc_status(struct ms_peer* p, int status);

const char* ms_peer_description(const struct ms_peer* p);

void peers_timer_hook(struct ms_peer_collection *coll);

struct ms_peer_collection* make_peer_collection(struct ms_udp_receiver* node, struct ms_node_cfg* cfg);

struct ms_peer* get_peer_record(struct ms_peer_collection* coll,
                                unsigned int ip, unsigned short port, int add);

void update_peer_last_rx(struct ms_peer *fp);
void update_peer_last_tx(struct ms_peer *fp);

void ms_peer_get_idle(const struct ms_peer *fp,
                        int *since_last_rx, int *since_last_tx);

#endif
