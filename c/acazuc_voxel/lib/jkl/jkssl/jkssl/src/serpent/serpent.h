#ifndef JKSSL_SERPENT_H
#define JKSSL_SERPENT_H

#include <stdint.h>

struct evp_cipher;

struct serpent_ctx
{
	uint32_t k[33][4];
};

void serpent_encrypt(struct serpent_ctx *ctx, uint8_t *out, const uint8_t *in);
void serpent_decrypt(struct serpent_ctx *ctx, uint8_t *out, const uint8_t *in);
void serpent_keyschedule(struct serpent_ctx *ctx, const uint8_t *key, uint8_t len);

int serpent128_init(struct serpent_ctx *ctx, const uint8_t *key, const uint8_t *iv);
int serpent128_update(struct serpent_ctx *ctx, uint8_t *out, const uint8_t *in,
                      int enc);
int serpent128_final(struct serpent_ctx *ctx);
int serpent192_init(struct serpent_ctx *ctx, const uint8_t *key, const uint8_t *iv);
int serpent192_update(struct serpent_ctx *ctx, uint8_t *out, const uint8_t *in,
                      int enc);
int serpent192_final(struct serpent_ctx *ctx);
int serpent256_init(struct serpent_ctx *ctx, const uint8_t *key, const uint8_t *iv);
int serpent256_update(struct serpent_ctx *ctx, uint8_t *out, const uint8_t *in,
                      int enc);
int serpent256_final(struct serpent_ctx *ctx);

#endif
