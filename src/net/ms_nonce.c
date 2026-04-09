#include "ms_nonce.h"

int ms_nonce_chdup(const struct ms_nonce* nonce, unsigned long long n)
{
    unsigned long long diff;
    if (n > nonce->max_received)
        return 0; 
    diff  = nonce->max_received - n;
    if (diff >= 64 || (nonce->received_mask & (1ULL << diff)))
        return 1;
    return 0;
}

void ms_nonce_mark(struct ms_nonce* nonce, unsigned long long n)
{
    unsigned long long diff;

    if (n > nonce->max_received) {
        diff = n - nonce->max_received;
        if (diff < 64)
            nonce->received_mask <<= diff;
        else 
            nonce->received_mask = 0;
        nonce->received_mask |= 1ULL;
        nonce->max_received = n;
    } else {
        diff = nonce->max_received - n;
        nonce->received_mask |= (1ULL << diff);
    }
}

unsigned long long ms_nonce_get_next(struct ms_nonce * nonce)
{
    unsigned long long next = nonce->next_to_send;
    nonce->next_to_send++;
    return next;
}
