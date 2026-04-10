#ifndef _CRYPDF_H
#define _CRYPDF_H

enum {
    secret_key_size = 64,
    public_key_size = 32,

    kex_secret_size = 32,
    kex_public_size = 32,
    
    node_secret_size = 32,

    shared_secret_size = 32,

    node_id_size = 10,

    rx_key_size,
    tx_key_size = rx_key_size,
};

#endif // !_CRYPDF_H
