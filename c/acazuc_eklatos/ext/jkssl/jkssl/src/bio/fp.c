#include "bio/bio.h"

#include <string.h>

struct bio_file
{
	FILE *fp;
	int flags;
};

struct bio *bio_new_file(const char *filename, const char *mode)
{
	FILE *fp = fopen(filename, mode);
	if (!fp)
		return NULL;
	return bio_new_fp(fp, 0);
}

struct bio *bio_new_fp(FILE *fp, int flags)
{
	struct bio *bio = bio_new(bio_s_file());
	if (!bio)
		return NULL;
	struct bio_file *bio_file = bio->ctx;
	bio_file->fp = fp;
	bio_file->flags = flags;
	return bio;
}

static void bio_file_dtr(struct bio *bio)
{
	struct bio_file *bio_file = bio->ctx;
	if (!(bio_file->flags & BIO_NOCLOSE))
	{
		if (bio_file->fp)
			fclose(bio_file->fp);
	}
}

static long bio_file_ctrl(struct bio *bio, enum bio_ctrl cmd, long arg1,
                          void *arg2)
{
	(void)arg2;
	struct bio_file *bio_file = bio->ctx;
	switch (cmd)
	{
		case BIO_CTRL_RESET:
			if (!bio_file->fp)
				return 0;
			rewind(bio_file->fp);
			return 1;
		case BIO_CTRL_SEEK:
			if (!bio_file->fp)
				return -1;
			if (fseek(bio_file->fp, SEEK_SET, arg1) == -1)
				return -1;
			return 0;
		case BIO_CTRL_TELL:
			if (!bio_file->fp)
				return -1;
			return ftell(bio_file->fp);
		case BIO_CTRL_FLUSH:
			if (!bio_file->fp)
				return -1;
			if (fflush(bio_file->fp) == EOF)
				return -1;
			return 1;
		case BIO_CTRL_EOF:
			if (!bio_file->fp)
				return 1;
			return feof(bio_file->fp);
		default:
			return -1;
	}
}

static ssize_t bio_file_read(struct bio *bio, void *data, size_t size)
{
	struct bio_file *bio_file = bio->ctx;
	size_t rd = fread(data, 1, size, bio_file->fp);
	if (ferror(bio_file->fp))
		return -1;
	return rd;
}

static ssize_t bio_file_write(struct bio *bio, const void *data, size_t size)
{
	struct bio_file *bio_file = bio->ctx;
	size_t wr = fwrite(data, 1, size, bio_file->fp);
	if (ferror(bio_file->fp))
		return -1;
	return wr;
}

static ssize_t bio_file_gets(struct bio *bio, char *data, size_t size)
{
	if (!size)
		return 0;
	char *org = data;
	struct bio_file *bio_file = bio->ctx;
	int c = '\0';
	while (size > 1 && (c = fgetc(bio_file->fp)) != EOF)
	{
		*data = c;
		data++;
		size--;
		if (c == '\n')
			break;
	}
	*data = '\0';
	if (c == EOF && ferror(bio_file->fp))
		return -1;
	return strlen(org);
}

const struct bio_method *bio_s_file(void)
{
	static const struct bio_method method =
	{
		.ctx_size = sizeof(struct bio_file),
		.op_dtr = bio_file_dtr,
		.op_ctrl = bio_file_ctrl,
		.op_read = bio_file_read,
		.op_write = bio_file_write,
		.op_gets = bio_file_gets,
	};
	return &method;
}
