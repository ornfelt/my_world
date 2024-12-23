#include "bio.h"

#include <unistd.h>
#include <string.h>

struct bio_fd
{
	int fd;
	int flags;
};

struct bio *bio_new_fd(int fd, int flags)
{
	struct bio *bio = bio_new(bio_s_fd());
	if (!bio)
		return NULL;
	struct bio_fd *bio_fd = bio->ctx;
	bio_fd->fd = fd;
	bio_fd->flags = flags;
	return bio;
}

static void bio_fd_dtr(struct bio *bio)
{
	struct bio_fd *bio_fd = bio->ctx;
	if (!(bio_fd->flags & BIO_NOCLOSE))
		close(bio_fd->fd);
}

static long bio_fd_ctrl(struct bio *bio, enum bio_ctrl cmd, long arg1,
                        void *arg2)
{
	(void)arg2;
	struct bio_fd *bio_fd = bio->ctx;
	switch (cmd)
	{
		case BIO_CTRL_RESET:
			if (bio_fd->fd < 0)
				return -1;
			return lseek(bio_fd->fd, 0, SEEK_SET);
		case BIO_CTRL_SEEK:
			if (bio_fd->fd < 0)
				return -1;
			return lseek(bio_fd->fd, arg1, SEEK_SET);
		case BIO_CTRL_TELL:
			if (bio_fd->fd < 0)
				return -1;
			return lseek(bio_fd->fd, 0, SEEK_CUR);
		case BIO_CTRL_FLUSH:
			if (bio_fd->fd < 0)
				return -1;
			return 1;
		case BIO_CTRL_EOF:
			if (bio_fd->fd < 0)
				return -1;
			return 0; /* XXX */
		default:
			return -1;
	}
}

static ssize_t bio_fd_read(struct bio *bio, void *data, size_t size)
{
	struct bio_fd *bio_fd = bio->ctx;
	return read(bio_fd->fd, data, size);
}

static ssize_t bio_fd_write(struct bio *bio, const void *data, size_t size)
{
	struct bio_fd *bio_fd = bio->ctx;
	return write(bio_fd->fd, data, size);
}

static ssize_t bio_fd_gets(struct bio *bio, char *data, size_t size)
{
	if (!size)
		return 0;
	char *org = data;
	struct bio_fd *bio_fd = bio->ctx;
	while (size > 1)
	{
		char c;
		int ret = read(bio_fd->fd, &c, 1);
		if (ret == -1)
			return -1;
		if (!ret)
			break;
		*data = c;
		data++;
		size--;
		if (c == '\n')
			break;
	}
	*data = '\0';
	return strlen(org);
}

static const struct bio_method g_bio_s_fd =
{
	.ctx_size = sizeof(struct bio_fd),
	.op_dtr = bio_fd_dtr,
	.op_ctrl = bio_fd_ctrl,
	.op_read = bio_fd_read,
	.op_write = bio_fd_write,
	.op_gets = bio_fd_gets,
};

const struct bio_method *bio_s_fd(void)
{
	return &g_bio_s_fd;
}
