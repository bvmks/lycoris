#ifndef _MS_SWINDOW_H
#define _MS_SWINDOW_H

struct ms_nonce {
    unsigned long long local; 

    unsigned long long max_received;
    unsigned long long received_mask;
};

void ms_nonce_init_rand(struct ms_nonce* n);

/* 
 * checks if nounce is duplicate
 * if so returns 1, otherwise 0
 */
int ms_nonce_chdup(const struct ms_nonce* nonce, unsigned long long n);

/* 
 * marks nounce as received,
 */
void ms_nonce_mark(struct ms_nonce* nonce, unsigned long long n);

unsigned long long ms_nonce_get_next(struct ms_nonce*);

/*
 *  fills memory with nounce
 */
void fill_nounce(struct ms_nonce* nonce, unsigned char* buf);

#endif // !_MS_SWINDOW_H
