#ifndef _MS_TXQ_H
#define _MS_TXQ_H


struct ms_peer;
struct ms_transmin_queue;

struct ms_transmit_item {
    struct feda_transmit_queue* master;
    unsigned char* buf;
    int len, offset;
    struct ms_peer* the_peer;
    struct ms_transmit_item* next;
};

struct ms_transmin_queue {
    struct ms_transmit_item *first, *last;
};

void ms_txq_enqueue (struct ms_transmit_item* item);

#endif
