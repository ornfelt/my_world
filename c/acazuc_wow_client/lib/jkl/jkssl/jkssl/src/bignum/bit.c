#include <jkssl/bignum.h>

int bignum_clear_bit(struct bignum *bignum, uint32_t n)
{
	size_t dv = sizeof(*bignum->data) * 8;
	if ((n + dv - 1) / dv > bignum->len)
		return 0;
	bignum->data[n / dv] &= ~(1 << (n % dv));
	return 1;
}

int bignum_is_bit_set(const struct bignum *bignum, uint32_t n)
{
	uint32_t tmp = n / 8 / sizeof(*bignum->data);
	if (bignum->len <= tmp)
		return 0;
	return (bignum->data[tmp] >> (n % (8 * sizeof(*bignum->data)))) & 1;
}

int bignum_set_bit(struct bignum *bignum, uint32_t n)
{
	uint32_t tmp = n / 8 / sizeof(*bignum->data);
	uint32_t len = tmp + 1;
	if (bignum->len < len && !bignum_resize(bignum, len))
		return 0;
	bignum->data[tmp] |= bignum_word_one() << (n % (8 * sizeof(*bignum->data)));
	return 1;
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
	return (bignum_num_bits(bignum) + 7) / 8;
}
