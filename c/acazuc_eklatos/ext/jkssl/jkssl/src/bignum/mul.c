#include "bignum/bignum.h"

#include <string.h>

#if 0
# define KARATSUBA /* karatsuba works but reduces performances.. */
#endif

#ifdef KARATSUBA
static int karatsuba_split(struct bignum *l, struct bignum *h,
                           const struct bignum *n, uint32_t m)
{
	if (!bignum_reserve(l, m)
	 || !bignum_reserve(h, n->len - m))
		return 0;
	memcpy(l->data, n->data, sizeof(*l->data) * m);
	l->len = m;
	memcpy(h->data, &n->data[m], sizeof(*h->data) * (n->len - m));
	h->len = n->len - m;
	return 1;
}

static int karatsuba(struct bignum *r, const struct bignum *a,
                     const struct bignum *b, struct bignum_ctx *ctx)
{
	struct bignum *ha;
	struct bignum *la;
	struct bignum *hb;
	struct bignum *lb;
	struct bignum *z0;
	struct bignum *z1;
	struct bignum *z2;
	uint32_t m;
	int ret = 0;

	m = a->len < b->len ? a->len / 2 : b->len / 2;
	ha = bignum_ctx_get(ctx);
	la = bignum_ctx_get(ctx);
	hb = bignum_ctx_get(ctx);
	lb = bignum_ctx_get(ctx);
	z0 = bignum_ctx_get(ctx);
	z1 = bignum_ctx_get(ctx);
	z2 = bignum_ctx_get(ctx);
	if (!ha || !la || !hb || !lb || !z0 || !z1 || !z2
	 || !karatsuba_split(la, ha, a, m)
	 || !karatsuba_split(lb, hb, b, m)
	 || !bignum_uadd(z2, la, ha, ctx)
	 || !bignum_uadd(z0, lb, hb, ctx)
	 || !bignum_umul(z1, z0, z2, ctx)
	 || !bignum_umul(z0, la, lb, ctx)
	 || !bignum_umul(z2, ha, hb, ctx)
	 || !bignum_sub(z1, z1, z0, ctx)
	 || !bignum_sub(z1, z1, z2, ctx)
	 || !bignum_ulshift(la, z2, m * sizeof(bignum_word_t) * 8 * 2, ctx)
	 || !bignum_ulshift(ha, z1, m * sizeof(bignum_word_t) * 8, ctx)
	 || !bignum_add(lb, ha, la, ctx)
	 || !bignum_add(r, lb, z0, ctx))
		goto end;
	bignum_trunc(r);
	ret = 1;

end:
	bignum_ctx_release(ctx, ha);
	bignum_ctx_release(ctx, la);
	bignum_ctx_release(ctx, hb);
	bignum_ctx_release(ctx, lb);
	bignum_ctx_release(ctx, z0);
	bignum_ctx_release(ctx, z1);
	bignum_ctx_release(ctx, z2);
	return ret;
}
#endif

static void do_loop(struct bignum *tmp, const struct bignum *a,
                    const struct bignum *b, uint32_t i)
{
	bignum_dword_t carry;
	bignum_dword_t ai;
	uint32_t j;

	carry = 0;
	ai = a->data[i];
	for (j = 0; j < b->len; ++j)
	{
		carry += tmp->data[i + j] + ai * b->data[j];
		tmp->data[i + j] = carry;
		carry >>= sizeof(*a->data) * 8;
	}
	if (carry)
		tmp->data[i + j] += carry;
}

int bignum_umul(struct bignum *r, const struct bignum *a,
                const struct bignum *b, struct bignum_ctx *ctx)
{
	struct bignum *result;

	if (__bignum_is_zero(a)
	 || __bignum_is_zero(b))
	{
		__bignum_zero(r);
		return 1;
	}
	if (__bignum_is_one(a))
		return bignum_copy(r, b);
	if (__bignum_is_one(b))
		return bignum_copy(r, a);
#ifdef KARATSUBA
	if (a->len > 10 && b->len > 10)
		return karatsuba(r, a, b, ctx);
#endif
	result = bignum_ctx_get(ctx);
	if (!result)
		return 0;
	if (!bignum_resize(result, a->len + b->len))
	{
		bignum_ctx_release(ctx, result);
		return 0;
	}
	memset(result->data, 0, (a->len + b->len) * sizeof(*result->data));
	for (size_t i = 0; i < a->len; ++i)
		do_loop(result, a, b, i);
	bignum_trunc(result);
	bignum_swap(r, result);
	bignum_ctx_release(ctx, result);
	return 1;
}

int bignum_mul(struct bignum *r, const struct bignum *a,
               const struct bignum *b, struct bignum_ctx *ctx)
{
	int ret;
	if (__bignum_is_negative(a) == __bignum_is_negative(b))
	{
		struct bignum tmp_a = *a;
		struct bignum tmp_b = *b;
		__bignum_set_negative(&tmp_a, 0);
		__bignum_set_negative(&tmp_b, 0);
		ret = bignum_umul(r, &tmp_a, &tmp_b, ctx);
		__bignum_set_negative(r, 0);
	}
	else if (__bignum_is_negative(a))
	{
		struct bignum tmp_a = *a;
		__bignum_set_negative(&tmp_a, 0);
		ret = bignum_umul(r, &tmp_a, b, ctx);
		__bignum_set_negative(r, 1);
	}
	else
	{
		struct bignum tmp_b = *b;
		__bignum_set_negative(&tmp_b, 0);
		ret = bignum_mul(r, a, &tmp_b, ctx);
		__bignum_set_negative(r, 1);
	}
	return ret;
}

int bignum_usqr(struct bignum *r, const struct bignum *a,
                struct bignum_ctx *ctx)
{
	return bignum_umul(r, a, a, ctx);
}

int bignum_sqr(struct bignum *r, const struct bignum *a,
               struct bignum_ctx *ctx)
{
	return bignum_mul(r, a, a, ctx);
}

int bignum_mul_word(struct bignum *r, const struct bignum *a,
                    bignum_word_t b)
{
	struct bignum_ctx *ctx = bignum_ctx_new();
	if (!ctx)
		return 0;
	struct bignum bn;
	bignum_init(&bn);
	if (!__bignum_set_word(&bn, b))
	{
		bignum_ctx_free(ctx);
		bignum_clear(&bn);
		return 0;
	}
	int ret = bignum_mul(r, a, &bn, ctx);
	bignum_clear(&bn);
	bignum_ctx_free(ctx);
	return ret;
}
