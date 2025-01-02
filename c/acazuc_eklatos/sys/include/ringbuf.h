#ifndef RINGBUF_H
#define RINGBUF_H

#include <types.h>

struct uio;

struct ringbuf
{
	void *data;
	size_t size;
	size_t read_pos;
	size_t write_pos;
};

int ringbuf_init(struct ringbuf *ringbuf, size_t size);
void ringbuf_destroy(struct ringbuf *ringbuf);
size_t ringbuf_write(struct ringbuf *ringbuf, const void *data, size_t size);
size_t ringbuf_read(struct ringbuf *ringbuf, void *data, size_t size);
size_t ringbuf_peek(struct ringbuf *ringbuf, void *data, size_t size);
ssize_t ringbuf_writeuio(struct ringbuf *ringbuf, struct uio *uio, size_t size);
ssize_t ringbuf_readuio(struct ringbuf *ringbuf, struct uio *uio, size_t size);
ssize_t ringbuf_peekuio(struct ringbuf *ringbuf, struct uio *uio, size_t size);
size_t ringbuf_write_size(const struct ringbuf *ringbuf);
size_t ringbuf_contiguous_write_size(const struct ringbuf *ringbuf);
size_t ringbuf_read_size(const struct ringbuf *ringbuf);
size_t ringbuf_contiguous_read_size(const struct ringbuf *ringbuf);

static inline void *ringbuf_write_ptr(const struct ringbuf *ringbuf)
{
	return (uint8_t*)ringbuf->data + ringbuf->write_pos;
}

static inline void ringbuf_advance_write(struct ringbuf *ringbuf, size_t size)
{
	ringbuf->write_pos = (ringbuf->write_pos + size) % ringbuf->size;
}

static inline const void *ringbuf_read_ptr(const struct ringbuf *ringbuf)
{
	return (uint8_t*)ringbuf->data + ringbuf->read_pos;
}

static inline void ringbuf_advance_read(struct ringbuf *ringbuf, size_t size)
{
	ringbuf->read_pos = (ringbuf->read_pos + size) % ringbuf->size;
}

#endif
