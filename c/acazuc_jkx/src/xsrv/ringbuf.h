#ifndef RINGBUF_H
#define RINGBUF_H

#include <stddef.h>
#include <stdint.h>

struct ringbuf
{
	void *data;
	size_t size;
	size_t read_pos;
	size_t write_pos;
	int bitswap;
};

int ringbuf_init(struct ringbuf *ringbuf, size_t size);
void ringbuf_destroy(struct ringbuf *ringbuf);
int ringbuf_resize(struct ringbuf *ringbuf, size_t size);
size_t ringbuf_write(struct ringbuf *ringbuf, const void *data, size_t size);
size_t ringbuf_read(struct ringbuf *ringbuf, void *data, size_t size);
size_t ringbuf_peek(struct ringbuf *ringbuf, void *data, size_t size);
size_t ringbuf_write_size(const struct ringbuf *ringbuf);
size_t ringbuf_contiguous_write_size(const struct ringbuf *ringbuf);
size_t ringbuf_read_size(const struct ringbuf *ringbuf);
size_t ringbuf_contiguous_read_size(const struct ringbuf *ringbuf);
size_t ringbuf_read_rollback(struct ringbuf *ringbuf, size_t size);

static inline void *
ringbuf_write_ptr(const struct ringbuf *ringbuf)
{
	return (uint8_t*)ringbuf->data + ringbuf->write_pos;
}

static inline size_t
ringbuf_next_write_pos(const struct ringbuf *ringbuf, size_t size)
{
	return (ringbuf->write_pos + size) % ringbuf->size;
}

static inline void
ringbuf_advance_write(struct ringbuf *ringbuf, size_t size)
{
	ringbuf->write_pos = ringbuf_next_write_pos(ringbuf, size);
}

static inline const void *
ringbuf_read_ptr(const struct ringbuf *ringbuf)
{
	return (uint8_t*)ringbuf->data + ringbuf->read_pos;
}

static inline size_t
ringbuf_next_read_pos(const struct ringbuf *ringbuf, size_t size)
{
	return (ringbuf->read_pos + size) % ringbuf->size;
}

static inline void
ringbuf_advance_read(struct ringbuf *ringbuf, size_t size)
{
	ringbuf->read_pos = ringbuf_next_read_pos(ringbuf, size);
}

int8_t ringbuf_ri8(struct ringbuf *ringbuf);
uint8_t ringbuf_ru8(struct ringbuf *ringbuf);
int16_t ringbuf_ri16(struct ringbuf *ringbuf);
uint16_t ringbuf_ru16(struct ringbuf *ringbuf);
int32_t ringbuf_ri32(struct ringbuf *ringbuf);
uint32_t ringbuf_ru32(struct ringbuf *ringbuf);
void ringbuf_rpad(struct ringbuf *ringbuf, uint32_t len);
void ringbuf_wi8(struct ringbuf *ringbuf, int8_t v);
void ringbuf_wu8(struct ringbuf *ringbuf, uint8_t v);
void ringbuf_wi16(struct ringbuf *ringbuf, int16_t v);
void ringbuf_wu16(struct ringbuf *ringbuf, uint16_t v);
void ringbuf_wi32(struct ringbuf *ringbuf, int32_t v);
void ringbuf_wu32(struct ringbuf *ringbuf, uint32_t v);
void ringbuf_wpad(struct ringbuf *ringbuf, uint32_t len);

#endif
