#include <jkssl/bignum.h>

int bignum_ucmp(const struct bignum *a, const struct bignum *b)
{
	size_t start;
	if (a->len > b->len)
	{
		for (size_t i = a->len - 1; i >= b->len; --i)
		{
			if (a->data[i])
				return 1;
		}
		start = b->len - 1;
	}
	else if (b->len > a->len)
	{
		for (size_t i = b->len - 1; i >= a->len; --i)
		{
			if (b->data[i])
				return -1;
		}
		start = a->len - 1;
	}
	else
	{
		start = a->len - 1;
	}
	while (1)
	{
		if (a->data[start] > b->data[start])
			return 1;
		if (b->data[start] > a->data[start])
			return -1;
		if (!start)
			break;
		--start;
	}
	return 0;
}

int bignum_cmp(const struct bignum *a, const struct bignum *b)
{
	if (!bignum_is_negative(a) && !bignum_is_negative(b))
		return bignum_ucmp(a, b);
	if (bignum_is_negative(a) && bignum_is_negative(b))
		return -bignum_ucmp(a, b);
	if (bignum_is_zero(a) && bignum_is_zero(b))
		return 0;
	if (bignum_is_negative(a) && !bignum_is_negative(b))
		return -1;
	if (!bignum_is_negative(a) && bignum_is_negative(b))
		return 1;
	return 0;
}
