#include "sue_aloc.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

void * (*sue_malloc)(int) = NULL;
void (*sue_free)(void *) = NULL;
void (*sue_malloc_error)(void) = NULL;
