#include "dsa/dsa.h"

#include <jkssl/bignum.h>
#include <jkssl/dsa.h>

#include <string.h>
#include <stdio.h>

static int genpq(struct dsa *dsa, size_t n, size_t l,
                 struct bignum_ctx *bn_ctx, struct bignum_gencb *gencb)
{
	int ret = 0;
	size_t i;
	struct bignum *x;
	struct bignum *c;
	struct bignum *q2;
	struct bignum *one;

	x = bignum_ctx_get(bn_ctx);
	c = bignum_ctx_get(bn_ctx);
	q2 = bignum_ctx_get(bn_ctx);
	one = bignum_ctx_get(bn_ctx);
	bignum_free(dsa->p);
	bignum_free(dsa->q);
	dsa->p = bignum_new();
	dsa->q = bignum_new();
	if (!x
	 || !c
	 || !q2
	 || !one
	 || !dsa->p
	 || !dsa->q
	 || !bignum_one(one)
	 || !bignum_generate_prime(dsa->q, n, 0, NULL, NULL, gencb, bn_ctx)
	 || !bignum_set_word(x, 2)
	 || !bignum_mul(q2, dsa->q, x, bn_ctx))
		goto end;
	for (i = 0; i < 4096; ++i)
	{
		if (!bignum_rand(x, l, BIGNUM_RAND_TOP_ONE, BIGNUM_RAND_BOT_ODD)
		 || !bignum_mod(c, x, q2, bn_ctx)
		 || !bignum_sub(c, c, one, bn_ctx)
		 || !bignum_sub(dsa->p, x, c, bn_ctx))
			goto end;
		if (!bignum_is_bit_set(dsa->p, l - 1))
			continue;
		int res = bignum_check_prime(dsa->p, bn_ctx, gencb);
		if (res == -1)
			goto end;
		if (res == 1)
			break;
	}
	if (gencb && !bignum_gencb_call(gencb, 2, 1))
		goto end;
	ret = (i == 4096) ? -1 : 1;

end:
	bignum_ctx_release(bn_ctx, x);
	bignum_ctx_release(bn_ctx, c);
	bignum_ctx_release(bn_ctx, q2);
	bignum_ctx_release(bn_ctx, one);
	return ret;
}

static int genh(struct dsa *dsa, struct bignum_ctx *bn_ctx)
{
	struct bignum *one;
	struct bignum *tmp;
	struct bignum *h;
	int ret = 0;

	h = bignum_ctx_get(bn_ctx);
	one = bignum_ctx_get(bn_ctx);
	tmp = bignum_ctx_get(bn_ctx);
	bignum_free(dsa->g);
	dsa->g = bignum_new();
	if (!h
	 || !one
	 || !tmp
	 || !dsa->g
	 || !bignum_one(one)
	 || !bignum_sub(tmp, dsa->p, one, bn_ctx)
	 || !bignum_div(tmp, tmp, dsa->q, bn_ctx))
		goto err;
	while (1)
	{
		if (!bignum_rand_range(h, dsa->p, BIGNUM_RAND_TOP_TWO, BIGNUM_RAND_BOT_ODD)
		 || !bignum_mod_exp(dsa->g, h, tmp, dsa->p, bn_ctx))
			goto err;
		if (bignum_cmp(dsa->g, one) > 0)
			break;
	}
	ret = 1;

err:
	bignum_ctx_release(bn_ctx, h);
	bignum_ctx_release(bn_ctx, one);
	bignum_ctx_release(bn_ctx, tmp);
	return ret;
}

/* handbook of applied cryptography, 4.56 */
int dsa_generate_parameters(struct dsa *dsa, size_t bits,
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
	memset(dsa, 0, sizeof(*dsa));
	bn_ctx = bignum_ctx_new();
	if (!bn_ctx)
		goto err;
	while (1)
	{
		switch (genpq(dsa, n, bits, bn_ctx, gencb))
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
	if (!genh(dsa, bn_ctx))
		goto err;
	return 1;

err:
	bignum_free(dsa->p);
	bignum_free(dsa->q);
	bignum_free(dsa->g);
	bignum_free(dsa->y);
	bignum_free(dsa->x);
	bignum_ctx_free(bn_ctx);
	memset(dsa, 0, sizeof(*dsa));
	return 0;
}
