#ifndef JKSSL_EVP_CIPHER_INT_H
#define JKSSL_EVP_CIPHER_INT_H

#include <jkssl/evp.h>

#include <stdint.h>
#include <stddef.h>

#define EVP_CIPHER_MOD_FLAG_NOPAD (1 << 0)
#define EVP_CIPHER_MOD_FLAG_IV    (1 << 1)

struct evp_cipher_ctx;

typedef int (*evp_cipher_init_t)(void *ctx, const uint8_t *key,
                                 const uint8_t *iv);
typedef int (*evp_cipher_update_t)(void *ctx, uint8_t *out, const uint8_t *in,
                                   int enc);
typedef int (*evp_cipher_final_t)(void *ctx);
typedef int (*evp_cipher_mod_fn_t)(struct evp_cipher_ctx *ctx, uint8_t *out,
                                   const uint8_t *in);

struct evp_cipher_mod
{
	evp_cipher_mod_fn_t update;
	uint32_t flags;
};

struct evp_cipher
{
	const char *name;
	evp_cipher_init_t init;
	evp_cipher_update_t update;
	evp_cipher_final_t final;
	const struct evp_cipher_mod *mod;
	uint32_t block_size;
	uint32_t key_size;
	uint32_t ctx_size;
	const uint32_t *oid;
	size_t oid_size;
};

struct evp_cipher_ctx
{
	const struct evp_cipher *evp_cipher;
	void *ctx;
	uint32_t buf_pos;
	uint8_t *buf;
	uint8_t *mod1;
	uint8_t *mod2;
	uint8_t ended;
	uint8_t enc;
};

extern const struct evp_cipher_mod g_evp_mod_ecb64;
extern const struct evp_cipher_mod g_evp_mod_ecb128;
extern const struct evp_cipher_mod g_evp_mod_ecb_iv;
extern const struct evp_cipher_mod g_evp_mod_ecb_nopad;
extern const struct evp_cipher_mod g_evp_mod_ecb_nopad_iv;
extern const struct evp_cipher_mod g_evp_mod_cbc64;
extern const struct evp_cipher_mod g_evp_mod_cbc128;
extern const struct evp_cipher_mod g_evp_mod_pcbc64;
extern const struct evp_cipher_mod g_evp_mod_pcbc128;
extern const struct evp_cipher_mod g_evp_mod_cfb64;
extern const struct evp_cipher_mod g_evp_mod_cfb128;
extern const struct evp_cipher_mod g_evp_mod_ofb64;
extern const struct evp_cipher_mod g_evp_mod_ofb128;
extern const struct evp_cipher_mod g_evp_mod_ctr64;
extern const struct evp_cipher_mod g_evp_mod_ctr128;
extern const struct evp_cipher_mod g_evp_mod_gcm128;

#endif
