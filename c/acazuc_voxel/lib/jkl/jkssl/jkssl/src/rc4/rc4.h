#ifndef JKSSL_RC4_H
#define JKSSL_RC4_H

#include <stdint.h>

struct evp_cipher;

struct rc4_ctx
{
	uint8_t state[256];
	uint8_t i;
	uint8_t j;
};

void rc4_init_state(struct rc4_ctx *ctx, const uint8_t *key);
void rc4_operate_block(struct rc4_ctx *ctx, uint8_t *out, const uint8_t *in);

int rc4_init(struct rc4_ctx *ctx, const uint8_t *key, const uint8_t *iv);
int rc4_update(struct rc4_ctx *ctx, uint8_t *out, const uint8_t *in, int enc);
int rc4_final(struct rc4_ctx *ctx);

#endif
