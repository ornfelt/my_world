#include <jkssl/bignum.h>

#include <stddef.h>

static int do_exec(struct bignum *dv, struct bignum *rm,
                   const struct bignum *a, const struct bignum *b,
                   struct bignum_ctx *ctx)
{
	struct bignum *r;
	struct bignum *q;
	uint64_t i;
	int ret = 0;

	if (bignum_ucmp(a, b) < 0)
	{
		if (dv)
			bignum_zero(dv);
		if (rm)
		{
			bignum_copy(rm, a);
			bignum_trunc(rm);
		}
		return 1;
	}
	r = bignum_ctx_get(ctx);
	q = bignum_ctx_get(ctx);
	if (!r || !q)
		goto end;
	if (!bignum_reserve(r, b->len)
	 || !bignum_reserve(q, a->len - b->len + 1))
		goto end;
	i = bignum_num_bits(a);
	while (1)
	{
		if (!bignum_ulshift1(r, r, ctx))
			goto end;
		if (bignum_is_bit_set(a, i) && !bignum_set_bit(r, 0))
			goto end;
		if (bignum_ucmp(r, b) >= 0)
		{
			if (!bignum_usub(r, r, b, ctx))
				goto end;
			if (!bignum_set_bit(q, i))
				goto end;
		}
		if (!i)
			break;
		i--;
	}
	if (rm)
	{
		bignum_trunc(r);
		bignum_swap(rm, r);
	}
	if (dv)
	{
		bignum_trunc(q);
		bignum_swap(dv, q);
	}
	ret = 1;

end:
	bignum_ctx_release(ctx, r);
	bignum_ctx_release(ctx, q);
	return ret;
}

int bignum_udiv_mod(struct bignum *dv, struct bignum *rm,
                    const struct bignum *a, const struct bignum *b,
                    struct bignum_ctx *ctx)
{
	if (bignum_is_zero(b))
		return 0;
	if (bignum_is_zero(a))
	{
		if (dv)
			bignum_zero(dv);
		if (rm)
			bignum_zero(rm);
		return 1;
	}
	if (bignum_is_one(b))
	{
		if (dv)
		{
			if (!bignum_copy(dv, a))
				return 0;
			bignum_trunc(dv);
		}
		if (rm)
			bignum_zero(rm);
		return 1;
	}
	return do_exec(dv, rm, a, b, ctx);
}

int bignum_div_mod(struct bignum *dv, struct bignum *rm,
                   const struct bignum *a, const struct bignum *b,
                   struct bignum_ctx *ctx)
{
	struct bignum tmp_a;
	struct bignum tmp_b;
	struct bignum tmp;
	int a_negative;
	int b_negative;
	int ret;

	bignum_init(&tmp);
	a_negative = bignum_is_negative(a);
	b_negative = bignum_is_negative(b);
	if (rm && a_negative != b_negative && !bignum_copy(&tmp, b))
	{
		ret = 0;
		goto end;
	}
	tmp_a = *a;
	tmp_b = *b;
	bignum_set_negative(&tmp_a, 0);
	bignum_set_negative(&tmp_b, 0);
	ret = bignum_udiv_mod(dv, rm, &tmp_a, &tmp_b, ctx);
	if (dv)
		bignum_set_negative(dv, a_negative != b_negative);
	if (rm)
	{
		if (a_negative && b_negative)
		{
			bignum_set_negative(rm, 1);
		}
		else if (a_negative || b_negative)
		{
			bignum_set_negative(rm, 0);
			bignum_set_negative(&tmp, 0);
			if (!bignum_is_zero(rm) && !bignum_sub(rm, &tmp, rm, ctx))
			{
				ret = 0;
				goto end;
			}
			bignum_set_negative(rm, b_negative);
		}
	}
end:
	bignum_clear(&tmp);
	return ret;
}

int bignum_div(struct bignum *r, const struct bignum *a,
               const struct bignum *b, struct bignum_ctx *ctx)
{
	return bignum_div_mod(r, NULL, a, b, ctx);
}

int bignum_udiv(struct bignum *r, const struct bignum *a,
               const struct bignum *b, struct bignum_ctx *ctx)
{
	return bignum_udiv_mod(r, NULL, a, b, ctx);
}

int bignum_mod(struct bignum *r, const struct bignum *a,
               const struct bignum *b, struct bignum_ctx *ctx)
{
	return bignum_div_mod(NULL, r, a, b, ctx);
}

int bignum_umod(struct bignum *r, const struct bignum *a,
                const struct bignum *b, struct bignum_ctx *ctx)
{
	return bignum_udiv_mod(NULL, r, a, b, ctx);
}
