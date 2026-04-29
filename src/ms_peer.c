#include <stdlib.h>
#include <string.h>

#include "ms_peer.h"

struct ms_peer* make_peer()
{
    struct ms_peer* s;
    s = malloc(sizeof(*s));
    memset(s->id, 0, node_id_size);
    memset(s->public_key, 0, node_id_size);
    s->state = ms_us_stub;
    s->init_assoc = 0;
    s->created_at = 0;
    s->last_rx = 0;
    s->last_tx = 0;
    s->ip = INADDR_ANY;
    s->port = -1;
    comctx_init(&s->comctx);
    return s;
}

void dispose_peer(struct ms_peer* s)
{
    free(s);
}


struct ms_peer_collection* make_peer_collection(struct ms_node* node, struct ms_node_cfg* cfg)
{
    struct ms_peer_collection* col;
    col = malloc(sizeof(*col));
    col->count = 0;
    col->head = NULL;
    return col;
}

struct ms_peer* get_peer(struct ms_peer_collection* coll,
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

