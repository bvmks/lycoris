#ifndef _MS_COMM_CTX_H
#define _MS_COMM_CTX_H

#include "crypdf.h"
#include "ms_nonce.h"

struct ms_node_config;

struct ms_crypto_comm_ctx {
    unsigned char rx_key[rx_key_size];
    unsigned char tx_key[tx_key_size];
    struct ms_nonce nonce;
    struct ms_node_config *node;
};

struct ms_handshake_ctx {
    unsigned char kex_priv[kex_secret_size];
    unsigned char kex_pub[kex_public_size];
    unsigned char kex_pub_recvd[kex_public_size];
    struct ms_node_config *node;
};

#endif
