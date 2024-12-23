#include "bio.h"

static long bio_s_null_ctrl(struct bio *bio, enum bio_ctrl cmd, long arg1,
                            void *arg2)
{
	(void)arg1;
	(void)arg2;
	(void)bio;
	switch (cmd)
	{
		case BIO_CTRL_RESET:
			return 1;
		case BIO_CTRL_SEEK:
			return 0;
		case BIO_CTRL_TELL:
			return 0;
		case BIO_CTRL_FLUSH:
			return 1;
		case BIO_CTRL_EOF:
			return 1;
		default:
			return -1;
	}
}

static ssize_t bio_s_null_read(struct bio *bio, void *data, size_t size)
{
	(void)bio;
	(void)data;
	(void)size;
	return 0;
}

static ssize_t bio_s_null_write(struct bio *bio, const void *data, size_t size)
{
	(void)bio;
	(void)data;
	return size;
}

static const struct bio_method g_bio_s_null =
{
	.op_ctrl = bio_s_null_ctrl,
	.op_read = bio_s_null_read,
	.op_write = bio_s_null_write,
};

const struct bio_method *bio_s_null(void)
{
	return &g_bio_s_null;
}

static long bio_f_null_ctrl(struct bio *bio, enum bio_ctrl cmd, long arg1,
                            void *arg2)
{
	return bio_ctrl(bio->next, cmd, arg1, arg2);
}

static ssize_t bio_f_null_read(struct bio *bio, void *data, size_t size)
{
	return bio_read(bio->next, data, size);
}

static ssize_t bio_f_null_write(struct bio *bio, const void *data, size_t size)
{
	return bio_write(bio->next, data, size);
}

static const struct bio_method g_bio_f_null =
{
	.op_ctrl = bio_f_null_ctrl,
	.op_read = bio_f_null_read,
	.op_write = bio_f_null_write,
};

const struct bio_method *bio_f_null(void)
{
	return &g_bio_f_null;
}
