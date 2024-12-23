#ifndef JKSSL_SM4_H
#define JKSSL_SM4_H

#include <stdint.h>

struct evp_cipher;

struct sm4_ctx
{
	uint32_t rk[32];
};

void sm4_encrypt(struct sm4_ctx *ctx, uint8_t *out, const uint8_t *in);
void sm4_decrypt(struct sm4_ctx *ctx, uint8_t *out, const uint8_t *in);
void sm4_keyschedule(struct sm4_ctx *ctx, const uint8_t *key);

int sm4_init(struct sm4_ctx *ctx, const uint8_t *key, const uint8_t *iv);
int sm4_update(struct sm4_ctx *ctx, uint8_t *out, const uint8_t *in, int enc);
int sm4_final(struct sm4_ctx *ctx);

#endif
