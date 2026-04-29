#ifndef _MS_COMM_CTX_H
#define _MS_COMM_CTX_H

#include "crypdf.h"
#include "ms_nonce.h"

struct ms_crypto_comm_ctx {
    unsigned char kex_secret[kex_secret_size];
    unsigned char kex_public[kex_public_size];
    unsigned char remote_kex_public[kex_public_size];
    unsigned char decrypt_key[cipher_key_size];
    unsigned char encrypt_key[cipher_key_size];
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

enum {
    ms_min_dgram         = 64,
    ms_max_dgram         = 508,
    ms_min_payload       = 40,

    ms_zv_enc_min        = 0x00,
    ms_zb_enc_max        = 0xE0,       
    ms_zb_plain_min      = ms_zb_enc_max + 1,       
    ms_zb_plain_max      = 0xFF,       

    ms_cmd_echo_req      = 0xEC,         /* Commands */
    ms_cmd_echo_reply    = 0xED,
    ms_cmd_assoc_req     = 0xA5,
    ms_cmd_assoc_fini    = 0xAA,
    ms_cmd_intro_req     = 0xA1,
    ms_cmd_intro_reply   = 0x1A,
    ms_cmd_error         = 0xEE,
    


};

const char *ms_err_diags(int code);

void set_plain_dgram_head(unsigned char *dgram, int cmd);

int get_plain_dgram_cmd(const unsigned char *dgram);
 
#endif
