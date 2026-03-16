#include "ms_pstack.h"
#include <stdlib.h>

void init_pstack(struct ms_pstack* stack)
{
    stack->buf = malloc(sizeof(struct ms_pstack_entry) * ms_pstack_basic_size);
    stack->size = ms_pstack_basic_size;
    stack->sp = 0;
}

void delete_pstack(struct ms_pstack* stack)
{
    free(stack->buf);
}
