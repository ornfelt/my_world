#include "bignum/bignum.h"

int bignum_usub(struct bignum *r, const struct bignum *a,
                const struct bignum *b, struct bignum_ctx *ctx)
{
	bignum_word_t carry;
	struct bignum *result;

	if (bignum_ucmp(a, b) < 0)
		return 0;
	result = bignum_ctx_get(ctx);
	if (!result)
		return 0;
	if (!bignum_resize(result, a->len))
	{
		bignum_ctx_release(ctx, result);
		return 0;
	}
	carry = 0;
	for (uint32_t i = 0; i < a->len; ++i)
	{
		bignum_word_t sum = a->data[i];
		if (i < b->len)
			sum -= b->data[i];
		bignum_word_t tmp_carry = sum > a->data[i];
		bignum_word_t word = sum - carry;
		carry = word > sum;
		result->data[i] = word;
		carry |= tmp_carry;
	}
	bignum_trunc(result);
	bignum_swap(r, result);
	bignum_ctx_release(ctx, result);
	return 1;
}

int bignum_sub(struct bignum *r, const struct bignum *a,
               const struct bignum *b, struct bignum_ctx *ctx)
{
	int ret;

	if (__bignum_is_negative(a))
	{
		if (__bignum_is_negative(b))
		{
			struct bignum tmp_b = *b;
			__bignum_set_negative(&tmp_b, 0);
			ret = bignum_add(r, a, &tmp_b, ctx);
		}
		else
		{
			struct bignum tmp_a = *a;
			__bignum_set_negative(&tmp_a, 0);
			ret = bignum_uadd(r, &tmp_a, b, ctx);
			__bignum_set_negative(r, 1);
		}
	}
	else
	{
		if (!__bignum_is_negative(b))
		{
			if (bignum_cmp(a, b) >= 0)
			{
				ret = bignum_usub(r, a, b, ctx);
				__bignum_set_negative(r, 0);
			}
			else
			{
				ret = bignum_usub(r, b, a, ctx);
				__bignum_set_negative(r, 1);
			}
		}
		else
		{
			struct bignum tmp_b = *b;
			__bignum_set_negative(&tmp_b, 0);
			ret = bignum_uadd(r, a, &tmp_b, ctx);
		}
	}
	return ret;
}
