#include "evp/encode.h"

#include <jkssl/evp.h>

#include <string.h>
#include <stdlib.h>

static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static uint8_t get_next_val(struct evp_encode_ctx *ctx, const uint8_t **in,
                            size_t *inl)
{
	uint8_t ret;
	if (ctx->tmp_len)
	{
		ret = ctx->tmp[0];
		for (size_t i = 1; i < ctx->tmp_len; ++i)
			ctx->tmp[i - 1] = ctx->tmp[i];
		ctx->tmp_len--;
		return ret;
	}
	ret = **in;
	(*in)++;
	(*inl)--;
	return ret;
}

static void encode_chunk(uint8_t *out, const uint8_t *in)
{
	out[0] = alphabet[in[0] >> 2];
	out[1] = alphabet[((in[0] & 0x3) << 4) | (in[1] >> 4)];
	out[2] = alphabet[((in[1] & 0xf) << 2) | (in[2] >> 6)];
	out[3] = alphabet[in[2] & 0x3f];
}

static void encode_final(uint8_t *out, const uint8_t *in, size_t inl)
{
	if (inl == 1)
	{
		out[0] = alphabet[in[0] >> 2];
		out[1] = alphabet[(in[0] & 0x3) << 4];
		out[2] = '=';
		out[3] = '=';
	}
	else if (inl == 2)
	{
		out[0] = alphabet[in[0] >> 2];
		out[1] = alphabet[((in[0] & 0x3) << 4) | (in[1] >> 4)];
		out[2] = alphabet[(in[1] & 0xf) << 2];
		out[3] = '=';
	}
}

struct evp_encode_ctx *evp_encode_ctx_new(void)
{
	return calloc(sizeof(struct evp_encode_ctx), 1);
}

void evp_encode_ctx_free(struct evp_encode_ctx *ctx)
{
	if (!ctx)
		return;
	free(ctx);
}

void evp_encode_init(struct evp_encode_ctx *ctx)
{
	ctx->tmp_len = 0;
	ctx->count = 0;
}

int evp_encode_update(struct evp_encode_ctx *ctx, uint8_t *out,
                      size_t *outl, const uint8_t *in, size_t inl)
{
	size_t org = *outl;
	while (*outl >= 4 && inl + ctx->tmp_len >= 3)
	{
		uint8_t vals[3];
		vals[0] = get_next_val(ctx, &in, &inl);
		vals[1] = get_next_val(ctx, &in, &inl);
		vals[2] = get_next_val(ctx, &in, &inl);
		encode_chunk(out, vals);
		*outl -= 4;
		out += 4;
		ctx->count += 4;
		if (ctx->count >= 64)
		{
			if (*outl)
			{
				*(out++) = '\n';
				(*outl)--;
			}
			ctx->count -= 64;
		}
	}
	if (inl > 0)
	{
		if (inl + ctx->tmp_len > 3u)
			return 0;
		memcpy(&ctx->tmp[ctx->tmp_len], in, inl);
		ctx->tmp_len += inl;
	}
	*outl = org - *outl;
	return 1;
}

void evp_encode_final(struct evp_encode_ctx *ctx, uint8_t *out, size_t *outl)
{
	if (ctx->ended)
	{
		*outl = 0;
		return;
	}
	ctx->ended = 1;
	if (!ctx->tmp_len)
	{
		if (*outl && ctx->count)
		{
			*out = '\n';
			*outl = 1;
		}
		else
		{
			*outl = 0;
		}
		return;
	}
	if (*outl < 5)
	{
		*outl = 0;
		return;
	}
	encode_final(out, ctx->tmp, ctx->tmp_len);
	out[4] = '\n';
	*outl = 5;
	return;
}

int evp_encode_block(uint8_t *out, const uint8_t *in, size_t inl)
{
	size_t ret = (inl + 2) / 3 * 4;
	while (inl >= 3)
	{
		encode_chunk(out, in);
		out += 4;
		in += 3;
		inl -= 3;
	}
	encode_final(out, in, inl);
	*out = '\0';
	return ret;
}
