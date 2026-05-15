#ifndef _ms_peerION_H
#define _ms_peerION_H

#include "rep.h"

struct ms_peer;
struct ms_peer_collection;
struct peercoll_item;
struct ms_node_cfg;
struct ms_udp_receiver;
struct crypto_comm_ctx;


enum ms_peer_assoc_status {
    as_none,
    as_not_desired,
    as_gave_up,
    as_echo_request_sent,
    as_assoc_request_sent,
    as_assoc_fini_sent,
    as_established,
};

const char* assoc_status_str(int status);

void dbug_print_all_peers(struct ms_peer_collection* col);

void peer_get_addr(const struct ms_peer* peer, unsigned int* ip, unsigned short* port);

int timemark_minutes(const struct ms_peer_collection* col);
unsigned long long timemark_sec(const struct ms_peer_collection* col);

void peer_set_token(struct ms_peer* peer, unsigned long long token);
unsigned long long peer_token(struct ms_peer* peer);

void peer_generate_new_cookie(struct ms_peer* peer);
void peer_set_cookie(struct ms_peer* peer, const unsigned char cookie[8]);
int peer_check_cookie(const struct ms_peer* peer, const unsigned char cookie [8]);
const unsigned char* peer_cookie(struct ms_peer* peer);

const unsigned char* peer_id(struct ms_peer* peer);

int peer_kex_public_is_same(const struct ms_peer* peer,
                            const unsigned char* kex_public);

int peer_set_kex_public(struct ms_peer_collection* col, struct ms_peer* peer,
                        const unsigned char* kex_public, int signchecked);

int peer_should_init_assoc(const struct ms_peer* peer);
int peer_assoc_status(const struct ms_peer* peer);
void peer_set_assoc_status(struct ms_peer* peer, int status);

void peer_set_init_assoc(struct ms_peer* peer);
void peer_reset_init_assoc(struct ms_peer* peer);

const unsigned char* peer_encrypt_key(const struct ms_peer* peer);
const unsigned char* peer_decrypt_key(const struct ms_peer* peer);

int peer_set_identity(struct ms_peer* peer,
                      const unsigned char* node_id,
                      const unsigned char* pubkey);

int node_id_is_same(const struct ms_peer* peer, const unsigned char* id);

unsigned long long peer_get_last_tm(const struct ms_peer* peer);
void peer_set_last_tm(struct ms_peer* peer, unsigned long long ts);

const char* peer_description(const struct ms_peer* peer);

void peers_timer_hook(struct ms_peer_collection* col);

int peer_check_update_nonce(struct ms_peer* peer, const unsigned char* nonce,
                     const char* caller_name);

struct ms_peer_collection* make_peer_collection(struct ms_udp_receiver* rx, 
                                                struct ms_node_cfg* cfg,
                                                struct crypto_comm_ctx* comctx);

struct ms_peer* get_peer_record(struct ms_peer_collection* col,
                                unsigned int ip, unsigned short port, int add);

void peers_report(struct ms_peer_collection *col, report_callback cb, void *ud);

void update_peer_last_rx(struct ms_peer* peer);
void update_peer_last_tx(struct ms_peer* peer);

void peer_get_idle(const struct ms_peer* peer,
                   int* since_last_rx, int* since_last_tx);

#endif
