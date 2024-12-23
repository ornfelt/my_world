#ifndef JKSSL_ARIA_H
#define JKSSL_ARIA_H

#include <stdint.h>

struct evp_cipher;

struct aria_ctx
{
	uint8_t ekeys[17][16];
	uint8_t dkeys[17][16];
	uint8_t key_len;
};

void aria_encrypt(struct aria_ctx *ctx, uint8_t *out, const uint8_t *in);
void aria_decrypt(struct aria_ctx *ctx, uint8_t *out, const uint8_t *in);
void aria_keyschedule(struct aria_ctx *ctx, const uint8_t *key, const uint8_t len);

int aria128_init(struct aria_ctx *ctx, const uint8_t *key, const uint8_t *iv);
int aria128_update(struct aria_ctx *ctx, uint8_t *out, const uint8_t *in,
                   int enc);
int aria128_final(struct aria_ctx *ctx);
int aria192_init(struct aria_ctx *ctx, const uint8_t *key, const uint8_t *iv);
int aria192_update(struct aria_ctx *ctx, uint8_t *out, const uint8_t *in,
                   int enc);
int aria192_final(struct aria_ctx *ctx);
int aria256_init(struct aria_ctx *ctx, const uint8_t *key, const uint8_t *iv);
int aria256_update(struct aria_ctx *ctx, uint8_t *out, const uint8_t *in,
                   int enc);
int aria256_final(struct aria_ctx *ctx);

#endif
