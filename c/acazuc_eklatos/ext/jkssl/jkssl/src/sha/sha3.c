#include "utils/utils.h"
#include "sha/sha.h"

#include <string.h>

static void theta(uint64_t *buf)
{
	uint64_t c[5];
	uint64_t d[5];
	for (size_t x = 0; x < 5; ++x)
		c[x] = buf[x + 0]
		     ^ buf[x + 5]
		     ^ buf[x + 10]
		     ^ buf[x + 15]
		     ^ buf[x + 20];
	for (size_t x = 0; x < 5; ++x)
		d[x] = c[(x + 4) % 5] ^ rol64(c[(x + 1) % 5], 1);
	for (size_t x = 0; x < 5; ++x)
		for (int y = 0; y < 5; ++y)
			buf[y * 5 + x] ^= d[x];
}

static void rho_pi(uint64_t *dst, const uint64_t *src)
{
	static const uint8_t rho_shift[25] =
	{
		 0, 36,  3, 41, 18,
		 1, 44, 10, 45,  2,
		62,  6, 43, 15, 61,
		28, 55, 25, 21, 56,
		27, 20, 39,  8, 14,
	};
	for (size_t y = 0; y < 5; ++y)
	{
		for (size_t x = 0; x < 5; ++x)
			dst[((2 * x + 3 * y) % 5) * 5 + y] = rol64(src[y * 5 + x], rho_shift[x * 5 + y]);
	}
}

static void chi(uint64_t *dst, const uint64_t *src)
{
	for (size_t y = 0; y < 5; ++y)
	{
		for (size_t x = 0; x < 5; ++x)
			dst[y * 5 + x] = src[y * 5 + x] ^ ((~src[y * 5 + (x + 1) % 5]) & src[y * 5 + (x + 2) % 5]);
	}
}

static void iota(uint64_t *buf, uint64_t t)
{
	static const uint64_t rc[24] =
	{
		0x0000000000000001,
		0x0000000000008082,
		0x800000000000808A,
		0x8000000080008000,
		0x000000000000808B,
		0x0000000080000001,
		0x8000000080008081,
		0x8000000000008009,
		0x000000000000008A,
		0x0000000000000088,
		0x0000000080008009,
		0x000000008000000A,
		0x000000008000808B,
		0x800000000000008B,
		0x8000000000008089,
		0x8000000000008003,
		0x8000000000008002,
		0x8000000000000080,
		0x000000000000800A,
		0x800000008000000A,
		0x8000000080008081,
		0x8000000000008080,
		0x0000000080000001,
		0x8000000080008008,
	};
	buf[0] ^= rc[t];
}

static void chunk(struct sha3_ctx *ctx, const uint8_t *data)
{
	uint64_t tmp1[25];
	uint64_t tmp2[25];
	for (size_t i = 0; i < ctx->block_size / 8; ++i)
		tmp1[i] = ctx->h[i] ^ le64dec(&data[i * 8]);
	for (size_t i = ctx->block_size / 8; i < 25; ++i)
		tmp1[i] = ctx->h[i];
	for (size_t i = 0; i < 24; ++i)
	{
		theta(tmp1);
		rho_pi(tmp2, tmp1);
		chi(tmp1, tmp2);
		iota(tmp1, i);
	}
	for (size_t i = 0; i < 25; ++i)
		ctx->h[i] = tmp1[i];
}

static int sha3_init(struct sha3_ctx *ctx, size_t block_size)
{
	memset(ctx, 0, sizeof(*ctx));
	ctx->block_size = block_size;
	return 1;
}

static int sha3_update(struct sha3_ctx *ctx, const void *data, size_t size)
{
	HASH_BUFFERIZE(ctx, data, size, ctx->block_size);
	return 1;
}

static void sha3_finalize(struct sha3_ctx *ctx, uint8_t xorpad)
{
	memset(&ctx->data[ctx->data_size], 0, ctx->block_size - ctx->data_size);
	uint64_t *ptr = (uint64_t*)&ctx->data[ctx->data_size & ~7];
	uint8_t data[8];
	le64enc(data, *ptr);
	data[ctx->data_size & 7] ^= xorpad;
	*ptr = le64dec(data);
	if (!ctx->data_size)
		ctx->data_size += 8;
	else
		ctx->data_size += 7 - (ctx->data_size & ~7);
	ptr = (uint64_t*)&ctx->data[ctx->block_size - 8];
	le64enc(data, *ptr);
	data[7] ^= 0x80;
	*ptr = le64dec(data);
	chunk(ctx, ctx->data);
}

int sha3_224_init(struct sha3_ctx *ctx)
{
	return sha3_init(ctx, 144);
}

int sha3_224_update(struct sha3_ctx *ctx, const void *data, size_t size)
{
	return sha3_update(ctx, data, size);
}

int sha3_224_final(uint8_t *md, struct sha3_ctx *ctx)
{
	sha3_finalize(ctx, 0x06);
	for (size_t i = 0; i < 3; ++i)
		le64enc(&md[i * 8], ctx->h[i]);
	le32enc(&md[3 * 8], ctx->h[3]);
	return 1;
}

int sha3_224(const uint8_t *data, size_t size, uint8_t *md)
{
	struct sha3_ctx ctx;
	if (!sha3_224_init(&ctx)
	 || !sha3_224_update(&ctx, data, size)
	 || !sha3_224_final(md, &ctx))
		return 0;
	return 1;
}

int sha3_256_init(struct sha3_ctx *ctx)
{
	return sha3_init(ctx, 136);
}

int sha3_256_update(struct sha3_ctx *ctx, const void *data, size_t size)
{
	return sha3_update(ctx, data, size);
}

int sha3_256_final(uint8_t *md, struct sha3_ctx *ctx)
{
	sha3_finalize(ctx, 0x06);
	for (size_t i = 0; i < 4; ++i)
		le64enc(&md[i * 8], ctx->h[i]);
	return 1;
}

int sha3_256(const uint8_t *data, size_t size, uint8_t *md)
{
	struct sha3_ctx ctx;
	if (!sha3_256_init(&ctx)
	 || !sha3_256_update(&ctx, data, size)
	 || !sha3_256_final(md, &ctx))
		return 0;
	return 1;
}

int sha3_384_init(struct sha3_ctx *ctx)
{
	return sha3_init(ctx, 104);
}

int sha3_384_update(struct sha3_ctx *ctx, const void *data, size_t size)
{
	return sha3_update(ctx, data, size);
}

int sha3_384_final(uint8_t *md, struct sha3_ctx *ctx)
{
	sha3_finalize(ctx, 0x06);
	for (size_t i = 0; i < 6; ++i)
		le64enc(&md[i * 8], ctx->h[i]);
	return 1;
}

int sha3_384(const uint8_t *data, size_t size, uint8_t *md)
{
	struct sha3_ctx ctx;
	if (!sha3_384_init(&ctx)
	 || !sha3_384_update(&ctx, data, size)
	 || !sha3_384_final(md, &ctx))
		return 0;
	return 1;
}

int sha3_512_init(struct sha3_ctx *ctx)
{
	return sha3_init(ctx, 72);
}

int sha3_512_update(struct sha3_ctx *ctx, const void *data, size_t size)
{
	return sha3_update(ctx, data, size);
}

int sha3_512_final(uint8_t *md, struct sha3_ctx *ctx)
{
	sha3_finalize(ctx, 0x06);
	for (size_t i = 0; i < 8; ++i)
		le64enc(&md[i * 8], ctx->h[i]);
	return 1;
}

int sha3_512(const uint8_t *data, size_t size, uint8_t *md)
{
	struct sha3_ctx ctx;
	if (!sha3_512_init(&ctx)
	 || !sha3_512_update(&ctx, data, size)
	 || !sha3_512_final(md, &ctx))
		return 0;
	return 1;
}

int shake128_init(struct sha3_ctx *ctx)
{
	return sha3_init(ctx, 168);
}

int shake128_update(struct sha3_ctx *ctx, const void *data, size_t size)
{
	return sha3_update(ctx, data, size);
}

int shake128_final(uint8_t *md, struct sha3_ctx *ctx)
{
	sha3_finalize(ctx, 0x1F);
	for (size_t i = 0; i < 2; ++i)
		le64enc(&md[i * 8], ctx->h[i]);
	return 1;
}

int shake128(const uint8_t *data, size_t size, uint8_t *md)
{
	struct sha3_ctx ctx;
	if (!shake128_init(&ctx)
	 || !shake128_update(&ctx, data, size)
	 || !shake128_final(md, &ctx))
		return 0;
	return 1;
}

int shake256_init(struct sha3_ctx *ctx)
{
	return sha3_init(ctx, 136);
}

int shake256_update(struct sha3_ctx *ctx, const void *data, size_t size)
{
	return sha3_update(ctx, data, size);
}

int shake256_final(uint8_t *md, struct sha3_ctx *ctx)
{
	sha3_finalize(ctx, 0x1F);
	for (size_t i = 0; i < 4; ++i)
		le64enc(&md[i * 8], ctx->h[i]);
	return 1;
}

int shake256(const uint8_t *data, size_t size, uint8_t *md)
{
	struct sha3_ctx ctx;
	if (!shake256_init(&ctx)
	 || !shake256_update(&ctx, data, size)
	 || !shake256_final(md, &ctx))
		return 0;
	return 1;
}
