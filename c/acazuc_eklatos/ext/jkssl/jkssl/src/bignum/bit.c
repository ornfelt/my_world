#include "bignum/bignum.h"

int bignum_clear_bit(struct bignum *bignum, uint32_t n)
{
	return bignum_clear_bit(bignum, n);
}

int bignum_is_bit_set(const struct bignum *bignum, uint32_t n)
{
	return __bignum_is_bit_set(bignum, n);
}

int bignum_set_bit(struct bignum *bignum, uint32_t n)
{
	return __bignum_set_bit(bignum, n);
}

int bignum_num_bits(const struct bignum *bignum)
{
	if (!bignum->len)
		return 0;
	int i = 8 * sizeof(*bignum->data) - 1;
	while (i >= 0)
	{
		if ((bignum->data[bignum->len - 1] >> i) & 1)
			break;
		--i;
	}
	return (bignum->len - 1) * 8 * sizeof(*bignum->data) + (i + 1);
}

int bignum_num_bytes(const struct bignum *bignum)
{
	return __bignum_num_bytes(bignum);
}
