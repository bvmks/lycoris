#ifndef _MS_COMM_CTX_H
#define _MS_COMM_CTX_H

#include "crypdf.h"
#include "ms_nonce.h"

struct ms_crypto_comm_ctx {
    unsigned char kex_secret[kex_secret_size];
    unsigned char kex_public[kex_public_size];
    unsigned char remote_kex_public[kex_public_size];
    unsigned char remote_public_key[public_key_size];/* sign key*/
    unsigned char rx_key[cipher_key_size];
    unsigned char tx_key[cipher_key_size];
    struct ms_nonce nonce;
};

/*
 * returs boolean
 */
int comctx_init(struct ms_crypto_comm_ctx* ctx);

void derive_keys(const unsigned char *local_secret,
                 const unsigned char *local_pub_key,
                 const unsigned char *remote_pub_key,
                 unsigned char *encrypt_key,
                 unsigned char *decrypt_key);
 
#endif
