#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <stdint.h>
#include <stddef.h>

struct bitstream
{
	int (*get)(struct bitstream *bs);
	int (*put)(struct bitstream *bs);
	void *userdata;
	uint8_t len;
	size_t buf;
};

static inline void bs_init_read(struct bitstream *bs)
{
	bs->len = 0;
	bs->buf = 0;
}

static inline void bs_init_write(struct bitstream *bs)
{
	bs->len = sizeof(bs->buf) * 8;
	bs->buf = 0;
}

static inline int bs_getbit(struct bitstream *bs)
{
	if (!bs->len)
	{
		if (bs->get(bs))
			return -1;
		if (!bs->len)
			return -1;
	}
	bs->len--;
	return (bs->buf >> bs->len) & 1;
}

static inline size_t bs_getbits(struct bitstream *bs, uint32_t len)
{
	if (len > bs->len)
	{
		if (bs->get(bs))
			return (size_t)-1;
		if (len > bs->len)
			return (size_t)-1;
	}
	bs->len -= len;
	return (bs->buf >> bs->len) & ((1 << len) - 1);
}

static inline int bs_putbit(struct bitstream *bs, size_t value)
{
	if (!bs->len)
	{
		int ret = bs->put(bs);
		if (ret)
			return ret;
	}
	bs->len--;
	value &= 1;
	bs->buf |= value << bs->len;
	return 0;
}

static inline int bs_putbits(struct bitstream *bs, size_t value, uint32_t len)
{
	if (len > bs->len)
	{
		int ret = bs->put(bs);
		if (ret)
			return ret;
	}
	bs->len -= len;
	value &= (1 << len) - 1;
	bs->buf |= value << bs->len;
	return 0;
}

static inline int bs_flushbits(struct bitstream *bs)
{
	if (bs->len == sizeof(bs->buf) * 8)
		return 0;
	uint8_t cur = sizeof(bs->buf) * 8 - bs->len;
	if (bs_putbits(bs, 0, 8 - (cur % 8)))
		return 1;
	return bs->put(bs);
}

#endif
