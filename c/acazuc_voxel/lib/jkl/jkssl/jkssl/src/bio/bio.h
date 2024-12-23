#ifndef JKSSL_BIO_BIO_H
#define JKSSL_BIO_BIO_H

#include "refcount.h"

#include <jkssl/bio.h>

#include <sys/types.h>
#include <sys/queue.h>

struct bio_method
{
	size_t ctx_size;
	int (*op_ctr)(struct bio *bio);
	void (*op_dtr)(struct bio *bio);
	long (*op_ctrl)(struct bio *bio, enum bio_ctrl cmd, long arg1, void *arg2);
	ssize_t (*op_read)(struct bio *bio, void *data, size_t size);
	ssize_t (*op_write)(struct bio *bio, const void *data, size_t size);
	ssize_t (*op_gets)(struct bio *bio, char *buf, size_t size);
};

struct bio
{
	const struct bio_method *method;
	void *ctx;
	struct bio *next;
	TAILQ_HEAD(, bio) prev_head;
	TAILQ_ENTRY(bio) prev_chain;
	refcount_t refcount;
};

#endif
