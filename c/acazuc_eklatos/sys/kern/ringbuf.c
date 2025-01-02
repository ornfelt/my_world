#include <ringbuf.h>
#include <errno.h>
#include <std.h>
#include <uio.h>
#include <mem.h>

int ringbuf_init(struct ringbuf *ringbuf, size_t size)
{
	if (size)
	{
		size_t alloc_size = (size + PAGE_MASK) & ~PAGE_MASK;
		alloc_size -= alloc_size % PAGE_SIZE;
		ringbuf->data = vmalloc(alloc_size);
		if (ringbuf->data == NULL)
			return -ENOMEM;
	}
	else
	{
		ringbuf->data = NULL;
	}
	ringbuf->size = size;
	ringbuf->read_pos = 0;
	ringbuf->write_pos = 0;
	return 0;
}

void ringbuf_destroy(struct ringbuf *ringbuf)
{
	if (!ringbuf->data)
		return;
	size_t alloc_size = (ringbuf->size + PAGE_MASK) & ~PAGE_MASK;
	vfree(ringbuf->data, alloc_size);
}

size_t ringbuf_write(struct ringbuf *ringbuf, const void *data, size_t size)
{
	size_t wr = 0;
	while (size > 0)
	{
		size_t available = ringbuf_contiguous_write_size(ringbuf);
		if (!available)
			break;
		if (size < available)
			available = size;
		memcpy(ringbuf_write_ptr(ringbuf), (uint8_t*)data + wr, available);
		wr += available;
		size -= available;
		ringbuf_advance_write(ringbuf, available);
	}
	return wr;
}

size_t ringbuf_read(struct ringbuf *ringbuf, void *data, size_t size)
{
	size_t rd = 0;
	while (size > 0)
	{
		size_t available = ringbuf_contiguous_read_size(ringbuf);
		if (!available)
			break;
		if (size < available)
			available = size;
		memcpy((uint8_t*)data + rd, ringbuf_read_ptr(ringbuf), available);
		rd += available;
		size -= available;
		ringbuf_advance_read(ringbuf, available);
	}
	return rd;
}

size_t ringbuf_peek(struct ringbuf *ringbuf, void *data, size_t size)
{
	size_t current = ringbuf->read_pos;
	size_t ret = ringbuf_read(ringbuf, data, size);
	ringbuf->read_pos = current;
	return ret;
}

ssize_t ringbuf_writeuio(struct ringbuf *ringbuf, struct uio *uio, size_t size)
{
	size_t wr = 0;
	while (size > 0)
	{
		size_t available = ringbuf_contiguous_write_size(ringbuf);
		if (!available)
			break;
		if (size < available)
			available = size;
		ssize_t ret = uio_copyout(ringbuf_write_ptr(ringbuf), uio, available);
		if (ret < 0)
			return ret;
		wr += ret;
		size -= ret;
		ringbuf_advance_write(ringbuf, ret);
	}
	return wr;
}

ssize_t ringbuf_readuio(struct ringbuf *ringbuf, struct uio *uio, size_t size)
{
	size_t rd = 0;
	while (size > 0)
	{
		size_t available = ringbuf_contiguous_read_size(ringbuf);
		if (!available)
			break;
		if (size < available)
			available = size;
		ssize_t ret = uio_copyin(uio, ringbuf_read_ptr(ringbuf), available);
		if (ret < 0)
			return ret;
		rd += ret;
		size -= ret;
		ringbuf_advance_read(ringbuf, ret);
	}
	return rd;
}

ssize_t ringbuf_peekuio(struct ringbuf *ringbuf, struct uio *uio, size_t size)
{
	size_t current = ringbuf->read_pos;
	ssize_t ret = ringbuf_readuio(ringbuf, uio, size);
	ringbuf->read_pos = current;
	return ret;
}

size_t ringbuf_write_size(const struct ringbuf *ringbuf)
{
	if (ringbuf->write_pos < ringbuf->read_pos)
		return ringbuf->read_pos - ringbuf->write_pos - 1;
	return ringbuf->size - 1 - ringbuf->write_pos + ringbuf->read_pos;
}

size_t ringbuf_contiguous_write_size(const struct ringbuf *ringbuf)
{
	if (ringbuf->write_pos < ringbuf->read_pos)
		return ringbuf->read_pos - ringbuf->write_pos - 1;
	if (!ringbuf->read_pos)
		return ringbuf->size - ringbuf->write_pos - 1;
	return ringbuf->size - ringbuf->write_pos;
}

size_t ringbuf_read_size(const struct ringbuf *ringbuf)
{
	if (ringbuf->read_pos <= ringbuf->write_pos)
		return ringbuf->write_pos - ringbuf->read_pos;
	return ringbuf->size - ringbuf->read_pos + ringbuf->write_pos;
}

size_t ringbuf_contiguous_read_size(const struct ringbuf *ringbuf)
{
	if (ringbuf->read_pos <= ringbuf->write_pos)
		return ringbuf->write_pos - ringbuf->read_pos;
	return ringbuf->size - ringbuf->read_pos;
}
