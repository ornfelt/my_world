#include "evp/encode.h"

#include <jkssl/evp.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static const char invtab[] =
{
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55,
	56, 57, 58, 59, 60, 61, -1, -1, -1, 0 , -1, -1, -1,
	0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10, 11, 12,
	13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
	-1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32,
	33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
	46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1
};

static int assemble(struct evp_encode_ctx *ctx, uint8_t *out, size_t *n)
{
	if (ctx->tmp[0] == '=')
		return 0;
	out[(*n)++] = (invtab[ctx->tmp[0]] << 2)
	            | (invtab[ctx->tmp[1]] >> 4);
	if (ctx->tmp[2] != '=')
	{
		out[(*n)++] = (invtab[ctx->tmp[1]] << 4)
		            | (invtab[ctx->tmp[2]] >> 2);
		if (ctx->tmp[3] != '=')
		{
			out[(*n)++] = (invtab[ctx->tmp[2]] << 6)
			             | invtab[ctx->tmp[3]];
		}
		else if (ctx->ended)
		{
			return 0;
		}
		else
		{
			ctx->ended = 1;
		}
	}
	else if (ctx->ended)
	{
		return 0;
	}
	else
	{
		ctx->ended = 1;
	}
	ctx->tmp_len = 0;
	return 1;
}

static int decode_chunk(struct evp_encode_ctx *ctx, const uint8_t **in,
                        size_t *inl)
{
	if (isspace(**in))
	{
		--(*inl);
		++(*in);
		return 1;
	}
	if (invtab[**in] == -1)
		return 0;
	ctx->tmp[ctx->tmp_len++] = **in;
	--(*inl);
	++(*in);
	return 1;
}

void evp_decode_init(struct evp_encode_ctx *ctx)
{
	ctx->tmp_len = 0;
}

int evp_decode_update(struct evp_encode_ctx *ctx, uint8_t *out,
                      size_t *outl, const uint8_t *in, size_t inl)
{
	size_t org = *outl;
	while (*outl >= 3 && inl + ctx->tmp_len >= 4)
	{
		if (ctx->ended)
			return 0;
		if (!decode_chunk(ctx, &in, &inl))
			return 0;
		if (ctx->tmp_len == 4)
		{
			size_t n = 0;
			if (!assemble(ctx, out, &n))
				return 0;
			*outl -= n;
			out += n;
		}
	}
	if (inl > 0)
	{
		while (ctx->tmp_len < 4 && inl)
		{
			if (!decode_chunk(ctx, &in, &inl))
				return 0;
		}
		if (inl)
			return 0;
	}
	*outl = org - *outl;
	return 1;
}

int evp_decode_final(struct evp_encode_ctx *ctx, uint8_t *out, size_t *outl)
{
	(void)out;
	*outl = 0;
	if (ctx->tmp_len)
		return -1;
	return 1;
}
