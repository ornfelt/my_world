#ifndef JKSSL_RC2_H
#define JKSSL_RC2_H

#include <stdint.h>

struct evp_cipher;

struct rc2_ctx
{
	uint16_t k[64];
};

void rc2_key_expand(struct rc2_ctx *ctx, const uint8_t *key);
void rc2_encrypt(struct rc2_ctx *ctx, uint8_t *out, const uint8_t *in);
void rc2_decrypt(struct rc2_ctx *ctx, uint8_t *out, const uint8_t *in);

int rc2_init(struct rc2_ctx *ctx, const uint8_t *key, const uint8_t *iv);
int rc2_update(struct rc2_ctx *ctx, uint8_t *out, const uint8_t *in, int enc);
int rc2_final(struct rc2_ctx *ctx);

#endif
