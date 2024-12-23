#include "utils/utils.h"
#include "aes/aes.h"

#include <arm_neon.h>
#include <string.h>

static size_t get_rounds_count(struct aes_ctx *ctx)
{
	if (ctx->key_len == 16)
		return 10;
	if (ctx->key_len == 24)
		return 12;
	return 14;
}

void aes_neon_encrypt(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in)
{
	uint8x16_t b = vld1q_u8(in);
	b = vaesmcq_u8(vaeseq_u8(b, vld1q_u8(ctx->keys[0x0])));
	b = vaesmcq_u8(vaeseq_u8(b, vld1q_u8(ctx->keys[0x1])));
	b = vaesmcq_u8(vaeseq_u8(b, vld1q_u8(ctx->keys[0x2])));
	b = vaesmcq_u8(vaeseq_u8(b, vld1q_u8(ctx->keys[0x3])));
	b = vaesmcq_u8(vaeseq_u8(b, vld1q_u8(ctx->keys[0x4])));
	b = vaesmcq_u8(vaeseq_u8(b, vld1q_u8(ctx->keys[0x5])));
	b = vaesmcq_u8(vaeseq_u8(b, vld1q_u8(ctx->keys[0x6])));
	b = vaesmcq_u8(vaeseq_u8(b, vld1q_u8(ctx->keys[0x7])));
	b = vaesmcq_u8(vaeseq_u8(b, vld1q_u8(ctx->keys[0x8])));
	if (ctx->key_len > 24)
	{
		b = vaesmcq_u8(vaeseq_u8(b, vld1q_u8(ctx->keys[0x9])));
		b = vaesmcq_u8(vaeseq_u8(b, vld1q_u8(ctx->keys[0xA])));
		b = vaesmcq_u8(vaeseq_u8(b, vld1q_u8(ctx->keys[0xB])));
		b = vaesmcq_u8(vaeseq_u8(b, vld1q_u8(ctx->keys[0xC])));
		b = vaeseq_u8(b, vld1q_u8(ctx->keys[0xD]));
		b ^= vld1q_u8(ctx->keys[0xE]);
	}
	else if (ctx->key_len == 24)
	{
		b = vaesmcq_u8(vaeseq_u8(b, vld1q_u8(ctx->keys[0x9])));
		b = vaesmcq_u8(vaeseq_u8(b, vld1q_u8(ctx->keys[0xA])));
		b = vaeseq_u8(b, vld1q_u8(ctx->keys[0xB]));
		b ^= vld1q_u8(ctx->keys[0xC]);
	}
	else
	{
		b = vaeseq_u8(b, vld1q_u8(ctx->keys[0x9]));
		b ^= vld1q_u8(ctx->keys[0xA]);
	}
	vst1q_u8(out, b);
}

void aes_neon_decrypt(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in)
{
	uint8x16_t b = vld1q_u8(in);
	b = vaesimcq_u8(vaesdq_u8(b, vld1q_u8(ctx->keys[0x0F])));
	b = vaesimcq_u8(vaesdq_u8(b, vld1q_u8(ctx->keys[0x10])));
	b = vaesimcq_u8(vaesdq_u8(b, vld1q_u8(ctx->keys[0x11])));
	b = vaesimcq_u8(vaesdq_u8(b, vld1q_u8(ctx->keys[0x12])));
	b = vaesimcq_u8(vaesdq_u8(b, vld1q_u8(ctx->keys[0x13])));
	b = vaesimcq_u8(vaesdq_u8(b, vld1q_u8(ctx->keys[0x14])));
	b = vaesimcq_u8(vaesdq_u8(b, vld1q_u8(ctx->keys[0x15])));
	b = vaesimcq_u8(vaesdq_u8(b, vld1q_u8(ctx->keys[0x16])));
	b = vaesimcq_u8(vaesdq_u8(b, vld1q_u8(ctx->keys[0x17])));
	if (ctx->key_len > 24)
	{
		b = vaesimcq_u8(vaesdq_u8(b, vld1q_u8(ctx->keys[0x18])));
		b = vaesimcq_u8(vaesdq_u8(b, vld1q_u8(ctx->keys[0x19])));
		b = vaesimcq_u8(vaesdq_u8(b, vld1q_u8(ctx->keys[0x1A])));
		b = vaesimcq_u8(vaesdq_u8(b, vld1q_u8(ctx->keys[0x1B])));
		b = vaesdq_u8(b, vld1q_u8(ctx->keys[0x1C]));
		b ^= vld1q_u8(ctx->keys[0x1D]);
	}
	else if (ctx->key_len == 24)
	{
		b = vaesimcq_u8(vaesdq_u8(b, vld1q_u8(ctx->keys[0x18])));
		b = vaesimcq_u8(vaesdq_u8(b, vld1q_u8(ctx->keys[0x19])));
		b = vaesdq_u8(b, vld1q_u8(ctx->keys[0x1A]));
		b ^= vld1q_u8(ctx->keys[0x1B]);
	}
	else
	{
		b = vaesdq_u8(b, vld1q_u8(ctx->keys[0x18]));
		b ^= vld1q_u8(ctx->keys[0x19]);
	}
	vst1q_u8(out, b);
}

void aes_neon_keyschedule(struct aes_ctx *ctx, const uint8_t *key, uint8_t len)
{
	aes_keyschedule(ctx, key, len);
	size_t rounds = get_rounds_count(ctx);
	memcpy(ctx->keys[15], ctx->keys[rounds], sizeof(*ctx->keys));
	for (size_t j = 1; j < rounds; ++j)
		vst1q_u8(ctx->keys[15 + j], vaesimcq_u8(vld1q_u8(ctx->keys[rounds - j])));
	memcpy(ctx->keys[15 + rounds], ctx->keys[0], sizeof(*ctx->keys));
}
