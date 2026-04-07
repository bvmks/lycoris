#include "ms_list.h"

void list_init(struct _ms_list* list)
{
    list->mask.last_seq = 0;
    list->mask.mask = 0;
    pqueue_init(&list->queue);
}

int ms_mask_check(unsigned short seq, struct ms_mask* mask)
{
    short diff = (short)(seq - mask->last_seq);
    if (diff > 0)
        return 0;
    else if (-diff >= (short)(sizeof(mask->mask) * 8))
        return -1;
    if (mask->mask & (1ULL << -diff))
        return 1;
    else
        return 0;
}
int ms_mask_add(unsigned short seq, struct ms_mask* mask) {
    short diff = (short)(seq - mask->last_seq);
    unsigned long long m;
    if (diff > 0) {
        if (diff >= (short)(sizeof(mask->mask) * 8)) {
            mask->mask = 1ULL;
        }
        else {
            mask->mask <<= diff;
            mask->mask |= 1ULL;
        }
        mask->last_seq = seq;
        return 0;
    }

    if (-diff >= (short)(sizeof(mask->mask) * 8))
        return -1;

    m = (1ULL << -diff);
    if (mask->mask & m)
        return 1;
    else {
        mask->mask |= m;
        return 0;
    }
}

int ms_mask_inc(struct ms_mask* mask)
{
    mask->mask <<= 1;
    mask->last_seq++;
    return 0;
}
