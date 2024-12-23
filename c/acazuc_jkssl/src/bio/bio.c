#include "bio.h"

#include <stdlib.h>
#include <string.h>

struct bio *bio_new(const struct bio_method *method)
{
	if (!method)
		return NULL;
	struct bio *bio = calloc(sizeof(*bio), 1);
	if (!bio)
		return NULL;
	if (method->ctx_size)
	{
		bio->ctx = calloc(method->ctx_size, 1);
		if (!bio->ctx)
		{
			free(bio);
			return NULL;
		}
	}
	else
	{
		bio->ctx = NULL;
	}
	bio->method = method;
	refcount_init(&bio->refcount, 1);
	TAILQ_INIT(&bio->prev_head);
	if (method->op_ctr && !method->op_ctr(bio))
	{
		free(bio->ctx);
		free(bio);
		return NULL;
	}
	return bio;
}

int bio_free(struct bio *bio)
{
	if (!bio)
		return 1;
	if (refcount_dec(&bio->refcount))
		return 1;
	bio_flush(bio);
	if (bio->method->op_dtr)
		bio->method->op_dtr(bio);
	if (bio->next)
		bio_free(bio->next);
	free(bio->ctx);
	free(bio);
	return 1;
}

void bio_vfree(struct bio *bio)
{
	(void)bio_free(bio);
}

void bio_free_all(struct bio *bio)
{
	if (!bio)
		return;
	bio_free_all(bio->next);
	bio_free(bio);
}

int bio_up_ref(struct bio *bio)
{
	return refcount_inc(&bio->refcount);
}

struct bio *bio_push(struct bio *bio, struct bio *next)
{
	if (!bio)
		return next;
	if (!next)
		return bio;
	if (bio->next)
		TAILQ_REMOVE(&bio->next->prev_head, bio, prev_chain);
	TAILQ_INSERT_TAIL(&next->prev_head, bio, prev_chain);
	bio->next = next;
	bio_up_ref(next);
	return bio;
}

struct bio *bio_pop(struct bio *bio)
{
	if (!bio)
		return NULL;
	struct bio *next = bio->next;
	struct bio *next_next = next ? next->next : NULL;
	struct bio *it;
	TAILQ_FOREACH(it, &bio->prev_head, prev_chain)
	{
		it->next = next_next;
		bio_free(bio);
	}
	return next;
}

void bio_set_next(struct bio *bio, struct bio *next)
{
	if (!bio)
		return;
	if (bio->next)
	{
		TAILQ_REMOVE(&bio->next->prev_head, bio, prev_chain);
		bio_free(bio->next);
	}
	if (next)
	{
		bio_up_ref(next);
		TAILQ_INSERT_TAIL(&next->prev_head, bio, prev_chain);
	}
	bio->next = next;
}

long bio_ctrl(struct bio *bio, enum bio_ctrl cmd, long arg1, void *arg2)
{
	if (!bio)
		return -1;
	if (!bio->method->op_ctrl)
		return -2;
	return bio->method->op_ctrl(bio, cmd, arg1, arg2);
}

ssize_t bio_read(struct bio *bio, void *data, size_t size)
{
	if (!bio)
		return -1;
	if (!bio->method->op_read)
		return -2;
	return bio->method->op_read(bio, data, size);
}

ssize_t bio_write(struct bio *bio, const void *data, size_t size)
{
	if (!bio)
		return -1;
	if (!bio->method->op_write)
		return -2;
	return bio->method->op_write(bio, data, size);
}

ssize_t bio_gets(struct bio *bio, char *data, size_t size)
{
	if (!bio)
		return -1;
	if (!bio->method->op_gets)
		return -2;
	return bio->method->op_gets(bio, data, size);
}

ssize_t bio_puts(struct bio *bio, const char *line)
{
	ssize_t ret = bio_write(bio, line, strlen(line));
	if (ret < 0)
		return ret;
	ssize_t ret2 = bio_write(bio, "\n", 1);
	if (ret2 < 0)
		return ret2;
	return ret + ret2;
}

int bio_printf(struct bio *bio, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int ret = bio_vprintf(bio, fmt, ap);
	va_end(ap);
	return ret;
}

int bio_vprintf(struct bio *bio, const char *fmt, va_list ap)
{
	char *buf;
	va_list tmp;
	va_copy(tmp, ap);
	int len = vsnprintf(NULL, 0, fmt, tmp);
	va_end(tmp);
	if (len < 0)
		return -1;
	buf = malloc(len + 1);
	vsnprintf(buf, len + 1, fmt, ap);
	int ret = bio_write(bio, buf, len);
	free(buf);
	return ret;
}
