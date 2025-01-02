#ifndef JKSSL_BF_H
#define JKSSL_BF_H

#include <stdint.h>

struct bf_ctx
{
	uint32_t p[18];
	uint32_t s[4][256];
};

void bf_encrypt(struct bf_ctx *ctx, uint8_t *out, const uint8_t *in);
void bf_decrypt(struct bf_ctx *ctx, uint8_t *out, const uint8_t *in);
void bf_keyschedule(struct bf_ctx *ctx, const uint8_t *key, uint8_t len);

#endif
