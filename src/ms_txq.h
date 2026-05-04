#ifndef _MS_TXQ_H
#define _MS_TXQ_H

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
    struct ms_transmit_item *first, *last;
};

struct ms_transmit_queue* 
make_transmit_queue(struct sue_event_selector* s);

void ms_txq_enqueue(struct ms_transmit_item* item);

int txq_want_write(const struct ms_transmit_queue* txq);

struct ms_transmit_item* 
get_item_to_transmit(struct ms_transmit_queue *txq);

void ms_txq_item_sent(struct ms_transmit_item* item);

struct ms_transmit_item* 
make_txitem_4peer(struct ms_transmit_queue* txq,
                  int len, int offset, 
                  struct ms_peer* peer);

struct ms_transmit_item* 
make_txitem_4ip(struct ms_transmit_queue* txq,
                int len, int offset,
                unsigned int ip, unsigned short port);

#endif
