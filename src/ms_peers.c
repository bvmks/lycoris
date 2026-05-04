#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

#include "ms_peers.h"
#include "ms_comm_ctx.h"

struct ms_peer {
    unsigned int ip;
    unsigned short port;

    int state;

    struct ms_crypto_comm_ctx comctx;

    char init_assoc;
    unsigned char id[node_id_size];
    unsigned char public_key[public_key_size];/* remote sign key*/
    unsigned long long last_cookie;

    unsigned long long created_at;
    unsigned long long last_rx;
    unsigned long long last_tx;
};

struct ms_peer_el {
    struct ms_peer *peer;
    struct ms_peer_el *next;
};

struct ms_peer_collection {
    struct ms_peer_el *head;
    size_t count;
};


void ms_peer_init(struct ms_peer* p)
{
    memset(p->id, 0, node_id_size);
    memset(p->public_key, 0, node_id_size);
    p->ip = INADDR_ANY;
    p->port = -1;
    p->last_cookie = -1;
    p->state = msas_not_needed;
    p->init_assoc = 0;
    p->created_at = 0;
    p->last_rx = 0;
    p->last_tx = 0;
    comctx_init(&p->comctx);
}

void ms_peer_getaddr(const struct ms_peer* p, unsigned int* ip, unsigned short* port)
{
    if(ip) 
        *ip = p->ip;
    if(port) 
        *port = p->port;
}


struct ms_peer_collection* make_peer_collection(struct ms_udp_receiver* node, struct ms_node_cfg* cfg)
{
    struct ms_peer_collection* col;
    col = malloc(sizeof(*col));
    col->count = 0;
    col->head = NULL;
    return col;
}

struct ms_peer* get_peer_record(struct ms_peer_collection* coll,
                         unsigned int ip, unsigned short port, int add)
{
    return NULL;
}

void dispose_peer_collection(struct ms_peer_collection *coll)
{
    free(coll);
}


void ms_peer_coll_cleanup(struct ms_peer_collection *coll) {

}

