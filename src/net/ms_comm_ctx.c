#include "../../lib/monocypher/monocypher.h"
#include "ms_comm_ctx.h"
#include "ms_keyutils.h"
#include <string.h>

int comctx_init(struct ms_crypto_comm_ctx *ctx)
{
    int res;

    memset(ctx->encrypt_key, 0, cipher_key_size);
    memset(ctx->decrypt_key, 0, cipher_key_size);
    memset(ctx->remote_kex_public, 0, cipher_key_size);

    res = get_random(&ctx->kex_secret, sizeof(ctx->kex_secret));
    if(!res)
        return 0;

    crypto_x25519_public_key(ctx->kex_public, ctx->kex_secret);
    
    ms_nonce_init_rand(&ctx->nonce);

    return 1;
}

void derive_keys(const unsigned char *local_secret,
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

int get_plain_dgram_cmd(const unsigned char *dgram)
{
    unsigned char uc;

    if(*dgram < 0xE0)
        return -1;

    uc = (dgram[0] & 0x0f) | ((dgram[0] << 4) & 0xf0);
    return dgram[1] ^ uc;
}

