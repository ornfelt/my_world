#ifndef JKSSL_AES_H
#define JKSSL_AES_H

#include <stdint.h>

struct aes_ctx
{
	uint8_t keys[30][16];
	uint8_t key_len;
};

void aes_encrypt(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in);
void aes_decrypt(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in);
void aes_keyschedule(struct aes_ctx *ctx, const uint8_t *key, uint8_t len);
void aesni_encrypt(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in);
void aesni_decrypt(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in);
void aesni_keyschedule(struct aes_ctx *ctx, const uint8_t *key, uint8_t len);
void aes_neon_encrypt(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in);
void aes_neon_decrypt(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in);
void aes_neon_keyschedule(struct aes_ctx *ctx, const uint8_t *key, uint8_t len);

#endif
