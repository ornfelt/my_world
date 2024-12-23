#include <jkssl/bignum.h>
#include <jkssl/dsa.h>

#include <string.h>
#include <stdio.h>

static int genpq(struct dsa_ctx *ctx, size_t n, size_t l,
                 struct bignum_ctx *bn_ctx, struct bignum_gencb *gencb)
{
	int ret = 0;
	size_t i;
	struct bignum x;
	struct bignum c;
	struct bignum q2;
	struct bignum one;

	bignum_init(&x);
	bignum_init(&c);
	bignum_init(&q2);
	bignum_init(&one);
	ctx->p = bignum_new();
	ctx->q = bignum_new();
	if (!ctx->p
	 || !ctx->q
	 || !bignum_one(&one)
	 || !bignum_generate_prime(ctx->q, n, 0, NULL, NULL, gencb, bn_ctx)
	 || !bignum_set_word(&x, 2)
	 || !bignum_mul(&q2, ctx->q, &x, bn_ctx))
		goto end;
	for (i = 0; i < 4096; ++i)
	{
		if (!bignum_rand(&x, l, BIGNUM_RAND_TOP_ONE, BIGNUM_RAND_BOT_ODD)
		 || !bignum_mod(&c, &x, &q2, bn_ctx)
		 || !bignum_sub(&c, &c, &one, bn_ctx)
		 || !bignum_sub(ctx->p, &x, &c, bn_ctx))
			goto end;
		if (!bignum_is_bit_set(ctx->p, l - 1))
			continue;
		int res = bignum_check_prime(ctx->p, bn_ctx, gencb);
		if (res == -1)
			goto end;
		if (res == 1)
			break;
	}
	if (gencb && !bignum_gencb_call(gencb, 2, 1))
		goto end;
	ret = (i == 4096) ? -1 : 1;

end:
	bignum_clear(&x);
	bignum_clear(&c);
	bignum_clear(&q2);
	bignum_clear(&one);
	return ret;
}

static int genh(struct dsa_ctx *ctx, struct bignum_ctx *bn_ctx)
{
	struct bignum one;
	struct bignum tmp;
	struct bignum h;
	int ret = 0;

	bignum_init(&h);
	bignum_init(&one);
	bignum_init(&tmp);
	ctx->g = bignum_new();
	if (!ctx->g
	 || !bignum_one(&one)
	 || !bignum_sub(&tmp, ctx->p, &one, bn_ctx)
	 || !bignum_div(&tmp, &tmp, ctx->q, bn_ctx))
		goto err;
	while (1)
	{
		if (!bignum_rand_range(&h, ctx->p, BIGNUM_RAND_TOP_TWO, BIGNUM_RAND_BOT_ODD)
		 || !bignum_mod_exp(ctx->g, &h, &tmp, ctx->p, bn_ctx))
			goto err;
		if (bignum_cmp(ctx->g, &one) > 0)
			break;
	}
	ret = 1;

err:
	bignum_clear(&h);
	bignum_clear(&one);
	bignum_clear(&tmp);
	return ret;
}

/* handbook of applied cryptography, 4.56 */
int dsa_generate_parameters(struct dsa_ctx *ctx, size_t bits,
                            struct bignum_gencb *gencb)
{
	struct bignum_ctx *bn_ctx;
	size_t n;

	if (bits % 64)
		return 0;
	if (bits < 512)
		return 0;
	if (bits < 2048)
		n = 160;
	else
		n = 256;
	memset(ctx, 0, sizeof(*ctx));
	bn_ctx = bignum_ctx_new();
	if (!bn_ctx)
		goto err;
	while (1)
	{
		switch (genpq(ctx, n, bits, bn_ctx, gencb))
		{
			default:
				goto err;
			case 1:
				break;
			case -1:
				continue;
		}
		break;
	}
	if (!genh(ctx, bn_ctx))
		goto err;
	return 1;

err:
	bignum_free(ctx->p);
	bignum_free(ctx->q);
	bignum_free(ctx->g);
	bignum_free(ctx->y);
	bignum_free(ctx->x);
	bignum_ctx_free(bn_ctx);
	memset(ctx, 0, sizeof(*ctx));
	return 0;
}
