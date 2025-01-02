#include "buf.h"

#include <sys/socket.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

void buf_init(struct buf *buf)
{
	buf->data = NULL;
	buf->capacity = 0;
	buf->size = 0;
	buf->pos = 0;
}

void buf_destroy(struct buf *buf)
{
	free(buf->data);
}

void *buf_grow(struct buf *buf, size_t size)
{
	if (!buf_reserve(buf, buf->size + size))
		return NULL;
	void *ptr = &buf->data[buf->size];
	buf->size += size;
	return ptr;
}

int buf_reserve(struct buf *buf, size_t size)
{
	if (size <= buf->capacity)
		return 1;
	uint8_t *d = realloc(buf->data, size);
	if (!d)
		return 0;
	buf->data = d;
	buf->capacity = size;
	return 1;
}

int buf_send(struct buf *buf, int fd)
{
	if (buf->pos >= buf->size)
		return 0;
again:;
	int ret = send(fd, &buf->data[buf->pos], buf->size - buf->pos, 0);
	if (ret < 0)
	{
		if (ret == EINTR)
			goto again;
		return ret;
	}
	buf->pos += ret;
	if (buf->pos == buf->size)
	{
		buf->pos = 0;
		buf->size = 0;
	}
	return ret;
}

int buf_recv(struct buf *buf, int fd)
{
	buf_rrst(buf);
	if (buf->size >= buf->capacity)
	{
		errno = EAGAIN;
		return -1;
	}
again:;
	int ret = recv(fd, &buf->data[buf->size], buf->capacity - buf->size, 0);
	if (ret <= 0)
	{
		if (ret == EINTR)
			goto again;
		return ret;
	}
	buf->size += ret;
	return ret;
}

int buf_write(struct buf *buf, const void *data, size_t size)
{
	if (!buf_reserve(buf, buf->size + size))
		return 0;
	if (data)
		memcpy(&buf->data[buf->size], data, size);
	else
		memset(&buf->data[buf->size], 0, size);
	buf->size += size;
	return 1;
}

int buf_wpad(struct buf *buf)
{
	static const char z[4] = {0};
	size_t m = buf->size % 4;
	if (!m)
		return 1;
	return buf_write(buf, z, 4 - m);
}

void buf_rrst(struct buf *buf)
{
	memmove(buf->data, &buf->data[buf->pos], buf->size - buf->pos);
	buf->size -= buf->pos;
	buf->pos = 0;
}

int buf_read(struct buf *buf, void *data, size_t size)
{
	if (buf->pos + size > buf->size)
		return 0;
	if (data)
		memcpy(data, &buf->data[buf->pos], size);
	buf->pos += size;
	return 1;
}

const char *buf_rstr(struct buf *buf)
{
	const char *base = (const char*)buf->data;
	while (buf->pos < buf->size)
	{
		buf->pos++;
		if (!buf->data[buf->pos - 1])
			return base;
	}
	return NULL;
}
