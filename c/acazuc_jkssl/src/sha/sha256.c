#include "utils/utils.h"
#include "sha/sha.h"

#include <string.h>

static const uint32_t k[64] =
{
	0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5,
	0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
	0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
	0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
	0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,
	0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
	0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7,
	0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
	0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
	0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
	0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3,
	0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
	0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5,
	0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
	0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
	0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2,
};

static void loop(int i, uint32_t *tmp, uint32_t *w)
{
	uint32_t tmp1 = ror32(tmp[4], 6)
	              ^ ror32(tmp[4], 11)
	              ^ ror32(tmp[4], 25);
	uint32_t tmp2 = (tmp[4] & tmp[5]) ^ ((~tmp[4]) & tmp[6]);
	uint32_t tmp3 = tmp[7] + tmp1 + tmp2 + k[i] + w[i];
	tmp1 = ror32(tmp[0], 2)
	     ^ ror32(tmp[0], 13)
	     ^ ror32(tmp[0], 22);
	tmp2 = (tmp[0] & tmp[1]) ^ (tmp[0] & tmp[2]) ^ (tmp[1] & tmp[2]);
	uint32_t tmp4 = tmp1 + tmp2;
	tmp[7] = tmp[6];
	tmp[6] = tmp[5];
	tmp[5] = tmp[4];
	tmp[4] = tmp[3] + tmp3;
	tmp[3] = tmp[2];
	tmp[2] = tmp[1];
	tmp[1] = tmp[0];
	tmp[0] = tmp3 + tmp4;
}

static void chunk(struct sha256_ctx *ctx, const uint8_t *data)
{
	uint32_t tmp[8];
	uint32_t w[64];
	int i;

	for (i = 0; i < 16; ++i)
		w[i] = be32dec(&data[i * 4]);
	for (; i < 64; ++i)
		w[i] = w[i - 16] + (ror32(w[i - 15], 7)
		                  ^ ror32(w[i - 15], 18)
		                  ^ (w[i - 15] >> 3))
		                 + w[i - 7]
		                 + (ror32(w[i - 2], 17)
		                  ^ ror32(w[i - 2], 19)
		                  ^ (w[i - 2] >> 10));
	for (i = 0; i < 8; ++i)
		tmp[i] = ctx->h[i];
	for (i = 0; i < 64; ++i)
		loop(i, tmp, w);
	for (i = 0; i < 8; ++i)
		ctx->h[i] += tmp[i];
}

int sha224_init(struct sha256_ctx *ctx)
{
	ctx->total_size = 0;
	ctx->data_size = 0;
	ctx->h[0] = 0xC1059ED8;
	ctx->h[1] = 0x367CD507;
	ctx->h[2] = 0x3070DD17;
	ctx->h[3] = 0xF70E5939;
	ctx->h[4] = 0xFFC00B31;
	ctx->h[5] = 0x68581511;
	ctx->h[6] = 0x64F98FA7;
	ctx->h[7] = 0xBEFA4FA4;
	return 1;
}

int sha224_update(struct sha256_ctx *ctx, const void *data, size_t size)
{
	HASH_BUFFERIZE(ctx, data, size, 64);
	return 1;
}

int sha224_final(uint8_t *md, struct sha256_ctx *ctx)
{
	MERKLE_DAMGARD_FINALIZE(ctx, 64, 1);
	for (int i = 0; i < 7; ++i)
		be32enc(&md[i * 4], ctx->h[i]);
	return 1;
}

int sha224(const uint8_t *data, size_t size, uint8_t *md)
{
	struct sha256_ctx ctx;
	if (!sha224_init(&ctx)
	 || !sha224_update(&ctx, data, size)
	 || !sha224_final(md, &ctx))
		return 0;
	return 1;
}

int sha256_init(struct sha256_ctx *ctx)
{
	ctx->total_size = 0;
	ctx->data_size = 0;
	ctx->h[0] = 0x6A09E667;
	ctx->h[1] = 0xBB67AE85;
	ctx->h[2] = 0x3C6EF372;
	ctx->h[3] = 0xA54FF53A;
	ctx->h[4] = 0x510E527F;
	ctx->h[5] = 0x9B05688C;
	ctx->h[6] = 0x1F83D9AB;
	ctx->h[7] = 0x5BE0CD19;
	return 1;
}

int sha256_update(struct sha256_ctx *ctx, const void *data, size_t size)
{
	HASH_BUFFERIZE(ctx, data, size, 64);
	return 1;
}

int sha256_final(uint8_t *md, struct sha256_ctx *ctx)
{
	MERKLE_DAMGARD_FINALIZE(ctx, 64, 1);
	for (int i = 0; i < 8; ++i)
		be32enc(&md[i * 4], ctx->h[i]);
	return 1;
}

int sha256(const uint8_t *data, size_t size, uint8_t *md)
{
	struct sha256_ctx ctx;
	if (!sha256_init(&ctx)
	 || !sha256_update(&ctx, data, size)
	 || !sha256_final(md, &ctx))
		return 0;
	return 1;
}

int sha256_192_init(struct sha256_ctx *ctx)
{
	return sha256_init(ctx);
}

int sha256_192_update(struct sha256_ctx *ctx, const void *data, size_t size)
{
	HASH_BUFFERIZE(ctx, data, size, 64);
	return 1;
}

int sha256_192_final(uint8_t *md, struct sha256_ctx *ctx)
{
	MERKLE_DAMGARD_FINALIZE(ctx, 64, 1);
	for (int i = 0; i < 6; ++i)
		be32enc(&md[i * 4], ctx->h[i]);
	return 1;
}

int sha256_192(const uint8_t *data, size_t size, uint8_t *md)
{
	struct sha256_ctx ctx;
	if (!sha256_192_init(&ctx)
	 || !sha256_192_update(&ctx, data, size)
	 || !sha256_192_final(md, &ctx))
		return 0;
	return 1;
}
