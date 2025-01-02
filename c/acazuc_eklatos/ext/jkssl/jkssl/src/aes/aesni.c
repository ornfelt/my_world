#include "utils/utils.h"
#include "aes/aes.h"

#include <wmmintrin.h>
#include <string.h>

static size_t get_rounds_count(struct aes_ctx *ctx)
{
	if (ctx->key_len == 16)
		return 10;
	if (ctx->key_len == 24)
		return 12;
	return 14;
}

void aesni_encrypt(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in)
{
	__m128i b = _mm_loadu_si128((__m128i*)in);
	b = _mm_xor_si128(b, *(__m128i*)ctx->keys[0x0]);
	b = _mm_aesenc_si128(b, ((__m128i*)ctx->keys)[0x1]);
	b = _mm_aesenc_si128(b, ((__m128i*)ctx->keys)[0x2]);
	b = _mm_aesenc_si128(b, ((__m128i*)ctx->keys)[0x3]);
	b = _mm_aesenc_si128(b, ((__m128i*)ctx->keys)[0x4]);
	b = _mm_aesenc_si128(b, ((__m128i*)ctx->keys)[0x5]);
	b = _mm_aesenc_si128(b, ((__m128i*)ctx->keys)[0x6]);
	b = _mm_aesenc_si128(b, ((__m128i*)ctx->keys)[0x7]);
	b = _mm_aesenc_si128(b, ((__m128i*)ctx->keys)[0x8]);
	b = _mm_aesenc_si128(b, ((__m128i*)ctx->keys)[0x9]);
	if (ctx->key_len > 24)
	{
		b = _mm_aesenc_si128(b, ((__m128i*)ctx->keys)[0xA]);
		b = _mm_aesenc_si128(b, ((__m128i*)ctx->keys)[0xB]);
		b = _mm_aesenc_si128(b, ((__m128i*)ctx->keys)[0xC]);
		b = _mm_aesenc_si128(b, ((__m128i*)ctx->keys)[0xD]);
		b = _mm_aesenclast_si128(b, ((__m128i*)ctx->keys)[0xE]);
	}
	else if (ctx->key_len == 24)
	{
		b = _mm_aesenc_si128(b, ((__m128i*)ctx->keys)[0xA]);
		b = _mm_aesenc_si128(b, ((__m128i*)ctx->keys)[0xB]);
		b = _mm_aesenclast_si128(b, ((__m128i*)ctx->keys)[0xC]);
	}
	else
	{
		b = _mm_aesenclast_si128(b, ((__m128i*)ctx->keys)[0xA]);
	}
	_mm_storeu_si128((__m128i*)out, b);
}

void aesni_decrypt(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in)
{
	__m128i b = _mm_loadu_si128((__m128i*)in);
	b = _mm_xor_si128(b, *(__m128i*)ctx->keys[0xF]);
	b = _mm_aesdec_si128(b, ((__m128i*)ctx->keys)[0x10]);
	b = _mm_aesdec_si128(b, ((__m128i*)ctx->keys)[0x11]);
	b = _mm_aesdec_si128(b, ((__m128i*)ctx->keys)[0x12]);
	b = _mm_aesdec_si128(b, ((__m128i*)ctx->keys)[0x13]);
	b = _mm_aesdec_si128(b, ((__m128i*)ctx->keys)[0x14]);
	b = _mm_aesdec_si128(b, ((__m128i*)ctx->keys)[0x15]);
	b = _mm_aesdec_si128(b, ((__m128i*)ctx->keys)[0x16]);
	b = _mm_aesdec_si128(b, ((__m128i*)ctx->keys)[0x17]);
	b = _mm_aesdec_si128(b, ((__m128i*)ctx->keys)[0x18]);
	if (ctx->key_len > 24)
	{
		b = _mm_aesdec_si128(b, ((__m128i*)ctx->keys)[0x19]);
		b = _mm_aesdec_si128(b, ((__m128i*)ctx->keys)[0x1A]);
		b = _mm_aesdec_si128(b, ((__m128i*)ctx->keys)[0x1B]);
		b = _mm_aesdec_si128(b, ((__m128i*)ctx->keys)[0x1C]);
		b = _mm_aesdeclast_si128(b, ((__m128i*)ctx->keys)[0x1D]);
	}
	else if (ctx->key_len == 24)
	{
		b = _mm_aesdec_si128(b, ((__m128i*)ctx->keys)[0x19]);
		b = _mm_aesdec_si128(b, ((__m128i*)ctx->keys)[0x1A]);
		b = _mm_aesdeclast_si128(b, ((__m128i*)ctx->keys)[0x1B]);
	}
	else
	{
		b = _mm_aesdeclast_si128(b, ((__m128i*)ctx->keys)[0x19]);
	}
	_mm_storeu_si128((__m128i*)out, b);
}

void aesni_keyschedule(struct aes_ctx *ctx, const uint8_t *key, uint8_t len)
{
	aes_keyschedule(ctx, key, len);
	size_t rounds = get_rounds_count(ctx);
	memcpy(ctx->keys[15], ctx->keys[rounds], sizeof(*ctx->keys));
	for (size_t j = 1; j < rounds; ++j)
		_mm_storeu_si128((__m128i*)ctx->keys[15 + j],
		                 _mm_aesimc_si128(*(__m128i*)ctx->keys[rounds - j]));
	memcpy(ctx->keys[15 + rounds], ctx->keys[0], sizeof(*ctx->keys));
}
