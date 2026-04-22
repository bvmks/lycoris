#ifndef _MS_PQUEUE_H
#define _MS_PQUEUE_H

#include "ms_packets.h"

enum {
    pqueue_init_size = 128,
    pqueue_max_size = 64
};

struct ms_pqueue_node {
    struct ms_packet packet;
    struct ms_pqueue_node* next;
};

/*
 * represents queue of packets
 */
struct ms_pqueue {
    struct ms_pqueue_node* head;
    struct ms_pqueue_node* tail;
    unsigned int size;
};

/*
 * inits the queue
*/
void pqueue_init(struct ms_pqueue* queue);


/*
 * deletes the queue
 * and frees memory
*/
void pqueue_free(struct ms_pqueue* queue);

/*
 * inserts new element at the end of queue
*/
int  pqueue_push(struct ms_pqueue* queue, const struct ms_packet* src);

/*
 * used to pop element from the start of queue
*/
int  pqueue_pop (struct ms_pqueue* queue, struct ms_packet* src);


/*
 * used to get queue element at specified position.
 * returns -1 if pos was specified incorrectly
*/
int pqueue_get(const struct ms_pqueue* queue, struct ms_packet* dst, 
               unsigned int pos);

/*
 * used to get pointer to queue element at specified position.
 * returns NULL if pos was specified incorrectly
*/
struct ms_packet* pqueue_peek(const struct ms_pqueue* queue, 
                              unsigned int pos);

#endif
