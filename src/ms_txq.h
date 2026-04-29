#ifndef _MS_TXQ_H
#define _MS_TXQ_H

struct ms_peer;
struct ms_transmit_queue;
struct sue_event_selector;

struct ms_transmit_item {
    struct feda_transmit_queue* master;
    unsigned char* buf;
    int len, offset;
    struct ms_peer* the_peer;
    struct ms_transmit_item* next;
};

struct ms_transmit_queue {
    struct ms_transmit_item *first, *last;
};

struct ms_transmit_queue* make_transmit_queue(struct sue_event_selector* s);

void ms_txq_enqueue (struct ms_transmit_item* item);

int txq_want_write(const struct ms_transmit_queue *txq);

#endif
