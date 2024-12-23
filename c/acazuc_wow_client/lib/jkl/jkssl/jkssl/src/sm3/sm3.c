#include "utils/utils.h"
#include "sm3/sm3.h"

#include <string.h>

static const uint32_t t0 = 0x79CC4519;
static const uint32_t t1 = 0x7A879D8A;

static inline uint32_t p0(uint32_t v)
{
	return v ^ rol32(v, 9) ^ rol32(v, 17);
}

static inline uint32_t p1(uint32_t v)
{
	return v ^ rol32(v, 15) ^ rol32(v, 23);
}

static inline uint32_t ff0(uint32_t x, uint32_t y, uint32_t z)
{
	return x ^ y ^ z;
}

static inline uint32_t ff1(uint32_t x, uint32_t y, uint32_t z)
{
	return (x & y) | (x & z) | (y & z);
}

static inline uint32_t gg0(uint32_t x, uint32_t y, uint32_t z)
{
	return x ^ y ^ z;
}

static inline uint32_t gg1(uint32_t x, uint32_t y, uint32_t z)
{
	return (x & y) | (~x & z);
}

static inline void r0(uint32_t *h, uint32_t *w, uint32_t *wp, uint32_t i)
{
	uint32_t ss1 = rol32(rol32(h[0], 12)
	                   + h[4]
	                   + rol32(t0, i % 32), 7);
	uint32_t ss2 = ss1 ^ rol32(h[0], 12);
	uint32_t tt1 = ff0(h[0], h[1], h[2]) + h[3] + ss2 + wp[i];
	uint32_t tt2 = gg0(h[4], h[5], h[6]) + h[7] + ss1 + w[i];
	h[3] = h[2];
	h[2] = rol32(h[1], 9);
	h[1] = h[0];
	h[0] = tt1;
	h[7] = h[6];
	h[6] = rol32(h[5], 19);
	h[5] = h[4];
	h[4] = p0(tt2);
}

static inline void r1(uint32_t *h, uint32_t *w, uint32_t *wp, uint32_t i)
{
	uint32_t ss1 = rol32(rol32(h[0], 12)
	                   + h[4]
	                   + rol32(t1, i % 32), 7);
	uint32_t ss2 = ss1 ^ rol32(h[0], 12);
	uint32_t tt1 = ff1(h[0], h[1], h[2]) + h[3] + ss2 + wp[i];
	uint32_t tt2 = gg1(h[4], h[5], h[6]) + h[7] + ss1 + w[i];
	h[3] = h[2];
	h[2] = rol32(h[1], 9);
	h[1] = h[0];
	h[0] = tt1;
	h[7] = h[6];
	h[6] = rol32(h[5], 19);
	h[5] = h[4];
	h[4] = p0(tt2);
}

static void chunk(struct sm3_ctx *ctx, const uint8_t *data)
{
	uint32_t w[68];
	uint32_t wp[64];
	uint32_t h[8];
	for (uint32_t i = 0; i < 16; ++i)
		w[i] = be32dec(&data[i * 4]);
	for (uint32_t i = 16; i < 68; ++i)
		w[i] = p1(w[i - 16] ^ w[i - 9] ^ rol32(w[i - 3], 15))
		     ^ rol32(w[i - 13], 7) ^ w[i - 6];
	for (uint32_t i = 0; i < 64; ++i)
		wp[i] = w[i] ^ w[i + 4];
	for (uint32_t i = 0; i < 8; ++i)
		h[i] = ctx->h[i];
	for (uint32_t i = 0; i < 16; ++i)
		r0(h, w, wp, i);
	for (uint32_t i = 16; i < 64; ++i)
		r1(h, w, wp, i);
	for (uint32_t i = 0; i < 8; ++i)
		ctx->h[i] ^= h[i];
}

int sm3_init(struct sm3_ctx *ctx)
{
	ctx->total_size = 0;
	ctx->data_size = 0;
	ctx->h[0] = 0x7380166F;
	ctx->h[1] = 0x4914B2B9;
	ctx->h[2] = 0x172442D7;
	ctx->h[3] = 0xDA8A0600;
	ctx->h[4] = 0xA96F30BC;
	ctx->h[5] = 0x163138AA;
	ctx->h[6] = 0xE38DEE4D;
	ctx->h[7] = 0xB0FB0E4E;
	return 1;
}

int sm3_update(struct sm3_ctx *ctx, const void *data, size_t size)
{
	HASH_BUFFERIZE(ctx, data, size, 64);
	return 1;
}

int sm3_final(uint8_t *md, struct sm3_ctx *ctx)
{
	MERKLE_DAMGARD_FINALIZE(ctx, 64, 1);
	for (int i = 0; i < 8; ++i)
		be32enc(&md[i * 4], ctx->h[i]);
	return 1;
}

int sm3(const uint8_t *data, size_t len, uint8_t *md)
{
	struct sm3_ctx ctx;
	if (!sm3_init(&ctx)
	 || !sm3_update(&ctx, data, len)
	 || !sm3_final(md, &ctx))
		return 0;
	return 1;
}
