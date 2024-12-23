#include "utils/utils.h"
#include "sha/sha.h"

#include <string.h>

static const uint32_t k[4] =
{
	0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6,
};

static void loop(int i, uint32_t *tmp, uint32_t *w)
{
	uint32_t f;

	if (i <= 19)
		f = (tmp[1] & tmp[2]) | ((~tmp[1]) & tmp[3]);
	else if (i <= 39)
		f = tmp[1] ^ tmp[2] ^ tmp[3];
	else if (i <= 59)
		f = (tmp[1] & tmp[2]) | (tmp[1] & tmp[3]) | (tmp[2] & tmp[3]);
	else
		f = tmp[1] ^ tmp[2] ^ tmp[3];
	f += rol32(tmp[0], 5) + tmp[4] + k[i / 20] + w[i];
	tmp[4] = tmp[3];
	tmp[3] = tmp[2];
	tmp[2] = rol32(tmp[1], 30);
	tmp[1] = tmp[0];
	tmp[0] = f;
}

static void chunk(struct sha0_ctx *ctx, const uint8_t *data)
{
	uint32_t tmp[5];
	uint32_t w[80];
	int i;

	for (i = 0; i < 16; ++i)
		w[i] = be32dec(&data[i * 4]);
	for (; i < 80; ++i)
		w[i] = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16];
	for (i = 0; i < 5; ++i)
		tmp[i] = ctx->h[i];
	for (i = 0; i < 80; ++i)
		loop(i, tmp, w);
	for (i = 0; i < 5; ++i)
		ctx->h[i] += tmp[i];
}

int sha0_init(struct sha0_ctx *ctx)
{
	ctx->total_size = 0;
	ctx->data_size = 0;
	ctx->h[0] = 0x67452301;
	ctx->h[1] = 0xEFCDAB89;
	ctx->h[2] = 0x98BADCFE;
	ctx->h[3] = 0x10325476;
	ctx->h[4] = 0xC3D2E1F0;
	return 1;
}

int sha0_update(struct sha0_ctx *ctx, const void *data, size_t size)
{
	HASH_BUFFERIZE(ctx, data, size, 64);
	return 1;
}

int sha0_final(uint8_t *md, struct sha0_ctx *ctx)
{
	MERKLE_DAMGARD_FINALIZE(ctx, 64, 1);
	for (int i = 0; i < 5; ++i)
		be32enc(&md[i * 4], ctx->h[i]);
	return 1;
}

int sha0(const uint8_t *data, size_t size, uint8_t *md)
{
	struct sha0_ctx ctx;
	if (!sha0_init(&ctx)
	 || !sha0_update(&ctx, data, size)
	 || !sha0_final(md, &ctx))
		return 0;
	return 1;
}
