#include "bignum/bignum.h"

int bignum_uadd(struct bignum *r, const struct bignum *a,
                const struct bignum *b, struct bignum_ctx *ctx)
{
	bignum_word_t carry = 0;
	struct bignum *result;
	int ret = 0;

	if (b->len > a->len)
	{
		const struct bignum *tmp = a;
		a = b;
		b = tmp;
	}
	result = bignum_ctx_get(ctx);
	if (!result)
		return 0;
	if (!bignum_resize(result, a->len + 1))
		goto end;
	for (uint32_t i = 0; i < a->len; ++i)
	{
		bignum_word_t sum = a->data[i];
		if (i < b->len)
			sum += b->data[i];
		bignum_word_t tmp_carry = sum < a->data[i];
		bignum_word_t word = sum + carry;
		carry = word < sum;
		result->data[i] = word;
		carry |= tmp_carry;
	}
	result->data[a->len] = carry;
	bignum_trunc(result);
	bignum_move(r, result);
	ret = 1;

end:
	bignum_ctx_release(ctx, result);
	return ret;
}

int bignum_add(struct bignum *r, const struct bignum *a,
               const struct bignum *b, struct bignum_ctx *ctx)
{
	int ret;
	if (__bignum_is_negative(a))
	{
		if (__bignum_is_negative(b))
		{
			struct bignum tmp_a = *a;
			struct bignum tmp_b = *b;
			__bignum_set_negative(&tmp_a, 0);
			__bignum_set_negative(&tmp_b, 0);
			ret = bignum_uadd(r, &tmp_a, &tmp_b, ctx);
			__bignum_set_negative(r, 1);
		}
		else
		{
			struct bignum tmp_a = *a;
			__bignum_set_negative(&tmp_a, 0);
			ret = bignum_sub(r, b, &tmp_a, ctx);
		}
	}
	else
	{
		if (__bignum_is_negative(b))
		{
			struct bignum tmp_b = *b;
			__bignum_set_negative(&tmp_b, 0);
			ret = bignum_sub(r, a, &tmp_b, ctx);
		}
		else
		{
			ret = bignum_uadd(r, a, b, ctx);
			__bignum_set_negative(r, 0);
		}
	}
	return ret;
}
