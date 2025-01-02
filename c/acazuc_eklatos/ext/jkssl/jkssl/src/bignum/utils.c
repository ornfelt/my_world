#include "bignum/bignum.h"

#include <string.h>
#include <stdlib.h>

static uint32_t npot32(uint32_t val)
{
	val--;
	val |= val >> 1;
	val |= val >> 2;
	val |= val >> 4;
	val |= val >> 8;
	val |= val >> 16;
	return ++val;
}

int bignum_resize(struct bignum *bignum, uint32_t len)
{
	if (len <= bignum->len)
	{
		bignum->len = len;
		return 1;
	}
	if (!bignum_reserve(bignum, len))
		return 0;
	memset(bignum->data + bignum->len, 0,
	       (len - bignum->len) * sizeof(*bignum->data));
	bignum->len = len;
	return 1;
}

int bignum_reserve(struct bignum *bignum, uint32_t len)
{
	if (len <= bignum->cap)
		return 1;
	uint32_t npot = npot32(len);
	if (npot < 32)
		npot = 32;
	bignum_word_t *tmp = malloc(sizeof(*tmp) * npot);
	if (!tmp)
		return 0;
	memcpy(tmp, bignum->data, bignum->len * sizeof(*tmp));
	free(bignum->data);
	bignum->data = tmp;
	bignum->cap = npot;
	return 1;
}

struct bignum *bignum_new(void)
{
	struct bignum *result;

	result = malloc(sizeof(*result));
	if (!result)
		return NULL;
	bignum_init(result);
	return result;
}

void bignum_init(struct bignum *bignum)
{
	memset(bignum, 0, sizeof(*bignum));
}

int bignum_grow(struct bignum *bignum, bignum_word_t a)
{
	if (!bignum_resize(bignum, bignum->len + 1))
		return 0;
	bignum->data[bignum->len - 1] = a;
	return 1;
}

int bignum_grow_front(struct bignum *bignum, bignum_word_t a)
{
	if (!bignum_resize(bignum, bignum->len + 1))
		return 0;
	memmove(bignum->data + 1, bignum->data,
	        sizeof(*bignum->data) * (bignum->len - 1));
	bignum->data[0] = a;
	return 1;
}

void bignum_free(struct bignum *bignum)
{
	if (!bignum)
		return;
	bignum_clear(bignum);
	free(bignum);
}

int bignum_copy(struct bignum *dst, const struct bignum *src)
{
	if (dst == src)
		return 1;
	if (!bignum_resize(dst, src->len))
		return 0;
	memcpy(dst->data, src->data, src->len * sizeof(*src->data));
	__bignum_set_negative(dst, __bignum_is_negative(src));
	return 1;
}

struct bignum *bignum_dup(const struct bignum *bignum)
{
	struct bignum *tmp = bignum_new();
	if (!tmp)
		return NULL;
	if (!bignum_copy(tmp, bignum))
	{
		free(tmp);
		return NULL;
	}
	return tmp;
}

void bignum_clear(struct bignum *bignum)
{
	free(bignum->data);
}

void bignum_move(struct bignum *a, struct bignum *b)
{
	if (a == b)
		return;
	free(a->data);
	a->data = b->data;
	a->len = b->len;
	a->cap = b->cap;
	a->sign = b->sign;
	b->data = NULL;
	b->len = 0;
	b->cap = 0;
	b->sign = 0;
}

void bignum_swap(struct bignum *a, struct bignum *b)
{
	struct bignum tmp;
	if (a == b)
		return;
	memcpy(&tmp, b, sizeof(*a));
	memcpy(b, a, sizeof(*a));
	memcpy(a, &tmp, sizeof(*a));
}

int bignum_set_word(struct bignum *bignum, bignum_word_t word)
{
	return __bignum_set_word(bignum, word);
}

void bignum_zero(struct bignum *bignum)
{
	__bignum_zero(bignum);
}

int bignum_one(struct bignum *bignum)
{
	return __bignum_one(bignum);
}

int bignum_is_word(const struct bignum *bignum, bignum_word_t word)
{
	return __bignum_is_word(bignum, word);
}

int bignum_is_zero(const struct bignum *bignum)
{
	return __bignum_is_zero(bignum);
}

int bignum_is_one(const struct bignum *bignum)
{
	return __bignum_is_one(bignum);
}

int bignum_is_negative(const struct bignum *bignum)
{
	return __bignum_is_negative(bignum);
}

void bignum_set_negative(struct bignum *bignum, int neg)
{
	__bignum_set_negative(bignum, neg);
}
