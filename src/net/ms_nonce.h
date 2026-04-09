#ifndef _MS_SWINDOW_H
#define _MS_SWINDOW_H

struct ms_nonce {
    unsigned long long next_to_send; 

    unsigned long long max_received;
    unsigned long long received_mask;
};

/* 
 * checks if nounce is duplicate
 * if so returns 1, otherwise 0
 */
int ms_nounce_chdup(const struct ms_nonce* nounce, unsigned long long n);

/* 
 * marks nounce as received,
 */
void ms_nounce_mark(struct ms_nonce* nounce, unsigned long long n);

unsigned long long ms_nonce_get_next(struct ms_nonce*);


#endif // !_MS_SWINDOW_H
