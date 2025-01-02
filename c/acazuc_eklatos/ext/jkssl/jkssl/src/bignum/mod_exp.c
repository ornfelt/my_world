#include "bignum/bignum.h"

#include <stddef.h>

/* handbook of applied cryptography, 2.143 */
int bignum_umod_exp(struct bignum *r, const struct bignum *a,
                    const struct bignum *p, const struct bignum *m,
                    struct bignum_ctx *ctx)
{
	struct bignum *base;
	struct bignum *res;
	size_t n;
	int ret = 0;

	if (__bignum_is_zero(a))
	{
		__bignum_zero(r);
		return 1;
	}
	if (__bignum_is_zero(p))
		return __bignum_one(r);
	if (__bignum_is_one(p))
		return bignum_copy(r, a);
	base = bignum_ctx_get(ctx);
	res = bignum_ctx_get(ctx);
	if (!base
	 || !res
	 || !bignum_copy(base, a)
	 || !bignum_umod(base, base, m, ctx)
	 || !bignum_grow(res, 1))
		goto end;
	n = bignum_num_bits(p);
	for (size_t i = 0; i < n; ++i)
	{
		if (__bignum_is_bit_set(p, i))
		{
			if (!bignum_umul(res, res, base, ctx)
			 || !bignum_umod(res, res, m, ctx))
				goto end;
		}
		if (!bignum_usqr(base, base, ctx)
		 || !bignum_umod(base, base, m, ctx))
			goto end;
	}
	bignum_trunc(res);
	bignum_swap(r, res);
	ret = 1;

end:
	bignum_ctx_release(ctx, base);
	bignum_ctx_release(ctx, res);
	return ret;
}

int bignum_mod_exp(struct bignum *r, const struct bignum *a,
                   const struct bignum *p, const struct bignum *m,
                   struct bignum_ctx *ctx)
{
	int ret;
	if (!__bignum_is_negative(a))
	{
		ret = bignum_umod_exp(r, a, p, m, ctx);
		__bignum_set_negative(r, 0);
	}
	else
	{
		struct bignum tmp_a = *a;
		__bignum_set_negative(&tmp_a, 0);
		ret = bignum_umod_exp(r, &tmp_a, p, m, ctx);
		if (!ret)
			return 0;
		if (__bignum_is_bit_set(p, 0))
			ret = bignum_sub(r, m, r, ctx);
		else
			__bignum_set_negative(r, 0);
	}
	return ret;
}
