#include "ms_pstack.h"
#include <stdlib.h>

void pstack_init(struct ms_pstack* stack)
{
    stack->buf = malloc(sizeof(struct ms_pstack_node) * ms_pstack_basic_size);
    stack->size = ms_pstack_basic_size;
    stack->sp = -1;
}

void pstack_free(struct ms_pstack* stack)
{
    int i = 0;
    for(;i < stack->sp; i++) {
        packet_free(&(stack->buf[i].packet.packet));
    }
    free(stack->buf);
    stack->buf = NULL;
    stack->size = 0;
    stack->sp = -1;
}


int pstack_push(struct ms_pstack* stack, const struct ms_received_packet* packet)
{
    if(stack->sp + 1 >= stack->size)
        return 1;
    stack->sp++;
    stack->buf[stack->sp].packet = *packet;
    return 0;
}

int pstack_pop(struct ms_pstack* stack, struct ms_received_packet* packet) 
{
    if(stack->sp < 0)
        return 1;
    *packet = stack->buf[stack->sp].packet;
    stack->sp--;
    return 0;
}

