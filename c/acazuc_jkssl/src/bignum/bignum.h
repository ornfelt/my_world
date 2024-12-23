#ifndef JKSSL_BIGNUM_BIGNUM_H
#define JKSSL_BIGNUM_BIGNUM_H

#include <jkssl/bignum.h>

#define BIGNUM_BITS_PER_WORD (8 * sizeof(bignum_word_t))

struct bignum
{
	bignum_word_t *data;
	uint32_t cap;
	uint32_t len;
	int sign;
};

struct bignum_ctx_entry
{
	struct bignum *bignum;
	int used;
};

struct bignum_ctx
{
	struct bignum_ctx_entry *entries;
	uint32_t len;
};

struct bignum_gencb
{
	void *arg;
	bignum_gencb_cb_t callback;
};

static inline void bignum_trunc(struct bignum *bignum)
{
	while (bignum->len > 1 && !bignum->data[bignum->len - 1])
		--bignum->len;
}

static inline bignum_word_t bignum_word_one(void)
{
	return 1;
}

static inline int __bignum_set_word(struct bignum *bignum, bignum_word_t word)
{
	if (!bignum_resize(bignum, 1))
		return 0;
	bignum->data[0] = word;
	return 1;
}

static inline void __bignum_zero(struct bignum *bignum)
{
	bignum->len = 0;
	bignum->sign = 0;
}

static inline int __bignum_one(struct bignum *bignum)
{
	return __bignum_set_word(bignum, 1);
}

static inline int __bignum_is_word(const struct bignum *bignum,
                                   bignum_word_t word)
{
	if (!bignum->len)
		return !word;
	if (word && bignum->sign)
		return 0;
	if (bignum->data[0] != word)
		return 0;
	for (uint32_t i = 1; i < bignum->len; ++i)
	{
		if (bignum->data[i])
			return 0;
	}
	return 1;
}

static inline int __bignum_is_zero(const struct bignum *bignum)
{
	return __bignum_is_word(bignum, 0);
}

static inline int __bignum_is_one(const struct bignum *bignum)
{
	return __bignum_is_word(bignum, 1);
}

static inline int __bignum_is_negative(const struct bignum *bignum)
{
	return bignum->sign;
}

static inline void __bignum_set_negative(struct bignum *bignum, int sign)
{
	bignum->sign = !!sign;
}

static inline int __bignum_clear_bit(struct bignum *bignum, uint32_t n)
{
	size_t dv = sizeof(*bignum->data) * 8;
	if ((n + dv - 1) / dv > bignum->len)
		return 0;
	bignum->data[n / dv] &= ~(1 << (n % dv));
	return 1;
}

static inline int __bignum_is_bit_set(const struct bignum *bignum, uint32_t n)
{
	uint32_t tmp = n / 8 / sizeof(*bignum->data);
	if (bignum->len <= tmp)
		return 0;
	return (bignum->data[tmp] >> (n % (8 * sizeof(*bignum->data)))) & 1;
}

static inline int __bignum_set_bit(struct bignum *bignum, uint32_t n)
{
	uint32_t tmp = n / 8 / sizeof(*bignum->data);
	uint32_t len = tmp + 1;
	if (bignum->len < len && !bignum_resize(bignum, len))
		return 0;
	bignum->data[tmp] |= bignum_word_one() << (n % (8 * sizeof(*bignum->data)));
	return 1;
}

static inline int __bignum_num_bytes(const struct bignum *bignum)
{
	return (bignum_num_bits(bignum) + 7) / 8;
}

#endif
