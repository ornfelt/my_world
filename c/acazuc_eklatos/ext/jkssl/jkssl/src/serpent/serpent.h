#ifndef JKSSL_SERPENT_H
#define JKSSL_SERPENT_H

#include <stdint.h>

struct serpent_ctx
{
	uint32_t k[33][4];
};

void serpent_encrypt(struct serpent_ctx *ctx, uint8_t *out, const uint8_t *in);
void serpent_decrypt(struct serpent_ctx *ctx, uint8_t *out, const uint8_t *in);
void serpent_keyschedule(struct serpent_ctx *ctx, const uint8_t *key, uint8_t len);

#endif
