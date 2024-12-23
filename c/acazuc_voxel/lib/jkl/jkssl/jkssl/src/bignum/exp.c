#include <jkssl/bignum.h>

int bignum_uexp(struct bignum *r, const struct bignum *a,
                const struct bignum *p, struct bignum_ctx *ctx)
{
	struct bignum *base;
	struct bignum *ex;
	struct bignum *res;
	int ret = 0;

	if (bignum_is_zero(a))
	{
		bignum_zero(r);
		return 1;
	}
	if (bignum_is_zero(p))
		return bignum_one(r);
	if (bignum_is_one(p))
		return bignum_copy(r, a);
	base = bignum_ctx_get(ctx);
	ex = bignum_ctx_get(ctx);
	res = bignum_ctx_get(ctx);
	if (!base || !ex || !res)
		goto end;
	if (!bignum_copy(ex, p)
	 || !bignum_copy(base, a)
	 || !bignum_grow(res, 1))
		goto end;
	while (!bignum_is_zero(ex))
	{
		if (bignum_is_bit_set(ex, 0))
		{
			if (!bignum_umul(res, res, base, ctx))
				goto end;
		}
		if (!bignum_umul(base, base, base, ctx))
			goto end;
		if (!bignum_urshift1(ex, ex, ctx))
			goto end;
	}
	bignum_trunc(res);
	bignum_swap(r, res);
	ret = 1;

end:
	bignum_ctx_release(ctx, base);
	bignum_ctx_release(ctx, res);
	bignum_ctx_release(ctx, ex);
	return ret;
}

int bignum_exp(struct bignum *r, const struct bignum *a,
               const struct bignum *p, struct bignum_ctx *ctx)
{
	int ret;
	if (!bignum_is_negative(a))
	{
		ret = bignum_uexp(r, a, p, ctx);
		bignum_set_negative(r, 0);
	}
	else
	{
		struct bignum tmp_a = *a;
		bignum_set_negative(&tmp_a, 0);
		ret = bignum_uexp(r, &tmp_a, p, ctx);
		bignum_set_negative(r, bignum_is_bit_set(p, 0));
	}
	return ret;
}
