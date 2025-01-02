#include "rsa/rsa.h"

#include <jkssl/bignum.h>
#include <jkssl/rsa.h>

#include <string.h>
#include <stdio.h>

static int gencrt(struct rsa *rsa, struct bignum_ctx *bn_ctx)
{
	struct bignum *tmp;
	struct bignum *one;
	int ret = 0;

	tmp = bignum_ctx_get(bn_ctx);
	one = bignum_ctx_get(bn_ctx);
	bignum_free(rsa->dmp);
	bignum_free(rsa->dmq);
	bignum_free(rsa->coef);
	rsa->dmp = bignum_new();
	rsa->dmq = bignum_new();
	rsa->coef = bignum_new();
	if (!tmp
	 || !one
	 || !rsa->dmp
	 || !rsa->dmq
	 || !rsa->coef
	 || !bignum_one(one)
	 || !bignum_copy(tmp, rsa->p)
	 || !bignum_sub(tmp, tmp, one, bn_ctx)
	 || !bignum_mod(rsa->dmp, rsa->d, tmp, bn_ctx)
	 || !bignum_copy(tmp, rsa->q)
	 || !bignum_sub(tmp, tmp, one, bn_ctx)
	 || !bignum_mod(rsa->dmq, rsa->d, tmp, bn_ctx)
	 || !bignum_mod_inverse(rsa->coef, rsa->q, rsa->p, bn_ctx))
		goto end;
	ret = 1;

end:
	bignum_ctx_release(bn_ctx, tmp);
	bignum_ctx_release(bn_ctx, one);
	return ret;
}

static int genphi(struct rsa *rsa, struct bignum_ctx *bn_ctx)
{
	struct bignum *tmp;
	struct bignum *one;
	int ret = 0;

	tmp = bignum_ctx_get(bn_ctx);
	one = bignum_ctx_get(bn_ctx);
	if (!tmp
	 || !one
	 || !bignum_one(one)
	 || !bignum_sub(tmp, rsa->p, one, bn_ctx))
		goto end;
	bignum_free(rsa->phi);
	rsa->phi = bignum_new();
	if (!rsa->phi)
		goto end;
	if (!bignum_sub(rsa->phi, rsa->q, one, bn_ctx)
	 || !bignum_mul(rsa->phi, rsa->phi, tmp, bn_ctx))
		goto end;
	ret = 1;

end:
	bignum_ctx_release(bn_ctx, tmp);
	bignum_ctx_release(bn_ctx, one);
	return ret;
}

static int genprime(struct bignum *r, struct bignum *e, size_t b,
                    struct bignum_ctx *bn_ctx, struct bignum_gencb *gencb)
{
	struct bignum *tmp;
	struct bignum *one;
	int ret = 0;

	tmp = bignum_ctx_get(bn_ctx);
	one = bignum_ctx_get(bn_ctx);
	if (!tmp
	 || !one
	 || !bignum_one(one)
	 || !bignum_generate_prime(r, b, 0, NULL, NULL, gencb, bn_ctx)) /* XXX use e */
		goto end;
	ret = 1;

end:
	bignum_ctx_release(bn_ctx, tmp);
	bignum_ctx_release(bn_ctx, one);
	return ret;
}

static int genpq(struct rsa *rsa, size_t bits, struct bignum_ctx *bn_ctx,
                 struct bignum_gencb *gencb)
{
	bignum_free(rsa->p);
	bignum_free(rsa->q);
	rsa->p = bignum_new();
	rsa->q = bignum_new();
	if (!rsa->p
	 || !rsa->q)
		goto err;
	if (!genprime(rsa->p, rsa->e, (bits + 2) / 2, bn_ctx, gencb))
		goto err;
	while (1)
	{
		if (!genprime(rsa->q, rsa->e, bits - (bits + 2) / 2, bn_ctx, gencb))
			goto err;
		if (bignum_cmp(rsa->p, rsa->q))
			break;
	}
	return 1;

err:
	return 0;
}

int rsa_generate_key(struct rsa *rsa, size_t bits, size_t e,
                     struct bignum_gencb *gencb)
{
	struct bignum_ctx *bn_ctx;

	if (bits < 16)
		return 0;
	bn_ctx = bignum_ctx_new();
	if (!bn_ctx)
		return 0;
	memset(rsa, 0, sizeof(*rsa));
	rsa->e = bignum_new();
	rsa->n = bignum_new();
	rsa->d = bignum_new();
	if (!rsa->e
	 || !rsa->n
	 || !rsa->d
	 || !bignum_grow(rsa->e, e)
	 || !genpq(rsa, bits, bn_ctx, gencb)
	 || !genphi(rsa, bn_ctx)
	 || !bignum_mul(rsa->n, rsa->p, rsa->q, bn_ctx)
	 || !bignum_mod_inverse(rsa->d, rsa->e, rsa->phi, bn_ctx)
	 || !gencrt(rsa, bn_ctx))
		goto err;
	bignum_ctx_free(bn_ctx);
	return 1;

err:
	bignum_free(rsa->p);
	bignum_free(rsa->q);
	bignum_free(rsa->e);
	bignum_free(rsa->n);
	bignum_free(rsa->d);
	bignum_free(rsa->phi);
	bignum_free(rsa->dmp);
	bignum_free(rsa->dmq);
	bignum_free(rsa->coef);
	bignum_ctx_free(bn_ctx);
	memset(rsa, 0, sizeof(*rsa));
	return 0;
}
