#ifndef _MS_COMM_CTX_H
#define _MS_COMM_CTX_H

#include "crypdf.h"
#include "ms_nodeid.h"

struct crypto_comm_ctx {
    unsigned char kex_secret[kex_secret_size];
    unsigned char kex_public[kex_public_size];
    unsigned char nonce_counter[cipher_nonce_used];
    unsigned char token_key[token_key_size];
    struct ms_nodeid_file* identity;
};

/*
 * returs boolean
 */
int comctx_init(struct crypto_comm_ctx* ctx);

int comctx_init_node(struct crypto_comm_ctx* ctx, const char* dir);

void comctx_fill_nonce(struct crypto_comm_ctx* ctx, unsigned char* buf);

void derive_cipher_keys(const unsigned char *local_secret,
                        const unsigned char *local_pub_key,
                        const unsigned char *remote_pub_key,
                        unsigned char *encrypt_key,
                        unsigned char *decrypt_key);

enum {
    ms_min_dgram         = 64,
    ms_max_dgram         = 508,
    ms_min_payload       = 40,
    ms_max_payload       = ms_max_dgram 
                         - 1 /* zero byte */
                         - cipher_nonce_used 
                         - cipher_mac_size
                         - 1, /* real cmd*/

    ms_zb_enc_min        = 0x00,
    ms_zb_enc_max        = 0xE0,       
    ms_zb_plain_min      = ms_zb_enc_max + 1,       
    ms_zb_plain_max      = 0xFF,       

    ms_cmd_echo_request  = 0xEC,         /* Commands */
    ms_cmd_echo_reply    = 0xED,
    ms_cmd_assoc_request = 0xA5,
    ms_cmd_assoc_fini    = 0xAA,
    ms_cmd_intro_request = 0x1C,
    ms_cmd_intro_reply   = 0x1D,
    ms_cmd_error         = 0xEE,

    ms_cmd_change_key    = 0xCC,

    ms_cmd_keep_alive    = 0x2C,        /* encrypted */
    ms_cmd_im_alive      = 0x2D,

    ms_cmd_data          = 0xDA,

};

const char *ms_err_diags(int code);

void set_plain_dgram_head(unsigned char *dgram, int cmd);

int get_plain_dgram_cmd(const unsigned char *dgram);
 
#endif
