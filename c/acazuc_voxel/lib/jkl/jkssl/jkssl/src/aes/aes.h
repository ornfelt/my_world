#ifndef JKSSL_AES_H
#define JKSSL_AES_H

#include <stdint.h>

struct evp_cipher;

struct aes_ctx
{
	uint8_t keys[30][16];
	uint8_t key_len;
};

void aes_encrypt(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in);
void aes_decrypt(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in);
void aes_keyschedule(struct aes_ctx *ctx, const uint8_t *key, uint8_t len);

int aes128_init(struct aes_ctx *ctx, const uint8_t *key, const uint8_t *iv);
int aes128_update(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in, int enc);
int aes128_final(struct aes_ctx *ctx);
int aes192_init(struct aes_ctx *ctx, const uint8_t *key, const uint8_t *iv);
int aes192_update(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in, int enc);
int aes192_final(struct aes_ctx *ctx);
int aes256_init(struct aes_ctx *ctx, const uint8_t *key, const uint8_t *iv);
int aes256_update(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in, int enc);
int aes256_final(struct aes_ctx *ctx);

#endif
