#include "ms_list.h"

void list_init(struct _ms_list* list)
{
    list->mask.last_seq = 0;
    list->mask.mask = 0;
    pqueue_init(&list->queue);
}



int list_push(struct _ms_list* list, struct ms_packet* p) 
{
    int res;
    struct ms_packet filler;
    unsigned short cur_last, seq;
    seq = ((struct ms_post_header*)p->header)->seq;
    cur_last = list->mask.last_seq;
    res = ms_mask_add(&list->mask, seq);
    if(res == 0) {
        packet_init(&filler);
        for(; seq - cur_last > 1; cur_last++) {
            pqueue_push(&list->queue, &filler);
        }
        pqueue_push(&list->queue, p);
    }

    return res;
}


int list_mask_check(const struct _ms_list* list, const struct ms_packet* p)
{
    unsigned short seq = ((struct ms_post_header*)p->header)->seq;
    return ms_mask_check(&list->mask, seq);
}

struct ms_packet* list_peek(const struct _ms_list* list, unsigned int num)
{
    return pqueue_peek(&list->queue, num);
}


int ms_mask_check(const struct ms_mask* mask, unsigned short seq)
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

int ms_mask_add(struct ms_mask* mask, unsigned short seq)
{
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
