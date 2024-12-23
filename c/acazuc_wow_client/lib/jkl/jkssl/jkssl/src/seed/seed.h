#ifndef JKSSL_SEED_H
#define JKSSL_SEED_H

#include <stdint.h>

struct evp_cipher;

struct seed_ctx
{
	uint64_t k[16];
};

void seed_keyschedule(struct seed_ctx *seed_ctx, const uint8_t *key);
void seed_encrypt(struct seed_ctx *ctx, uint8_t *out, const uint8_t *in);
void seed_decrypt(struct seed_ctx *ctx, uint8_t *out, const uint8_t *in);

int seed_init(struct seed_ctx *ctx, const uint8_t *key, const uint8_t *iv);
int seed_update(struct seed_ctx *ctx, uint8_t *out, const uint8_t *in, int enc);
int seed_final(struct seed_ctx *ctx);

#endif
