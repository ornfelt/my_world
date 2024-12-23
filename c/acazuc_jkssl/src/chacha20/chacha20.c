#include "chacha20/chacha20.h"
#include "utils/utils.h"

#include <string.h>

void chacha20_init_state(struct chacha20_ctx *ctx, const uint8_t *key,
                         const uint8_t *iv)
{
	ctx->state[0x0] = 0x61707865;
	ctx->state[0x1] = 0x3320646E;
	ctx->state[0x2] = 0x79622D32;
	ctx->state[0x3] = 0x6B206574;
	ctx->state[0x4] = le32dec(&key[0x00]);
	ctx->state[0x5] = le32dec(&key[0x04]);
	ctx->state[0x6] = le32dec(&key[0x08]);
	ctx->state[0x7] = le32dec(&key[0x0C]);
	ctx->state[0x8] = le32dec(&key[0x10]);
	ctx->state[0x9] = le32dec(&key[0x14]);
	ctx->state[0xA] = le32dec(&key[0x18]);
	ctx->state[0xB] = le32dec(&key[0x1C]);
	ctx->state[0xC] = le32dec(&iv[0x0]);
	ctx->state[0xD] = le32dec(&iv[0x4]);
	ctx->state[0xE] = le32dec(&iv[0x8]);
	ctx->state[0xF] = le32dec(&iv[0xC]);
}

static void do_round(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d)
{
	*a += *b;
	*d = rol32(*d ^ *a, 16);
	*c += *d;
	*b = rol32(*b ^ *c, 12);
	*a += *b;
	*d = rol32(*d ^ *a, 8);
	*c += *d;
	*b = rol32(*b ^ *c, 7);
}

static void rounds(uint32_t *x)
{
	do_round(x + 0x0, x + 0x4, x + 0x8, x + 0xC);
	do_round(x + 0x1, x + 0x5, x + 0x9, x + 0xD);
	do_round(x + 0x2, x + 0x6, x + 0xA, x + 0xE);
	do_round(x + 0x3, x + 0x7, x + 0xB, x + 0xF);
	do_round(x + 0x0, x + 0x5, x + 0xA, x + 0xF);
	do_round(x + 0x1, x + 0x6, x + 0xB, x + 0xC);
	do_round(x + 0x2, x + 0x7, x + 0x8, x + 0xD);
	do_round(x + 0x3, x + 0x4, x + 0x9, x + 0xE);
}

void chacha20_operate_block(struct chacha20_ctx *ctx, uint8_t *out,
                            const uint8_t *in)
{
	uint32_t x[16];

	for (int i = 0; i < 16; ++i)
		x[i] = ctx->state[i];
	for (size_t i = 0; i < 10; ++i)
		rounds(x);
	for (size_t i = 0; i < 16; ++i)
		le32enc(&out[i * 4], le32dec(&in[i * 4]) ^ (x[i] + ctx->state[i]));
	ctx->state[12]++;
	if (!ctx->state[12])
		ctx->state[13]++;
}
