#ifndef _MS_LIST_H
#define _MS_LIST_H

#include "ms_pqueue.h"

struct _ms_list{
    struct ms_mask {
        unsigned long long mask; /* mask for last 64 packages*/
        unsigned short last_seq; /* sequence number of last package */
    } mask;
    struct ms_pqueue queue;
};

void mss_init_list(struct _ms_list* list);

/* 
 * checks mask if seq is duplicate 
 * if so returns 1, otherwise 0, on error returns -1
 * error means message have sequence num older than 64, and cannot be checked
 */
int ms_mask_check(unsigned short seq, struct ms_mask* mask);

/*
 * marks sequence num in mask
 * if it`s already marked returns 1, otherwise 0, on error returns -1
 * error means message have sequence num older than 64, and cannot be marked
 */
int ms_mask_add(unsigned short seq, struct ms_mask* mask);

int ms_mask_inc(struct ms_mask* mask);

#endif
