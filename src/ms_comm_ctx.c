#include <stdlib.h>
#include <string.h>

#include <monocypher/monocypher.h>

#include "ms_comm_ctx.h"
#include "ms_nodeid.h"
#include "keyutils.h"
#include "fileutil.h"

int comctx_init(struct crypto_comm_ctx *ctx)
{
    int res;

    res = get_random(&ctx->kex_secret, sizeof(ctx->kex_secret));
    if(!res)
        return 0;
    res = get_random(&ctx->token_key, sizeof(ctx->token_key));
    if(!res)
        return 0;


    fill_noise(ctx->nonce_counter, sizeof(ctx->nonce_counter));
    ctx->nonce_counter[sizeof(ctx->nonce_counter)-1] = 1;

    crypto_x25519_public_key(ctx->kex_public, ctx->kex_secret);

    ctx->identity = NULL;

    return 1;
}

int comctx_init_node(struct crypto_comm_ctx* ctx, const char* dir)
{
    int res;
    char* idfile;
    ctx->identity = malloc(sizeof(*ctx->identity));

    idfile = concat_path(dir, "node.id");

    res = load_nodeid_file(ctx->identity, idfile);
    free(idfile);
    if(res) {
        dispose_nodeid(ctx->identity);
        return 0;
    }
    return 1;
}

void derive_cipher_keys(const unsigned char *local_secret,
                 const unsigned char *local_pub_key,
                 const unsigned char *remote_pub_key,
                 unsigned char *encrypt_key,
                 unsigned char *decrypt_key)
{
    unsigned char s[shared_secret_size + 2*kex_public_size];
    unsigned char *shared_secret = s + 2*kex_public_size;

    crypto_x25519(shared_secret, local_secret, remote_pub_key);

    if(encrypt_key) {
        memcpy(s,                 remote_pub_key, kex_public_size);
        memcpy(s+kex_public_size, local_pub_key,  kex_public_size);
        crypto_blake2b(encrypt_key, cipher_key_size, s, sizeof(s));
    }

    if(decrypt_key) {
        memcpy(s,                 local_pub_key,  kex_public_size);
        memcpy(s+kex_public_size, remote_pub_key, kex_public_size);
        crypto_blake2b(decrypt_key, cipher_key_size, s, sizeof(s));
    }

    crypto_wipe(s, sizeof(s));
}


void set_plain_dgram_head(unsigned char *dgram, int cmd)
{
    unsigned char uc;

    dgram[0] = rand_from_range(ms_zb_plain_min, ms_zb_plain_max);
    uc = dgram[0] & 0x0f;
    uc |= (uc << 4) & 0xf0;
    dgram[1] = cmd ^ uc;
}

void comctx_fill_nonce(struct crypto_comm_ctx* ctx, unsigned char* buf)
{
    increment_buf(ctx->nonce_counter, sizeof(ctx->nonce_counter));
    memcpy(buf, ctx->nonce_counter, sizeof(ctx->nonce_counter));
}

int get_plain_dgram_cmd(const unsigned char *dgram)
{
    unsigned char uc;

    if(*dgram < 0xE0)
        return -1;

    uc = (dgram[0] & 0x0f) | ((dgram[0] << 4) & 0xf0);
    return dgram[1] ^ uc;
}

