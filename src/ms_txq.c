#include <stdlib.h>
#include <time.h>
#include <sue/sue_base.h>

#include "ms_txq.h"
#include "ms_peers.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

static void destroy_txitem(struct ms_transmit_item *p)
{
    if(p->tmh) {   /* this means it is registered on the selector */
        sue_sel_remove_timeout(p->the_master->the_selector, p->tmh);
        free(p->tmh);
    }
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

void ms_txq_enqueue (struct ms_transmit_item* item)
{
    struct ms_transmit_queue* txq; 

    txq = item->the_master;
    if(!txq->qfirst)
        txq->qfirst = item;
    else 
        txq->qlast->next = item;
    txq->qlast = item;
    item->next = NULL;
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
fetch_txitem_to_transmit(struct ms_transmit_queue *txq)
{
    struct ms_transmit_item* tmp;
    
    tmp = txq->qfirst;
    if(!tmp)
        return NULL;

    txq->qfirst = tmp->next;
    if(!txq->qfirst)
        txq->qlast = NULL;
    tmp->next = NULL;
    txq->count--;
    return tmp;
}


void txq_peer_gone(struct ms_transmit_queue *txq, struct ms_peer *peer)
{
    struct ms_transmit_item *tmp;
    /*struct ms_transmit_item **pp;*/

    for(tmp = txq->qfirst; tmp; tmp = tmp->next)
        if(tmp->the_peer == peer)
            tmp->the_peer = NULL;
/*
    pp = &txq->retx_first;
    while(*pp) {
        if((*pp)->the_peer == peer) {
            tmp = *pp;
            *pp = (*pp)->next;
            tmp->next = NULL;
            destroy_txitem(tmp);
            if(!txq->retx_first)
                txq->retx_last = NULL;
        } else {
            pp = &(*pp)->next;
        }
    }
*/
}

void ms_txq_item_sent(struct ms_transmit_item* item)
{
    destroy_txitem(item);
}
