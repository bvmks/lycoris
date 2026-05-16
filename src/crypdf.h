#ifndef _CRYPDF_H
#define _CRYPDF_H

enum {
    secret_key_size = 64,
    public_key_size = 32,
    
    token_key_size = 32,
    token_size = 8,
    token_lifetime = 30, /* seconds */

    timemark_gap = 10, /* seconds */

    shared_secret_size = 32,
    kex_secret_size = 32,
    kex_public_size = 32,
    cipher_key_size = 32,

    sign_size = 64,

    timemark_size = 8,
    cookie_size = 8,

    cipher_nonce_total = 24,
    cipher_nonce_used = 8,
    cipher_nonce_offset = cipher_nonce_total - cipher_nonce_used,
    cipher_mac_size = 16,
    
    node_secret_size = 32,


    node_id_size = 10,
    node_name_max_size = 30,

};

#endif // !_CRYPDF_H
