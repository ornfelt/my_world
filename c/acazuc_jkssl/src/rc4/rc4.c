#include "rc4/rc4.h"

#include <stddef.h>

void rc4_init_state(struct rc4_ctx *ctx, const uint8_t *key)
{
	ctx->i = 0;
	ctx->j = 0;
	for (size_t i = 0; i < 256; ++i)
		ctx->state[i] = i;
	uint8_t j = 0;
	for (size_t i = 0; i < 256; ++i)
	{
		j += ctx->state[i] + key[i % 16];
		uint8_t tmp = ctx->state[i];
		ctx->state[i] = ctx->state[j];
		ctx->state[j] = tmp;
	}
}

void rc4_operate_block(struct rc4_ctx *ctx, uint8_t *out, const uint8_t *in)
{
	ctx->i++;
	ctx->j += ctx->state[ctx->i];
	uint8_t tmp = ctx->state[ctx->i];
	ctx->state[ctx->i] = ctx->state[ctx->j];
	ctx->state[ctx->j] = tmp;
	tmp = ctx->state[ctx->i] + ctx->state[ctx->j];
	*out = *in ^ ctx->state[tmp];
}
