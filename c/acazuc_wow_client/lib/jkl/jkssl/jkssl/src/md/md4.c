#include "utils/utils.h"
#include "md/md.h"

#include <string.h>

static const uint32_t s[48] =
{
	0x03, 0x07, 0x0B, 0x13, 0x03, 0x07, 0x0B, 0x13,
	0x03, 0x07, 0x0B, 0x13, 0x03, 0x07, 0x0B, 0x13,
	0x03, 0x05, 0x09, 0x0D, 0x03, 0x05, 0x09, 0x0D,
	0x03, 0x05, 0x09, 0x0D, 0x03, 0x05, 0x09, 0x0D,
	0x03, 0x09, 0x0B, 0x0F, 0x03, 0x09, 0x0B, 0x0F,
	0x03, 0x09, 0x0B, 0x0F, 0x03, 0x09, 0x0B, 0x0F,
};

static const uint32_t g[48] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x00, 0x04, 0x08, 0x0C, 0x01, 0x05, 0x09, 0x0D,
	0x02, 0x06, 0x0A, 0x0E, 0x03, 0x07, 0x0B, 0x0F,
	0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A, 0x06, 0x0E,
	0x01, 0x09, 0x05, 0x0D, 0x03, 0x0B, 0x07, 0x0F,
};

static uint32_t k[48] = {0, 0x5A827999, 0x6ED9EBA1};

static void loop(const uint8_t *data, int i, uint32_t *tmp)
{
	uint32_t f;

	if (i <= 15)
		f = (tmp[1] & tmp[2]) | ((~tmp[1]) & tmp[3]);
	else if (i <= 31)
		f = (tmp[1] & tmp[2]) | (tmp[1] & tmp[3]) | (tmp[2] & tmp[3]);
	else
		f = tmp[1] ^ tmp[2] ^ tmp[3];
	f += tmp[0] + k[i / 16] + le32dec(&data[g[i] * 4]);
	tmp[0] = tmp[3];
	tmp[3] = tmp[2];
	tmp[2] = tmp[1];
	tmp[1] = rol32(f, s[i]);
}

static void chunk(struct md4_ctx *ctx, const uint8_t *data)
{
	uint32_t tmp[4];

	for (int i = 0; i < 4; ++i)
		tmp[i] = ctx->h[i];
	for (int i = 0; i < 48; ++i)
		loop(data, i, tmp);
	for (int i = 0; i < 4; ++i)
		ctx->h[i] += tmp[i];
}

int md4_init(struct md4_ctx *ctx)
{
	ctx->total_size = 0;
	ctx->data_size = 0;
	ctx->h[0] = 0x67452301;
	ctx->h[1] = 0xEFCDAB89;
	ctx->h[2] = 0x98BADCFE;
	ctx->h[3] = 0x10325476;
	return 1;
}

int md4_update(struct md4_ctx *ctx, const void *data, size_t size)
{
	HASH_BUFFERIZE(ctx, data, size, 64);
	return 1;
}

int md4_final(uint8_t *md, struct md4_ctx *ctx)
{
	MERKLE_DAMGARD_FINALIZE(ctx, 64, 0);
	for (int i = 0; i < 4; ++i)
		le32enc(&md[i * 4], ctx->h[i]);
	return 1;
}

int md4(const uint8_t *data, size_t size, uint8_t *md)
{
	struct md4_ctx ctx;
	if (!md4_init(&ctx)
	 || !md4_update(&ctx, data, size)
	 || !md4_final(md, &ctx))
		return 0;
	return 1;
}
