#include "utils/utils.h"
#include "sha/sha.h"

#include <string.h>

#define ROUND(i, tmp, w, h, fv) \
do \
{ \
	uint32_t f = (fv) + rol32((tmp)[0], 5) + (tmp)[4] + (h) + (w)[i]; \
	(tmp)[4] = (tmp)[3]; \
	(tmp)[3] = (tmp)[2]; \
	(tmp)[2] = rol32((tmp)[1], 30); \
	(tmp)[1] = (tmp)[0]; \
	(tmp)[0] = f; \
} while (0)

#define ROUND1(i, tmp, w) ROUND(i, tmp, w, 0x5A827999, (tmp)[3] ^ ((tmp[1] & ((tmp)[2] ^ (tmp)[3]))))
#define ROUND2(i, tmp, w) ROUND(i, tmp, w, 0x6ED9EBA1, (tmp)[1] ^ (tmp)[2] ^ (tmp)[3])
#define ROUND3(i, tmp, w) ROUND(i, tmp, w, 0x8F1BBCDC, ((tmp)[1] & (tmp)[2]) | (((tmp)[1] | (tmp)[2]) & (tmp)[3]))
#define ROUND4(i, tmp, w) ROUND(i, tmp, w, 0xCA62C1D6, (tmp)[1] ^ (tmp)[2] ^ (tmp)[3])

static void chunk(struct sha1_ctx *ctx, const uint8_t *data)
{
	uint32_t tmp[5];
	uint32_t w[80];
	int i;

	for (i = 0; i < 16; ++i)
		w[i] = be32dec(&data[i * 4]);
	for (; i < 80; ++i)
		w[i] = rol32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
	for (i = 0; i < 5; ++i)
		tmp[i] = ctx->h[i];
	ROUND1( 0, tmp, w);
	ROUND1( 1, tmp, w);
	ROUND1( 2, tmp, w);
	ROUND1( 3, tmp, w);
	ROUND1( 4, tmp, w);
	ROUND1( 5, tmp, w);
	ROUND1( 6, tmp, w);
	ROUND1( 7, tmp, w);
	ROUND1( 8, tmp, w);
	ROUND1( 9, tmp, w);
	ROUND1(10, tmp, w);
	ROUND1(11, tmp, w);
	ROUND1(12, tmp, w);
	ROUND1(13, tmp, w);
	ROUND1(14, tmp, w);
	ROUND1(15, tmp, w);
	ROUND1(16, tmp, w);
	ROUND1(17, tmp, w);
	ROUND1(18, tmp, w);
	ROUND1(19, tmp, w);
	ROUND2(20, tmp, w);
	ROUND2(21, tmp, w);
	ROUND2(22, tmp, w);
	ROUND2(23, tmp, w);
	ROUND2(24, tmp, w);
	ROUND2(25, tmp, w);
	ROUND2(26, tmp, w);
	ROUND2(27, tmp, w);
	ROUND2(28, tmp, w);
	ROUND2(29, tmp, w);
	ROUND2(30, tmp, w);
	ROUND2(31, tmp, w);
	ROUND2(32, tmp, w);
	ROUND2(33, tmp, w);
	ROUND2(34, tmp, w);
	ROUND2(35, tmp, w);
	ROUND2(36, tmp, w);
	ROUND2(37, tmp, w);
	ROUND2(38, tmp, w);
	ROUND2(39, tmp, w);
	ROUND3(40, tmp, w);
	ROUND3(41, tmp, w);
	ROUND3(42, tmp, w);
	ROUND3(43, tmp, w);
	ROUND3(44, tmp, w);
	ROUND3(45, tmp, w);
	ROUND3(46, tmp, w);
	ROUND3(47, tmp, w);
	ROUND3(48, tmp, w);
	ROUND3(49, tmp, w);
	ROUND3(50, tmp, w);
	ROUND3(51, tmp, w);
	ROUND3(52, tmp, w);
	ROUND3(53, tmp, w);
	ROUND3(54, tmp, w);
	ROUND3(55, tmp, w);
	ROUND3(56, tmp, w);
	ROUND3(57, tmp, w);
	ROUND3(58, tmp, w);
	ROUND3(59, tmp, w);
	ROUND4(60, tmp, w);
	ROUND4(61, tmp, w);
	ROUND4(62, tmp, w);
	ROUND4(63, tmp, w);
	ROUND4(64, tmp, w);
	ROUND4(65, tmp, w);
	ROUND4(66, tmp, w);
	ROUND4(67, tmp, w);
	ROUND4(68, tmp, w);
	ROUND4(69, tmp, w);
	ROUND4(70, tmp, w);
	ROUND4(71, tmp, w);
	ROUND4(72, tmp, w);
	ROUND4(73, tmp, w);
	ROUND4(74, tmp, w);
	ROUND4(75, tmp, w);
	ROUND4(76, tmp, w);
	ROUND4(77, tmp, w);
	ROUND4(78, tmp, w);
	ROUND4(79, tmp, w);
	for (i = 0; i < 5; ++i)
		ctx->h[i] += tmp[i];
}

int sha1_init(struct sha1_ctx *ctx)
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

int sha1_update(struct sha1_ctx *ctx, const void *data, size_t size)
{
	HASH_BUFFERIZE(ctx, data, size, 64);
	return 1;
}

int sha1_final(uint8_t *md, struct sha1_ctx *ctx)
{
	MERKLE_DAMGARD_FINALIZE(ctx, 64, 1);
	for (int i = 0; i < 5; ++i)
		be32enc(&md[i * 4], ctx->h[i]);
	return 1;
}

int sha1(const uint8_t *data, size_t size, uint8_t *md)
{
	struct sha1_ctx ctx;
	if (!sha1_init(&ctx)
	 || !sha1_update(&ctx, data, size)
	 || !sha1_final(md, &ctx))
		return 0;
	return 1;
}
