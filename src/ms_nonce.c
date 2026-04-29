#include "ms_nonce.h"
#include "ms_keyutils.h"
#include <string.h>

void ms_nonce_init_rand(struct ms_nonce* n)
{
    fill_noise((unsigned char*)n->local, sizeof(n->local));
    ((unsigned char*)n->local)[sizeof(n->local)-1] = 1;
}

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
    nonce->local++;
    return nonce->local;
}

void fill_nounce(struct ms_nonce* nonce, unsigned char* buf)
{
    nonce->local++;
    memcpy(buf, &nonce->local, sizeof(nonce->local));
}
