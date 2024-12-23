#include <jkssl/bignum.h>

static void do_rshift(bignum_word_t *tmp, const struct bignum *a,
                      uint32_t *next, uint32_t s8)
{
	uint32_t nms8 = *next % s8;
	*tmp = (a->data[*next / s8] & ((bignum_word_t)-1 << nms8)) >> nms8;
	if ((*next += s8) >= a->len * s8)
		return;
	nms8 = *next % s8;
	*tmp |= (a->data[*next / s8] & ((bignum_word_t)-1 >> (s8 - nms8))) << (s8 - nms8);
}

int bignum_urshift(struct bignum *r, const struct bignum *a, uint32_t n,
                   struct bignum_ctx *ctx)
{
	struct bignum *tmp;
	uint32_t next;
	uint32_t s8;
	uint32_t len;

	if (!n)
	{
		bignum_copy(r, a);
		bignum_trunc(r);
		return 1;
	}
	if (bignum_is_zero(a) || n >= (uint32_t)bignum_num_bits(a))
	{
		bignum_zero(r);
		return 1;
	}
	s8 = sizeof(*tmp->data) * 8;
	len = (bignum_num_bits(a) - n + s8 - 1) / s8;
	tmp = bignum_ctx_get(ctx);
	if (!tmp)
		return 0;
	if (!bignum_reserve(tmp, len))
	{
		bignum_ctx_release(ctx, tmp);
		return 0;
	}
	tmp->len = len;
	for (uint32_t i = 0; i < tmp->len; ++i)
	{
		next = i * s8 + n;
		do_rshift(&tmp->data[i], a, &next, s8);
	}
	bignum_trunc(tmp);
	bignum_swap(r, tmp);
	bignum_ctx_release(ctx, tmp);
	return 1;
}

int bignum_rshift(struct bignum *r, const struct bignum *a, uint32_t n,
                  struct bignum_ctx *ctx)
{
	int negative = bignum_is_negative(a);
	int ret = bignum_urshift(r, a, n, ctx);
	bignum_set_negative(r, negative);
	return ret;
}

int bignum_urshift1(struct bignum *r, const struct bignum *a,
                    struct bignum_ctx *ctx)
{
	bignum_word_t carry;
	bignum_word_t tmp;
	uint32_t i;

	(void)ctx;
	if (bignum_is_zero(a))
	{
		bignum_zero(r);
		return 1;
	}
	if (!bignum_resize(r, a->len))
		return 0;
	carry = 0;
	i = a->len - 1;
	do
	{
		tmp = (a->data[i] & 0x1) << (sizeof(*a->data) * 8 - 1);
		r->data[i] = (a->data[i] >> 1) | carry;
		carry = tmp;
	} while (i--);
	bignum_trunc(r);
	return 1;
}

int bignum_rshift1(struct bignum *r, const struct bignum *a,
                   struct bignum_ctx *ctx)
{
	int negative = bignum_is_negative(a);
	int ret = bignum_urshift1(r, a, ctx);
	bignum_set_negative(r, negative);
	return ret;
}

static void do_lshift(bignum_word_t *tmp, const struct bignum *a,
                      int32_t next, uint32_t s8)
{
	int32_t nms8;

	if (next >= 0)
	{
		nms8 = next % s8;
		if (!nms8)
		{
			*tmp = a->data[next / s8];
			return;
		}
		*tmp = (a->data[next / s8] & ((bignum_word_t)-1 << nms8)) >> nms8;
	}
	else
	{
		*tmp = 0;
	}
	next += s8;
	if (next >= (int32_t)a->len * (int32_t)s8)
		return;
	if (next > 0)
	{
		nms8 = next % s8;
		*tmp |= (a->data[next / s8] & ((bignum_word_t)-1 >> (s8 - nms8))) << (s8 - nms8);
	}
}

int bignum_ulshift(struct bignum *r, const struct bignum *a, uint32_t n,
                   struct bignum_ctx *ctx)
{
	struct bignum *tmp;
	int32_t next;
	uint32_t s8;
	uint32_t len;

	if (!n)
	{
		bignum_copy(r, a);
		return 1;
	}
	if (bignum_is_zero(a))
	{
		bignum_zero(r);
		return 1;
	}
	s8 = sizeof(*tmp->data) * 8;
	len = (bignum_num_bits(a) + n + s8 - 1) / s8;
	tmp = bignum_ctx_get(ctx);
	if (!tmp)
		return 0;
	if (!bignum_reserve(tmp, len))
	{
		bignum_ctx_release(ctx, tmp);
		return 0;
	}
	tmp->len = len;
	for (uint32_t i = 0; i < tmp->len; ++i)
	{
		next = i * s8 - n;
		do_lshift(&tmp->data[i], a, next, s8);
	}
	bignum_trunc(tmp);
	bignum_move(r, tmp);
	bignum_ctx_release(ctx, tmp);
	return 1;
}

int bignum_lshift(struct bignum *r, const struct bignum *a, uint32_t n,
                  struct bignum_ctx *ctx)
{
	int negative = bignum_is_negative(a);
	int ret = bignum_ulshift(r, a, n, ctx);
	bignum_set_negative(r, negative);
	return ret;
}

int bignum_ulshift1(struct bignum *r, const struct bignum *a,
                    struct bignum_ctx *ctx)
{
	bignum_word_t carry;
	bignum_word_t tmp;
	bignum_word_t msk;
	uint32_t i;
	int add;

	(void)ctx;
	if (bignum_is_bit_set(a, a->len * sizeof(*a->data) * 8 - 1))
		add = 1;
	else
		add = 0;
	if (!bignum_resize(r, a->len + add))
		return 0;
	carry = 0;
	msk = (bignum_word_t)1 << (sizeof(*a->data) * 8 - 1);
	for (i = 0; i < a->len; ++i)
	{
		tmp = a->data[i] & msk;
		tmp >>= sizeof(*a->data) * 8 - 1;
		r->data[i] = (a->data[i] << 1) | carry;
		carry = tmp;
	}
	if (carry)
		r->data[i] = carry;
	bignum_trunc(r);
	return 1;
}

int bignum_lshift1(struct bignum *r, const struct bignum *a,
                          struct bignum_ctx *ctx)
{
	int negative = bignum_is_negative(a);
	int ret = bignum_ulshift1(r, a, ctx);
	bignum_set_negative(r, negative);
	return ret;
}
