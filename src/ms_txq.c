#include <stdlib.h>

#include "ms_txq.h"
#include "ms_peers.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

struct ms_transmit_queue* make_transmit_queue(struct sue_event_selector* s)
{
    /*TODO*/
    return NULL;
}

void ms_txq_enqueue (struct ms_transmit_item* item)
{
    /*TODO*/
}

int txq_want_write(const struct ms_transmit_queue *txq)
{
    /*TODO*/
    return 0;
}

static struct ms_transmit_item *make_txitem(struct ms_transmit_queue* txq,
                                            int len, int offset)
{
    struct ms_transmit_item* res;

    res = malloc(sizeof(*res));
    res->master = txq;
    res->buf = malloc(len);
    res->len = len;
    res->offset = offset;
    res->ip = -1;
    res->port = -1;
    res->the_peer = NULL;
    res->next = NULL;

    return res;
}


struct ms_transmit_item* make_txitem_4peer(struct ms_transmit_queue* txq,
                                    int len, int offset, struct ms_peer* peer)
{
    struct ms_transmit_item* res;
    res = make_txitem(txq, len, offset);
    ms_peer_getaddr(peer, &res->ip, &res->port);
    res->the_peer = peer;
    return res;
}

struct ms_transmit_item *make_txitem_4ip(struct ms_transmit_queue* txq,
                         int len, int offset, unsigned int ip, unsigned short port)
{
    struct ms_transmit_item* res;
    res = make_txitem(txq, len, offset);
    res->ip = ip;
    res->port = port;
    res->the_peer = NULL;
    return res;
}

struct ms_transmit_item* 
get_item_to_transmit(struct ms_transmit_queue *txq)
{
    /*TODO*/
    return 0;
}

void ms_txq_item_sent(struct ms_transmit_item* item)
{
    /*TODO*/
}
