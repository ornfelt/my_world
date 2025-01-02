#ifndef JKSSL_SEED_H
#define JKSSL_SEED_H

#include <stdint.h>

struct seed_ctx
{
	uint64_t k[16];
};

void seed_keyschedule(struct seed_ctx *seed_ctx, const uint8_t *key);
void seed_encrypt(struct seed_ctx *ctx, uint8_t *out, const uint8_t *in);
void seed_decrypt(struct seed_ctx *ctx, uint8_t *out, const uint8_t *in);

#endif
