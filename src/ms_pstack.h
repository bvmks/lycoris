#ifndef _MS_PSTACK_H
#define _MS_PSTACK_H

#include "ms_packets.h"


enum {
    ms_pstack_basic_size = 128,
};

/*
 *  
 */
struct ms_pstack_node {
    struct ms_received_packet packet;
};

/*
 *  stack for received dgrams
 */
struct ms_pstack {
    struct ms_pstack_node* buf;
    int size;
    int sp; /* a.k.a stack pointer */
};


/*
*  malloc memory for buffer
*/
void pstack_init(struct ms_pstack* stack);

/*
*  frees buffer
*/
void pstack_kill(struct ms_pstack* stack);

/*
 * directry copyes all fields! 
 * a.k.a transfers resources
 */
int pstack_push(struct ms_pstack* stack, struct ms_received_packet* packet);

/* directry copyes all fields! */
int pstack_pop(struct ms_pstack* stack, struct ms_received_packet* packet);

#endif
