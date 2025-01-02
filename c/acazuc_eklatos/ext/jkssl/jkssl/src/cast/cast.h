#ifndef JKSSL_CAST_H
#define JKSSL_CAST_H

#include <stdint.h>

struct cast_ctx
{
	uint32_t k[32];
};

void cast_encrypt(struct cast_ctx *ctx, uint8_t *out, const uint8_t *in);
void cast_decrypt(struct cast_ctx *ctx, uint8_t *out, const uint8_t *in);
void cast_keyschedule(struct cast_ctx *ctx, const uint8_t *key, uint8_t len);

#endif
