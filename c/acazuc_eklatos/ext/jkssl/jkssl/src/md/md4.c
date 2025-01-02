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

#define ROUND(delta, i, k, tmp, fv) \
do \
{ \
	uint32_t f = (fv) + (tmp)[0] + (k) + le32dec(&(data)[g[(i)] * 4]); \
	(tmp)[0] = (tmp)[3]; \
	(tmp)[3] = (tmp)[2]; \
	(tmp)[2] = (tmp)[1]; \
	(tmp)[1] = rol32(f, s[(i)]); \
} while (0)

#define ROUND1(data, i, tmp) ROUND(data, i, 0x00000000, tmp, ((tmp)[1] & (tmp)[2]) | ((~(tmp)[1]) & (tmp)[3]))
#define ROUND2(data, i, tmp) ROUND(data, i, 0x5A827999, tmp, ((tmp)[1] & (tmp)[2]) | ((tmp)[1] & (tmp)[3]) | ((tmp)[2] & (tmp)[3]))
#define ROUND3(data, i, tmp) ROUND(data, i, 0x6ED9EBA1, tmp, (tmp)[1] ^ (tmp)[2] ^ (tmp)[3])

static void chunk(struct md4_ctx *ctx, const uint8_t *data)
{
	uint32_t tmp[4];

	for (int i = 0; i < 4; ++i)
		tmp[i] = ctx->h[i];

	ROUND1(data, 0x00, tmp);
	ROUND1(data, 0x01, tmp);
	ROUND1(data, 0x02, tmp);
	ROUND1(data, 0x03, tmp);
	ROUND1(data, 0x04, tmp);
	ROUND1(data, 0x05, tmp);
	ROUND1(data, 0x06, tmp);
	ROUND1(data, 0x07, tmp);
	ROUND1(data, 0x08, tmp);
	ROUND1(data, 0x09, tmp);
	ROUND1(data, 0x0A, tmp);
	ROUND1(data, 0x0B, tmp);
	ROUND1(data, 0x0C, tmp);
	ROUND1(data, 0x0D, tmp);
	ROUND1(data, 0x0E, tmp);
	ROUND1(data, 0x0F, tmp);
	ROUND2(data, 0x10, tmp);
	ROUND2(data, 0x11, tmp);
	ROUND2(data, 0x12, tmp);
	ROUND2(data, 0x13, tmp);
	ROUND2(data, 0x14, tmp);
	ROUND2(data, 0x15, tmp);
	ROUND2(data, 0x16, tmp);
	ROUND2(data, 0x17, tmp);
	ROUND2(data, 0x18, tmp);
	ROUND2(data, 0x19, tmp);
	ROUND2(data, 0x1A, tmp);
	ROUND2(data, 0x1B, tmp);
	ROUND2(data, 0x1C, tmp);
	ROUND2(data, 0x1D, tmp);
	ROUND2(data, 0x1E, tmp);
	ROUND2(data, 0x1F, tmp);
	ROUND3(data, 0x20, tmp);
	ROUND3(data, 0x21, tmp);
	ROUND3(data, 0x22, tmp);
	ROUND3(data, 0x23, tmp);
	ROUND3(data, 0x24, tmp);
	ROUND3(data, 0x25, tmp);
	ROUND3(data, 0x26, tmp);
	ROUND3(data, 0x27, tmp);
	ROUND3(data, 0x28, tmp);
	ROUND3(data, 0x29, tmp);
	ROUND3(data, 0x2A, tmp);
	ROUND3(data, 0x2B, tmp);
	ROUND3(data, 0x2C, tmp);
	ROUND3(data, 0x2D, tmp);
	ROUND3(data, 0x2E, tmp);
	ROUND3(data, 0x2F, tmp);

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
