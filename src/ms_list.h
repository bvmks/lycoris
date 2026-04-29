#ifndef _MS_LIST_H
#define _MS_LIST_H

#include "ms_pqueue.h"

struct ms_mask {
    unsigned long long mask; /* mask for last 64 packages*/
    unsigned short last_seq; /* sequence number of last package */
};

struct _ms_list{
    struct ms_mask mask;
    struct ms_pqueue queue;
};

void list_init(struct _ms_list* list);

/*
 * puts packet into list, sets last_seq and shifts mask accordingly.
 * returns:
 * 0 on success,
 * 1 if packet already in list,
 * -1 on error (packet seq older than 64),
 */
int list_push(struct _ms_list* list, struct ms_packet* p);

/*
 * checks if packet is in list.
 * returns:
 * 1 if list in,
 * 0 otherwise,
 * -1 on error (packet seq older than 64)
 */
int list_mask_check(const struct _ms_list* list, const struct ms_packet* p);

/*
 * returns pointer to packet on specified position
 */
struct ms_packet* list_peek(const struct _ms_list* list, unsigned int num);


/* 
 * checks mask if seq is duplicate.
 * if so returns 1, otherwise 0, on error returns -1
 * error means message have sequence num older than 64, and cannot be checked
 */
int ms_mask_check(const struct ms_mask* mask, unsigned short seq);

/*
 * marks sequence num in mask
 * if it`s already marked returns 1, otherwise 0, on error returns -1
 * error means message have sequence num older than 64, and cannot be marked
 */
int ms_mask_add(struct ms_mask* mask, unsigned short seq);

int ms_mask_inc(struct ms_mask* mask);

#endif
