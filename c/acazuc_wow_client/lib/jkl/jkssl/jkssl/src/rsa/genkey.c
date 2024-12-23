#include <jkssl/bignum.h>
#include <jkssl/rsa.h>

#include <string.h>
#include <stdio.h>

static int gencrt(struct rsa_ctx *ctx, struct bignum_ctx *bn_ctx)
{
	struct bignum *tmp;
	struct bignum *one;
	int ret = 0;

	tmp = bignum_ctx_get(bn_ctx);
	one = bignum_ctx_get(bn_ctx);
	ctx->dmp = bignum_new();
	ctx->dmq = bignum_new();
	ctx->coef = bignum_new();
	if (!tmp
	 || !one
	 || !ctx->dmp
	 || !ctx->dmq
	 || !ctx->coef
	 || !bignum_one(one)
	 || !bignum_copy(tmp, ctx->p)
	 || !bignum_sub(tmp, tmp, one, bn_ctx)
	 || !bignum_mod(ctx->dmp, ctx->d, tmp, bn_ctx)
	 || !bignum_copy(tmp, ctx->q)
	 || !bignum_sub(tmp, tmp, one, bn_ctx)
	 || !bignum_mod(ctx->dmq, ctx->d, tmp, bn_ctx)
	 || !bignum_mod_inverse(ctx->coef, ctx->q, ctx->p, bn_ctx))
		goto end;
	ret = 1;

end:
	bignum_ctx_release(bn_ctx, tmp);
	bignum_ctx_release(bn_ctx, one);
	return ret;
}

static int genphi(struct rsa_ctx *ctx, struct bignum_ctx *bn_ctx)
{
	struct bignum *tmp;
	struct bignum *one;
	int ret = 0;

	tmp = bignum_ctx_get(bn_ctx);
	one = bignum_ctx_get(bn_ctx);
	if (!tmp
	 || !one
	 || !bignum_one(one)
	 || !bignum_sub(tmp, ctx->p, one, bn_ctx))
		goto end;
	ctx->phi = bignum_new();
	if (!ctx->phi)
		goto end;
	if (!bignum_sub(ctx->phi, ctx->q, one, bn_ctx)
	 || !bignum_mul(ctx->phi, ctx->phi, tmp, bn_ctx))
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

static int genpq(struct rsa_ctx *ctx, size_t bits, struct bignum_ctx *bn_ctx,
                 struct bignum_gencb *gencb)
{
	ctx->p = bignum_new();
	if (!ctx->p)
		goto err;
	if (!genprime(ctx->p, ctx->e, (bits + 2) / 2, bn_ctx, gencb))
		goto err;
	ctx->q = bignum_new();
	if (!ctx->q)
		goto err;
	while (1)
	{
		if (!genprime(ctx->q, ctx->e, bits - (bits + 2) / 2, bn_ctx, gencb))
			goto err;
		if (bignum_cmp(ctx->p, ctx->q))
			break;
	}
	return 1;

err:
	return 0;
}

int rsa_generate_key(struct rsa_ctx *ctx, size_t bits, size_t e,
                     struct bignum_gencb *gencb)
{
	struct bignum_ctx *bn_ctx;

	if (bits < 16)
		return 0;
	bn_ctx = bignum_ctx_new();
	if (!bn_ctx)
		return 0;
	memset(ctx, 0, sizeof(*ctx));
	ctx->e = bignum_new();
	ctx->n = bignum_new();
	ctx->d = bignum_new();
	if (!ctx->e
	 || !ctx->n
	 || !ctx->d
	 || !bignum_grow(ctx->e, e)
	 || !genpq(ctx, bits, bn_ctx, gencb)
	 || !genphi(ctx, bn_ctx)
	 || !bignum_mul(ctx->n, ctx->p, ctx->q, bn_ctx)
	 || !bignum_mod_inverse(ctx->d, ctx->e, ctx->phi, bn_ctx)
	 || !gencrt(ctx, bn_ctx))
		goto err;
	bignum_ctx_free(bn_ctx);
	return 1;

err:
	bignum_free(ctx->p);
	bignum_free(ctx->q);
	bignum_free(ctx->e);
	bignum_free(ctx->n);
	bignum_free(ctx->d);
	bignum_free(ctx->phi);
	bignum_free(ctx->dmp);
	bignum_free(ctx->dmq);
	bignum_free(ctx->coef);
	bignum_ctx_free(bn_ctx);
	memset(ctx, 0, sizeof(*ctx));
	return 0;
}
