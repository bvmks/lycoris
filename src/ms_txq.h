#ifndef _MS_TXQ_H
#define _MS_TXQ_H

#include "crypdf.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

enum { 
    min_retry_time = 10,
    reset_time = token_lifetime,
};

struct ms_peer;
struct ms_transmit_queue;
struct sue_event_selector;

struct ms_transmit_item {
    struct ms_transmit_queue* master;
    unsigned char* buf;
    int len, offset;
    unsigned int ip;
    unsigned short port;
    struct ms_peer* the_peer;

    struct ms_transmit_item* next;
};

struct ms_transmit_queue {
    struct sue_event_selector *the_selector;
    struct ms_transmit_item *qfirst, *qlast;
    int count;
    long long starttime;
    long long curtime;
};

struct ms_transmit_queue* 
make_transmit_queue(struct sue_event_selector* sel);

int txq_want_write(const struct ms_transmit_queue* txq);

void txq_peer_gone(struct ms_transmit_queue *txq, struct ms_peer *peer);

void txq_enqueue(struct ms_transmit_item* item);

struct ms_transmit_item*
fetch_item_to_transmit(struct ms_transmit_queue* txq);

struct ms_transmit_item* 
make_txitem_4peer(struct ms_transmit_queue* txq,
                  int len, int offset, 
                  struct ms_peer* peer);

struct ms_transmit_item* 
make_txitem_4ip(struct ms_transmit_queue* txq,
                int len, int offset,
                unsigned int ip, unsigned short port);

void txq_item_sent(struct ms_transmit_item* item);

#endif
