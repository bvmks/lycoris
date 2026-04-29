#include <stdlib.h>
#include <stdio.h>
#include "sue_aloc.h"

static void *sue_default_malloc(int size)
{
    void *r = malloc(size);
    if(!r) {
        (*sue_malloc_error)();
        return NULL;
    }
    return r;
}

static void sue_default_free(void *p)
{
    free(p);
}

static void sue_default_malloc_error(void)
{
    fprintf(stderr, "sue: memory allocation failed; panic\n");
    exit(1);
}

void sue_alloc_init_default()
{
    sue_malloc = &sue_default_malloc;
    sue_free = &sue_default_free;
    sue_malloc_error = &sue_default_malloc_error;
}
