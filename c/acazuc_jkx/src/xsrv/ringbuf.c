#include "ringbuf.h"

#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>

int
ringbuf_init(struct ringbuf *ringbuf, size_t size)
{
	if (size)
	{
		ringbuf->data = malloc(size);
		if (ringbuf->data == NULL)
			return 1;
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

void
ringbuf_destroy(struct ringbuf *ringbuf)
{
	free(ringbuf->data);
}

int
ringbuf_resize(struct ringbuf *ringbuf, size_t size)
{
	uint8_t *newdata;

	if (ringbuf->write_pos == ringbuf->read_pos)
	{
		/* reset the buffer because no data */
		newdata = realloc(ringbuf->data, size);
		if (!newdata)
			return 1;
		ringbuf->data = newdata;
		ringbuf->size = size;
		ringbuf->write_pos = 0;
		ringbuf->read_pos = 0;
		return 0;
	}
	if (ringbuf->write_pos > ringbuf->read_pos)
	{
		/* no current overflow */
		size_t current_len;
		current_len = ringbuf->write_pos - ringbuf->read_pos;
		if (size <= current_len)
			return 1;
		if (ringbuf->read_pos >= size)
		{
			/* all the data will overflow, move everything to the beginning */
			memcpy(ringbuf->data, (uint8_t*)ringbuf->data + ringbuf->read_pos,
			       current_len);
			ringbuf->read_pos = 0;
			ringbuf->write_pos = current_len;
		}
		else if (ringbuf->write_pos > size)
		{
			/* only copy the overflowing data */
			size_t overflow_size;
			overflow_size = ringbuf->write_pos - size;
			memcpy(ringbuf->data, (uint8_t*)ringbuf->data + size, overflow_size);
			ringbuf->write_pos = overflow_size;
		}
		newdata = realloc(ringbuf->data, size);
		if (!newdata)
			return 1;
		ringbuf->data = newdata;
		ringbuf->size = size;
		return 0;
	}
	/* data are overflowing */
	size_t current_len;
	current_len = ringbuf->size - 1 - ringbuf->read_pos + ringbuf->write_pos;
	if (size <= current_len)
		return 1;
	if (size >= ringbuf->read_pos + current_len)
	{
		/* all the data can be contiguous by copying the current overflowing part */
		newdata = realloc(ringbuf->data, size);
		if (!newdata)
			return 1;
		memcpy(newdata + ringbuf->size, newdata, ringbuf->write_pos);
		ringbuf->write_pos = (ringbuf->read_pos + current_len) % size;
		ringbuf->data = newdata;
		ringbuf->size = size;
		return 0;
	}
	/* data are overflowing and will be overflowing, might be worse optimizing later */
	newdata = malloc(size);
	if (!newdata)
		return 1;
	memcpy(newdata, (uint8_t*)ringbuf->data + ringbuf->read_pos,
	       ringbuf->size - ringbuf->read_pos - 1);
	memcpy(newdata + ringbuf->read_pos, ringbuf->data, ringbuf->write_pos);
	free(ringbuf->data);
	ringbuf->data = newdata;
	ringbuf->size = size;
	ringbuf->read_pos = 0;
	ringbuf->write_pos = current_len;
	return 0;
}

size_t
ringbuf_write(struct ringbuf *ringbuf, const void *data, size_t size)
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
			memcpy(ringbuf_write_ptr(ringbuf), (uint8_t*)data + wr, available);
		else
			memset(ringbuf_write_ptr(ringbuf), 0, available);
		wr += available;
		size -= available;
		ringbuf_advance_write(ringbuf, available);
	}
	return wr;
}

size_t
ringbuf_read(struct ringbuf *ringbuf, void *data, size_t size)
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
			memcpy((uint8_t*)data + rd, ringbuf_read_ptr(ringbuf), available);
		rd += available;
		size -= available;
		ringbuf_advance_read(ringbuf, available);
	}
	return rd;
}

size_t
ringbuf_peek(struct ringbuf *ringbuf, void *data, size_t size)
{
	size_t current = ringbuf->read_pos;
	size_t ret = ringbuf_read(ringbuf, data, size);
	ringbuf->read_pos = current;
	return ret;
}

size_t
ringbuf_write_size(const struct ringbuf *ringbuf)
{
	if (ringbuf->write_pos < ringbuf->read_pos)
		return ringbuf->read_pos - ringbuf->write_pos - 1;
	return ringbuf->size - 1 - ringbuf->write_pos + ringbuf->read_pos;
}

size_t
ringbuf_contiguous_write_size(const struct ringbuf *ringbuf)
{
	if (ringbuf->write_pos < ringbuf->read_pos)
		return ringbuf->read_pos - ringbuf->write_pos - 1;
	if (!ringbuf->read_pos)
		return ringbuf->size - ringbuf->write_pos - 1;
	return ringbuf->size - ringbuf->write_pos;
}

size_t
ringbuf_read_size(const struct ringbuf *ringbuf)
{
	if (ringbuf->read_pos <= ringbuf->write_pos)
		return ringbuf->write_pos - ringbuf->read_pos;
	return ringbuf->size - ringbuf->read_pos + ringbuf->write_pos;
}

size_t
ringbuf_contiguous_read_size(const struct ringbuf *ringbuf)
{
	if (ringbuf->read_pos <= ringbuf->write_pos)
		return ringbuf->write_pos - ringbuf->read_pos;
	return ringbuf->size - ringbuf->read_pos;
}

int8_t
ringbuf_ri8(struct ringbuf *ringbuf)
{
	return (int8_t)ringbuf_ru8(ringbuf);
}

uint8_t
ringbuf_ru8(struct ringbuf *ringbuf)
{
	uint8_t v;
	ringbuf_read(ringbuf, &v, 1);
	return v;
}

int16_t
ringbuf_ri16(struct ringbuf *ringbuf)
{
	return (int16_t)ringbuf_ru16(ringbuf);
}

uint16_t
ringbuf_ru16(struct ringbuf *ringbuf)
{
	uint16_t v;
	ringbuf_read(ringbuf, &v, 2);
	if (ringbuf->bitswap)
		v = ntohs(v);
	return v;
}

int32_t
ringbuf_ri32(struct ringbuf *ringbuf)
{
	return (int32_t)ringbuf_ru32(ringbuf);
}

uint32_t
ringbuf_ru32(struct ringbuf *ringbuf)
{
	uint32_t v;
	ringbuf_read(ringbuf, &v, 4);
	if (ringbuf->bitswap)
		v = ntohl(v);
	return v;
}

void
ringbuf_wi8(struct ringbuf *ringbuf, int8_t v)
{
	ringbuf_wu8(ringbuf, (uint8_t)v);
}

void
ringbuf_wu8(struct ringbuf *ringbuf, uint8_t v)
{
	ringbuf_write(ringbuf, &v, 1);
}

void
ringbuf_wi16(struct ringbuf *ringbuf, int16_t v)
{
	ringbuf_wu16(ringbuf, (uint16_t)v);
}

void
ringbuf_wu16(struct ringbuf *ringbuf, uint16_t v)
{
	if (ringbuf->bitswap)
		v = ntohs(v);
	ringbuf_write(ringbuf, &v, 2);
}

void
ringbuf_wi32(struct ringbuf *ringbuf, int32_t v)
{
	ringbuf_wu32(ringbuf, (uint32_t)v);
}

void
ringbuf_wu32(struct ringbuf *ringbuf, uint32_t v)
{
	if (ringbuf->bitswap)
		v = ntohl(v);
	ringbuf_write(ringbuf, &v, 4);
}

void
ringbuf_rpad(struct ringbuf *ringbuf, uint32_t len)
{
	size_t n = len & 3;
	if (!n)
		return;
	ringbuf_advance_read(ringbuf, 4 - n);
}

void
ringbuf_wpad(struct ringbuf *ringbuf, uint32_t len)
{
	size_t n = len & 3;
	if (!n)
		return;
	ringbuf_write(ringbuf, NULL, 4 - n);
}
