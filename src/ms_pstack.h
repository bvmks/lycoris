#ifndef _MS_PSTACK_H
#define _MS_PSTACK_H

#include "ms_packets.h"


enum {
    ms_pstack_basic_size = 128,
};

/*
 *  
 */
struct ms_pstack_entry {
    struct ms_packet packet;
};

/*
 *  stack for received dgrams
 */
struct ms_pstack {
    struct ms_pstack_entry* buf;
    int size;
    int sp; /* a.k.a stack pointer */
};


/*
*  malloc memory for buffer
*/
void init_pstack(struct ms_pstack* stack);


/*
*  frees buffer
*/
void delete_pstack(struct ms_pstack* stack);

#endif
