#ifndef _ms_peerION_H
#define _ms_peerION_H


#include "crypdf.h"
struct ms_peer;
struct ms_node_cfg;
struct ms_udp_receiver;

enum ms_peer_assoc_status {
    msas_none,
    msas_not_needed,
    msas_echo_req_sent,
    msas_assoc_req_sent,
    msas_assoc_fini_sent,
    msas_established,
    msas_terminated,
};



void ms_peer_init(struct ms_peer* p);

void ms_peer_getaddr(const struct ms_peer* p, unsigned int* ip, unsigned short* port);

void ms_peer_set_cookie(struct ms_peer* p, unsigned long long cookie);
unsigned long long ms_peer_get_cookie(struct ms_peer* p);

const unsigned char* ms_peer_get_id(struct ms_peer* p);
const unsigned char* ms_peer_get_kex(struct ms_peer* p);

void ms_peer_fill_nounce(struct ms_peer* p, unsigned char n[nonce_used]);

int ms_peer_assoc_status(const struct ms_peer* p);
void ms_peer_set_assoc_status(struct ms_peer* p, int status);
int ms_peer_should_init_assoc(const struct ms_peer* p);

const char* ms_peer_description(const struct ms_peer* p);


struct ms_peer_collection* make_peer_collection(struct ms_udp_receiver* node, struct ms_node_cfg* cfg);

struct ms_peer* get_peer_record(struct ms_peer_collection* coll,
                                unsigned int ip, unsigned short port, int add);

void dispose_peer_collection(struct ms_peer_collection *coll);


#endif
