#ifndef RINGBUF_H
#define RINGBUF_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ringbuf
{
	void *data;
	size_t size;
	size_t read_pos;
	size_t write_pos;
};

static inline void ringbuf_init(struct ringbuf *ringbuf, void *data,
                                size_t size)
{
	ringbuf->data = data;
	ringbuf->size = size;
	ringbuf->read_pos = 0;
	ringbuf->write_pos = 0;
}

static inline size_t ringbuf_write_size(const struct ringbuf *ringbuf)
{
	if (ringbuf->write_pos < ringbuf->read_pos)
		return ringbuf->read_pos - ringbuf->write_pos - 1;
	return ringbuf->size - 1 - ringbuf->write_pos + ringbuf->read_pos;
}

static inline size_t ringbuf_contiguous_write_size(const struct ringbuf *ringbuf)
{
	if (ringbuf->write_pos < ringbuf->read_pos)
		return ringbuf->read_pos - ringbuf->write_pos - 1;
	if (!ringbuf->read_pos)
		return ringbuf->size - ringbuf->write_pos - 1;
	return ringbuf->size - ringbuf->write_pos;
}

static inline size_t ringbuf_next_write_pos(const struct ringbuf *ringbuf,
                                            size_t size)
{
	return (ringbuf->write_pos + size) & (ringbuf->size - 1);
}

static inline void *ringbuf_write_ptr(const struct ringbuf *ringbuf)
{
	return (uint8_t*)ringbuf->data + ringbuf->write_pos;
}

static inline void *ringbuf_write_ptr_at(const struct ringbuf *ringbuf,
                                         size_t off)
{
	return (uint8_t*)ringbuf->data + ringbuf_next_write_pos(ringbuf, off);
}

static inline void ringbuf_advance_write(struct ringbuf *ringbuf, size_t size)
{
	ringbuf->write_pos = ringbuf_next_write_pos(ringbuf, size);
}

static inline void ringbuf_write_byte(struct ringbuf *ringbuf, uint8_t v)
{
	uint8_t *ptr = ringbuf_write_ptr(ringbuf);
	ringbuf_advance_write(ringbuf, 1);
	*ptr = v;
}

static inline size_t ringbuf_write(struct ringbuf *ringbuf, const void *data,
                                   size_t size)
{
	size_t wr = 0;
	while (size > 0)
	{
		size_t available = ringbuf_contiguous_write_size(ringbuf);
		if (!available)
			break;
		if (size < available)
			available = size;
		if (data)
			memcpy(ringbuf_write_ptr(ringbuf), (uint8_t*)data + wr,
			       available);
		else
			memset(ringbuf_write_ptr(ringbuf), 0, available);
		wr += available;
		size -= available;
		ringbuf_advance_write(ringbuf, available);
	}
	return wr;
}

static inline size_t ringbuf_read_size(const struct ringbuf *ringbuf)
{
	if (ringbuf->read_pos <= ringbuf->write_pos)
		return ringbuf->write_pos - ringbuf->read_pos;
	return ringbuf->size - ringbuf->read_pos + ringbuf->write_pos;
}

static inline size_t ringbuf_contiguous_read_size(const struct ringbuf *ringbuf)
{
	if (ringbuf->read_pos <= ringbuf->write_pos)
		return ringbuf->write_pos - ringbuf->read_pos;
	return ringbuf->size - ringbuf->read_pos;
}

static inline size_t ringbuf_next_read_pos(const struct ringbuf *ringbuf,
                                           size_t size)
{
	return (ringbuf->read_pos + size) & (ringbuf->size - 1);
}

static inline const void *ringbuf_read_ptr(const struct ringbuf *ringbuf)
{
	return (uint8_t*)ringbuf->data + ringbuf->read_pos;
}

static inline const void *ringbuf_read_ptr_at(const struct ringbuf *ringbuf,
                                              size_t off)
{
	return (uint8_t*)ringbuf->data + ringbuf_next_read_pos(ringbuf, off);
}

static inline void ringbuf_advance_read(struct ringbuf *ringbuf, size_t size)
{
	ringbuf->read_pos = ringbuf_next_read_pos(ringbuf, size);
}

static inline uint8_t ringbuf_read_byte(struct ringbuf *ringbuf)
{
	const uint8_t *ptr = ringbuf_read_ptr(ringbuf);
	ringbuf_advance_read(ringbuf, 1);
	return *ptr;
}

static inline size_t ringbuf_read(struct ringbuf *ringbuf, void *data,
                                  size_t size)
{
	size_t rd = 0;
	while (size > 0)
	{
		size_t available = ringbuf_contiguous_read_size(ringbuf);
		if (!available)
			break;
		if (size < available)
			available = size;
		if (data)
			memcpy((uint8_t*)data + rd, ringbuf_read_ptr(ringbuf),
			       available);
		rd += available;
		size -= available;
		ringbuf_advance_read(ringbuf, available);
	}
	return rd;
}

#ifdef __cplusplus
}
#endif

#endif
