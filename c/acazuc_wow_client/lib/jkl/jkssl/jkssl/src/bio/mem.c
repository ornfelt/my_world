#include "bio.h"

#include <stdlib.h>
#include <string.h>

struct bio_mem
{
	struct buf_mem *mem;
	int flags;
	int ro;
	size_t ro_pos;
};

struct bio *bio_new_mem_buf(const void *buf, ssize_t len)
{
	struct bio *bio = bio_new(bio_s_mem());
	if (!bio)
		return NULL;
	struct bio_mem *bio_mem = bio->ctx;
	bio_mem->mem = malloc(sizeof(*bio_mem->mem));
	if (!bio_mem->mem)
	{
		bio_free(bio);
		return NULL;
	}
	bio_mem->mem->data = (void*)buf;
	bio_mem->mem->size = (len == -1 ? strlen(buf) : (size_t)len);
	bio_mem->ro = 1;
	bio_mem->flags = BIO_NOCLOSE;
	return bio;
}

static int bio_mem_ctr(struct bio *bio)
{
	struct bio_mem *bio_mem = bio->ctx;
	bio_mem->mem = calloc(sizeof(*bio_mem->mem), 1);
	if (!bio_mem->mem)
		return 0;
	bio_mem->flags = 0;
	bio_mem->ro = 0;
	return 1;
}

static void bio_mem_dtr(struct bio *bio)
{
	struct bio_mem *bio_mem = bio->ctx;
	if (!(bio_mem->flags & BIO_NOCLOSE)
	 && bio_mem->mem)
	{
		if (!bio_mem->ro)
			free(bio_mem->mem->data);
		free(bio_mem->mem);
	}
}

static long bio_mem_ctrl(struct bio *bio, enum bio_ctrl cmd, long arg1,
                         void *arg2)
{
	struct bio_mem *bio_mem = bio->ctx;
	switch (cmd)
	{
		case BIO_CTRL_RESET:
			if (bio_mem->ro)
			{
				bio_mem->ro_pos = 0;
				return 1;
			}
			return 1;
		case BIO_CTRL_SEEK:
			if (bio_mem->ro)
			{
				if (arg1 < 0)
					return -1;
				if ((size_t)arg1 >= bio_mem->mem->size)
					arg1 = bio_mem->mem->size;
				bio_mem->ro_pos = arg1;
				return 0;
			}
			return 0;
		case BIO_CTRL_TELL:
			if (bio_mem->ro)
				return bio_mem->ro_pos;
			return 0;
		case BIO_CTRL_FLUSH:
			return 1;
		case BIO_CTRL_EOF:
			if (!bio_mem->mem)
				return 1;
			if (bio_mem->ro)
				return bio_mem->ro_pos >= bio_mem->mem->size;
			return 0;
		case BIO_CTRL_MEM_PTR_GET:
			*((struct buf_mem**)arg2) = bio_mem->mem;
			return 1;
		default:
			return -1;
	}
}

static ssize_t bio_mem_read(struct bio *bio, void *data, size_t size)
{
	struct bio_mem *bio_mem = bio->ctx;
	if (!size)
		return 0;
	if (!bio_mem->mem)
		return 0;
	if (bio_mem->ro)
	{
		if (bio_mem->ro_pos >= bio_mem->mem->size)
			return 0;
		if (size > bio_mem->mem->size - bio_mem->ro_pos)
			size = bio_mem->mem->size - bio_mem->ro_pos;
		memcpy(data, &((uint8_t*)bio_mem->mem->data)[bio_mem->ro_pos], size);
		bio_mem->ro_pos += size;
		return size;
	}
	size_t n = size;
	if (n > bio_mem->mem->size)
		n = bio_mem->mem->size;
	memcpy(data, bio_mem->mem->data, n);
	memcpy(bio_mem->mem->data, &((uint8_t*)bio_mem->mem->data)[n],
	       bio_mem->mem->size - n);
	bio_mem->mem->size -= n;
	void *newdata = realloc(bio_mem->mem->data, bio_mem->mem->size);
	if (newdata) /* best effort... */
		bio_mem->mem->data = newdata;
	return n;
}

static ssize_t bio_mem_write(struct bio *bio, const void *data, size_t size)
{
	struct bio_mem *bio_mem = bio->ctx;
	if (!size)
		return 0;
	if (!bio_mem->mem)
		return 0;
	if (bio_mem->ro)
		return 0;
	void *newdata = realloc(bio_mem->mem->data, bio_mem->mem->size + size);
	if (!newdata)
		return 0;
	memcpy(&((uint8_t*)newdata)[bio_mem->mem->size], data, size);
	bio_mem->mem->size += size;
	bio_mem->mem->data = newdata;
	return size;
}

static ssize_t bio_mem_gets(struct bio *bio, char *data, size_t size)
{
	if (!size)
		return 0;
	char *org = data;
	struct bio_mem *bio_mem = bio->ctx;
	size_t n = 0;
	while (size > 1)
	{
		char c;
		if (bio_mem->ro)
		{
			if (bio_mem->ro_pos >= bio_mem->mem->size)
				break;
			c = ((char*)bio_mem->mem->data)[bio_mem->ro_pos++];
		}
		else
		{
			if (n >= bio_mem->mem->size)
				break;
			c = ((char*)bio_mem->mem->data)[n];
		}
		*data = c;
		c++;
		size--;
		if (c == '\n')
			break;
	}
	if (!bio_mem->ro)
	{
		memcpy(bio_mem->mem->data, &((uint8_t*)bio_mem->mem->data)[n],
		       bio_mem->mem->size - n);
		bio_mem->mem->size -= n;
		void *newdata = realloc(bio_mem->mem->data, bio_mem->mem->size);
		if (newdata) /* best effort... */
			bio_mem->mem->data = newdata;
	}
	*data = '\0';
	return strlen(org);
}

static const struct bio_method g_bio_s_mem =
{
	.ctx_size = sizeof(struct bio_mem),
	.op_ctr = bio_mem_ctr,
	.op_dtr = bio_mem_dtr,
	.op_ctrl = bio_mem_ctrl,
	.op_read = bio_mem_read,
	.op_write = bio_mem_write,
	.op_gets = bio_mem_gets,
};

const struct bio_method *bio_s_mem(void)
{
	return &g_bio_s_mem;
}
