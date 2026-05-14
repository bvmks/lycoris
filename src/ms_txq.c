#include <stdlib.h>
#include <time.h>
#include <sue/sue_base.h>

#include "ms_txq.h"
#include "ms_peers.h"
#include "message.h"
#include "addrport.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

static void destroy_txitem(struct ms_transmit_item *p)
{
    free(p->buf);
    free(p);
}


struct ms_transmit_queue *make_transmit_queue(struct sue_event_selector *s)
{
    struct ms_transmit_queue *res;

    res = malloc(sizeof(*res));
    res->the_selector = s;
    res->starttime = time(NULL);
    res->curtime = 0;
    res->qfirst = NULL;
    res->qlast = NULL;

    return res;
}

void txq_enqueue (struct ms_transmit_item* item)
{
    struct ms_transmit_queue *txq = item->the_master;
    if(txq->qfirst)
        txq->qlast->next = item;
    else
        txq->qfirst = item;
    txq->qlast = item;
    item->next = NULL;
    if(item->the_peer) {
        update_peer_last_tx(item->the_peer);
    }
    txq->count++;
}

int txq_want_write(const struct ms_transmit_queue *txq)
{
    return txq->qfirst != NULL;
}

static struct ms_transmit_item *make_txitem(struct ms_transmit_queue* txq,
                                            int len, int offset)
{
    struct ms_transmit_item* res;

    res = malloc(sizeof(*res));
    res->the_master = txq;
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
    peer_getaddr(peer, &res->ip, &res->port);
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
fetch_item_to_transmit(struct ms_transmit_queue* txq)
{
    struct ms_transmit_item *tmp;
    int mlen;

    tmp = txq->qfirst;
    if(!tmp) {
        message(mlv_alert, "[ERROR] ready to send, but nothing to send\n");
        return NULL;
    }
    txq->qfirst = tmp->next;
    if(!txq->qfirst)
        txq->qlast = NULL;

    mlen = tmp->len - tmp->offset;
    message(mlv_debug2, "[DEBUG] going to send %d bytes to %s\n",
                    mlen, ipport2a(tmp->ip, tmp->port));
    return tmp;
}

void txq_peer_gone(struct ms_transmit_queue *txq, struct ms_peer *peer)
{
    struct ms_transmit_item *tmp;
    struct ms_transmit_item **pp;
    struct ms_transmit_item *prev = NULL;

    pp = &txq->qfirst;
    while(*pp) {
        if((*pp)->the_peer == peer) {
            tmp = *pp;
            *pp = (*pp)->next;
            if (tmp == txq->qlast) {
                txq->qlast = prev;
            }
            tmp->next = NULL;
            destroy_txitem(tmp);
            txq->count--;
        } else {
            prev = *pp;
            pp = &(*pp)->next;
        }
    }
    if (!txq->qfirst) {
        txq->qlast = NULL;
    }
}

void txq_item_sent(struct ms_transmit_item* item)
{
    destroy_txitem(item);
}
