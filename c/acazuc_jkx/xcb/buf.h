#ifndef XCB_BUF_H
#define XCB_BUF_H

#include <stddef.h>
#include <stdint.h>

struct buf
{
	uint8_t *data;
	size_t capacity;
	size_t size;
	size_t pos;
};

void buf_init(struct buf *buf);
void buf_destroy(struct buf *buf);

void *buf_grow(struct buf *buf, size_t size);
int buf_reserve(struct buf *buf, size_t size);

static inline size_t buf_remaining(struct buf *buf)
{
	if (buf->pos >= buf->size)
		return 0;
	return buf->size - buf->pos;
}

int buf_send(struct buf *buf, int fd);
int buf_recv(struct buf *buf, int fd);

int buf_write(struct buf *buf, const void *data, size_t size);
int buf_wpad(struct buf *buf);

static inline int buf_wi8(struct buf *buf, int8_t v)
{
	return buf_write(buf, &v, 1);
}

static inline int buf_wu8(struct buf *buf, uint8_t v)
{
	return buf_write(buf, &v, 1);
}

static inline int buf_wi16(struct buf *buf, int16_t v)
{
	return buf_write(buf, &v, 2);
}

static inline int buf_wu16(struct buf *buf, uint16_t v)
{
	return buf_write(buf, &v, 2);
}

static inline int buf_wi32(struct buf *buf, int32_t v)
{
	return buf_write(buf, &v, 4);
}

static inline int buf_wu32(struct buf *buf, uint32_t v)
{
	return buf_write(buf, &v, 4);
}

static inline int buf_wstr(struct buf *buf, const char *v, size_t len)
{
	return buf_write(buf, v, len);
}

int buf_read(struct buf *buf, void *data, size_t size);
void buf_rrst(struct buf *buf);
const char *buf_rstr(struct buf *buf);

static inline void buf_rrb(struct buf *buf, size_t n)
{
	buf->pos -= n;
}

static inline int buf_ri8(struct buf *buf, int8_t *v)
{
	return buf_read(buf, v, 1);
}

static inline int buf_ru8(struct buf *buf, uint8_t *v)
{
	return buf_read(buf, v, 1);
}

static inline int buf_ri16(struct buf *buf, int16_t *v)
{
	return buf_read(buf, v, 2);
}

static inline int buf_ru16(struct buf *buf, uint16_t *v)
{
	return buf_read(buf, v, 2);
}

static inline int buf_ri32(struct buf *buf, int32_t *v)
{
	return buf_read(buf, v, 4);
}

static inline int buf_ru32(struct buf *buf, uint32_t *v)
{
	return buf_read(buf, v, 4);
}

#endif
