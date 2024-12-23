#ifndef JKSSL_CAST_H
#define JKSSL_CAST_H

#include <stdint.h>

struct evp_cipher;

struct cast_ctx
{
	uint32_t k[32];
};

void cast_encrypt(struct cast_ctx *ctx, uint8_t *out, const uint8_t *in);
void cast_decrypt(struct cast_ctx *ctx, uint8_t *out, const uint8_t *in);
void cast_keyschedule(struct cast_ctx *ctx, const uint8_t *key, uint8_t len);

int cast_init(struct cast_ctx *ctx, const uint8_t *key, const uint8_t *iv);
int cast_update(struct cast_ctx *ctx, uint8_t *out, const uint8_t *in, int enc);
int cast_final(struct cast_ctx *ctx);

#endif
