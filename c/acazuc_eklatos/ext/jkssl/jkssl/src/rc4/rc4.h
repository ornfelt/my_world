#ifndef JKSSL_RC4_H
#define JKSSL_RC4_H

#include <stdint.h>

struct rc4_ctx
{
	uint8_t state[256];
	uint8_t i;
	uint8_t j;
};

void rc4_init_state(struct rc4_ctx *ctx, const uint8_t *key);
void rc4_operate_block(struct rc4_ctx *ctx, uint8_t *out, const uint8_t *in);

#endif
