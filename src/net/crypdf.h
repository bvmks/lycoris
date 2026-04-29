#ifndef _CRYPDF_H
#define _CRYPDF_H

enum {
    secret_key_size = 64,
    public_key_size = 32,
    
    cookish_size = 32,
    cookie_lifetime = 30, /* seconds */

    shared_secret_size = 32,
    kex_secret_size = 32,
    kex_public_size = 32,
    cipher_key_size = 32,
    
    node_secret_size = 32,


    node_id_size = 10,

};

#endif // !_CRYPDF_H
