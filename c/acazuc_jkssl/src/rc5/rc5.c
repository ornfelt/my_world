#include "utils/utils.h"
#include "rc5/rc5.h"

void rc5_encrypt(struct rc5_ctx *ctx, uint8_t *out, const uint8_t *in)
{
	uint32_t a = be32dec(&in[0]) + ctx->s[0];
	uint32_t b = be32dec(&in[4]) + ctx->s[1];
	for (size_t i = 1; i <= 12; ++i)
	{
		a = rol32(a ^ b, b) + ctx->s[2 * i + 0];
		b = rol32(b ^ a, a) + ctx->s[2 * i + 1];
	}
	le32enc(&out[0], a);
	le32enc(&out[4], b);
}

void rc5_decrypt(struct rc5_ctx *ctx, uint8_t *out, const uint8_t *in)
{
	uint32_t a = le32dec(&in[0]);
	uint32_t b = le32dec(&in[4]);
	for (size_t i = 12; i > 0; --i)
	{
		b = ror32(b - ctx->s[2 * i + 1], a) ^ a;
		a = ror32(a - ctx->s[2 * i + 0], b) ^ b;
	}
	le32enc(&out[0], a - ctx->s[0]);
	le32enc(&out[4], b - ctx->s[1]);
}

void rc5_keyschedule(struct rc5_ctx *ctx, const uint8_t *key)
{
	uint32_t l[4];
	uint32_t a;
	uint32_t b;
	int32_t t = 26;
	int32_t i;
	int32_t j;

	l[3] = 0;
	for (i = 15; i >= 0; --i)
		l[i / 4] = (l[i / 4] << 8) + key[i];
	ctx->s[0] = 0xB7E15163;
	for (i = 1; i < t; ++i)
		ctx->s[i] = ctx->s[i - 1] + 0x9E3779B9;
	a = 0;
	b = 0;
	i = 0;
	j = 0;
	for (int32_t k = 0; k < 3 * t; ++k)
	{
		a = rol32(ctx->s[i] + a + b, 3);
		b = rol32(l[j] + a + b, a + b);
		ctx->s[i] = a;
		l[j] = b;
		i = (i + 1) % t;
		j = (j + 1) % 4;
	}
}
