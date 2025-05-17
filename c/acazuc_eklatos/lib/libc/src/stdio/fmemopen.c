#include "_stdio.h"

#include <stdio.h>

struct memopen_data
{
	FILE *fp;
	uint8_t *data;
	size_t wr_pos;
	size_t size;
	size_t pos;
	int own_buf;
	int has_written;
};

static ssize_t
read_fn(void *cookie, char *buf, size_t size)
{
	struct memopen_data *data = cookie;
	size_t avail = data->wr_pos - data->pos;

	if (size > avail)
		size = avail;
	memcpy(buf, &data->data[data->pos], size);
	data->pos += size;
	return size;
}

static ssize_t
write_fn(void *cookie, const char *buf, size_t size)
{
	struct memopen_data *data = cookie;
	size_t avail = data->size - data->pos;
	size_t org = size;

	if (data->pos == data->size)
		return size;
	if (size > avail)
		size = avail;
	memcpy(&data->data[data->pos], buf, size);
	if (size)
		data->has_written = 1;
	data->pos += size;
	if (data->pos > data->wr_pos)
		data->wr_pos = data->pos;
	return org;
}

static int
seek_fn(void *cookie, off_t off, int whence)
{
	struct memopen_data *data = cookie;

	switch (whence)
	{
		case SEEK_CUR:
			if (off < 0)
			{
				if (off == INT64_MIN || (size_t)-off < data->pos)
				{
					errno = EINVAL;
					return -1;
				}
			}
			else
			{
				if ((size_t)off > data->size - data->pos)
				{
					errno = EINVAL;
					return -1;
				}
			}
			data->pos += off;
			break;
		case SEEK_SET:
			if (off < 0 || (size_t)off > data->pos)
			{
				errno = EINVAL;
				return -1;
			}
			data->pos = off;
			break;
		case SEEK_END:
			if (off > 0 || off == INT64_MIN
			 || (size_t)-off < -data->size)
			{
				errno = EINVAL;
				return -1;
			}
			data->pos = data->size + off;
			break;
		default:
			return -1;
	}
	return data->pos;
}

static int
close_fn(void *cookie)
{
	struct memopen_data *data = cookie;

	if (data->own_buf)
		free(data->data);
	free(data);
	return 0;
}

static int
flush_fn(FILE *fp)
{
	struct memopen_data *data = fp->cookie;

	switch (data->fp->mode & 3)
	{
		case O_WRONLY:
		case O_RDWR:
			if (data->has_written)
			{
				data->has_written = 0;
				if (data->pos < data->size)
					data->data[data->pos] = '\0';
			}
			break;
	}
	return 0;
}

FILE *
fmemopen(void *buf, size_t size, const char *mode)
{
	struct memopen_data *data;
	FILE *fp;
	int flags;

	fp = mkfp();
	if (!fp)
		return NULL;
	if (!parse_flags(mode, &flags))
	{
		free(fp);
		return NULL;
	}
	if (flags & O_CLOEXEC)
	{
		free(fp);
		return NULL;
	}
	fp->mode = flags;
	data = malloc(sizeof(*data));
	if (!data)
	{
		free(fp);
		return NULL;
	}
	data->data = buf;
	data->size = size;
	data->own_buf = !buf;
	data->has_written = 0;
	data->fp = fp;
	if (!buf)
	{
		data->data = malloc(size);
		if (!data->data)
		{
			free(fp);
			free(data);
			return NULL;
		}
	}
	fp->cookie = data;
	switch (fp->mode & 3)
	{
		case O_RDONLY:
			fp->io_funcs.read = read_fn;
			break;
		case O_WRONLY:
			fp->io_funcs.write = write_fn;
			break;
		case O_RDWR:
			fp->io_funcs.read = read_fn;
			fp->io_funcs.write = write_fn;
			break;
	}
	fp->io_funcs.seek = seek_fn;
	fp->io_funcs.close = close_fn;
	fp->flush = flush_fn;
	if (flags & O_APPEND)
	{
		if (data->own_buf)
		{
			data->pos = 0;
		}
		else
		{
			void *end = memchr(buf, '\0', size);
			if (end)
				data->pos = (uint8_t*)end - (uint8_t*)buf;
			else
				data->pos = 0;
		}
	}
	else
	{
		data->pos = 0;
	}
	if ((fp->mode & 3) == O_RDONLY)
		data->wr_pos = size;
	else
		data->wr_pos = data->pos;
	if ((flags & (3 | O_CREAT | O_TRUNC)) == (O_RDWR | O_CREAT | O_TRUNC))
		fwrite("", 1, 1, fp);
	return fp;
}
