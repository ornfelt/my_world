#ifndef BITSTREAM_H
#define BITSTREAM_H

#include "ringbuf.h"

#ifdef __cplusplus
extern "C" {
#endif

struct bitstream
{
	/* XXX store the current working byte to avoid ringbuf fetch each bit */
	struct ringbuf *ringbuf;
	size_t pos; /* in the current byte */
};

static inline size_t bitstream_avail_read(struct bitstream *bs)
{
	return ringbuf_read_size(bs->ringbuf) * 8 - bs->pos;
}

static inline size_t bitstream_avail_write(struct bitstream *bs)
{
	return ringbuf_write_size(bs->ringbuf) * 8 - bs->pos;
}

static inline uint8_t bitstream_getbit(struct bitstream *bs, size_t pos)
{
	size_t off = bs->pos + pos;
	return ((*(uint8_t*)ringbuf_read_ptr_at(bs->ringbuf, off / 8)) >> (off % 8)) & 1;
}

static inline void bitstream_putbit(struct bitstream *bs, size_t pos, uint8_t bit)
{
	size_t off = bs->pos + pos;
	uint8_t *dst = (uint8_t*)ringbuf_write_ptr_at(bs->ringbuf, off / 8);
	if (!(off % 8))
		*dst = 0;
	*dst |= (bit & 1) << (off % 8);
}

static inline int bitstream_has_read(struct bitstream *bs, size_t bits)
{
	return bits <= bitstream_avail_read(bs);
}

static inline int bitstream_has_write(struct bitstream *bs, size_t bits)
{
	return bits <= bitstream_avail_write(bs);
}

static inline void bitstream_peek(struct bitstream *bs, uint32_t *data,
                                  size_t bits)
{
	*data = 0;
	for (size_t i = 0; i < bits; ++i)
		*data |= bitstream_getbit(bs, i) << i;
}

static inline void bitstream_skip(struct bitstream *bs, size_t bits)
{
	size_t next = bs->pos + bits;
	bs->pos = next % 8;
	ringbuf_advance_read(bs->ringbuf, next / 8);
}

static inline void bitstream_read(struct bitstream *bs, uint32_t *data,
                                  size_t bits)
{
	bitstream_peek(bs, data, bits);
	bitstream_skip(bs, bits);
}

static inline void bitstream_write(struct bitstream *bs, uint32_t data,
                                   size_t bits)
{
	for (size_t i = 0; i < bits; ++i)
		bitstream_putbit(bs, i, data >> i);
	size_t next = bs->pos + bits;
	ringbuf_advance_write(bs->ringbuf, next / 8);
	bs->pos = next % 8;
}

#ifdef __cplusplus
}
#endif

#endif
