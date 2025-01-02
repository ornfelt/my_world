#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <zlib.h>

struct gzFile_s
{
	z_stream stream;
	FILE *fp;
	uint8_t *buf;
	size_t buf_size;
	size_t buf_len;
	int direction;
};

static gzFile gzfopen(FILE *fp)
{
	gzFile file = calloc(1, sizeof(*file));
	if (!file)
		return NULL;
	file->direction = -1;
	file->fp = fp;
	return file;
}

gzFile gzopen(const char *path, const char *mode)
{
	FILE *fp = fopen(path, mode);
	if (!fp)
		return NULL;
	gzFile file = gzfopen(fp);
	if (!file)
		fclose(fp);
	return file;
}

gzFile gzdopen(int fd, const char *mode)
{
	FILE *fp = fdopen(fd, mode);
	if (!fp)
		return NULL;
	gzFile file = gzfopen(fp);
	if (!file)
		fclose(fp);
	return file;
}

int gzbuffer(gzFile file, unsigned size)
{
	if (file->buf)
		return -1;
	void *newbuf = realloc(file->buf, size);
	if (!newbuf)
		return -1;
	file->buf_size = size;
	file->buf = newbuf;
	return 0;
}

int gzread(gzFile file, void *buf, unsigned len)
{
	if (!file->buf)
	{
		if (!file->buf_size)
			file->buf_size = 1024 * 8;
		file->buf = malloc(file->buf_size);
		if (!file->buf)
			return -1;
	}
	if (len > INT_MAX)
		return -1;
	switch (file->direction)
	{
		case -1:
			if (inflateInit(&file->stream) != Z_OK)
				return -1;
			file->direction = 0;
			break;
		case 0:
			break;
		default:
			return -1;
	}
	uint8_t *ptr = buf;
	int total = 0;
	while (len && (file->buf_len || !feof(file->fp)))
	{
		size_t rd = fread(&file->buf[file->buf_len], 1,
		                  file->buf_size - file->buf_len, file->fp);
		if (ferror(file->fp))
			return -1;
		file->buf_len += rd;
		file->stream.avail_in = file->buf_len;
		file->stream.next_in = file->buf;
		file->stream.avail_out = len;
		file->stream.next_out = ptr;
		int ret = inflate(&file->stream, Z_NO_FLUSH);
		switch (ret)
		{
			case Z_OK:
			case Z_STREAM_END:
				break;
			default:
				return -1;
		}
		memmove(file->buf,
		        &file->buf[file->buf_len - file->stream.avail_in],
		        file->stream.avail_in);
		file->buf_len = file->stream.avail_in;
		size_t wr = len - file->stream.avail_out;
		len -= wr;
		total += wr;
		ptr += wr;
		if (ret == Z_STREAM_END)
			break; /* XXX restart inflate */
	}
	return total;
}

size_t gzfread(void *buf, size_t size, size_t nitems, gzFile file)
{
	int rd = gzread(file, buf, size * nitems);
	if (rd < 0)
		return 0;
	return rd;
}

int gzwrite(gzFile file, void *buf, unsigned len)
{
	if (!file->buf)
	{
		if (!file->buf_size)
			file->buf_size = 1024 * 8;
		file->buf = malloc(file->buf_size);
		if (!file->buf)
			return -1;
	}
	if (len > INT_MAX)
		return -1;
	switch (file->direction)
	{
		case -1:
			if (deflateInit(&file->stream, Z_DEFAULT_COMPRESSION) != Z_OK)
				return -1;
			file->direction = 1;
			break;
		case 1:
			break;
		default:
			return -1;
	}
	uint8_t *ptr = buf;
	int total = 0;
	while (len)
	{
		file->stream.avail_in = len;
		file->stream.next_in = ptr;
		file->stream.avail_out = file->buf_size - file->buf_len;
		file->stream.next_out = &file->buf[file->buf_len];
		int ret = deflate(&file->stream, Z_NO_FLUSH);
		switch (ret)
		{
			case Z_OK:
				break;
			default:
				return -1;
		}
		file->buf_len = file->buf_size - file->stream.avail_out;
		size_t rd = len - file->stream.avail_in;
		len -= rd;
		total += rd;
		ptr += rd;
		size_t wr = fwrite(file->buf, 1, file->buf_len, file->fp);
		if (ferror(file->fp))
			return -1;
		memcpy(file->buf, &file->buf[wr], file->buf_len - wr);
		file->buf_len -= wr;
	}
	return total;
}

size_t gzfwrite(void *buf, size_t size, size_t nitems, gzFile file)
{
	int wr = gzwrite(file, buf, size * nitems);
	if (wr < 0)
		return 0;
	return wr;
}

int gzclose(gzFile file)
{
	if (!file)
		return Z_STREAM_ERROR;
	int ret = Z_OK;
	switch (file->direction)
	{
		case 0:
			inflateEnd(&file->stream);
			break;
		case 1:
		{
			do
			{
				file->stream.avail_in = 0;
				file->stream.next_in = NULL;
				file->stream.avail_out = file->buf_size - file->buf_len;
				file->stream.next_out = &file->buf[file->buf_len];
				ret = deflate(&file->stream, Z_FINISH);
				if (ret != Z_OK && ret != Z_STREAM_END)
					break;
				file->buf_len = file->buf_size - file->stream.avail_out;
				size_t wr = fwrite(file->buf, 1, file->buf_len, file->fp);
				if (ferror(file->fp))
					return -1;
				memcpy(file->buf, &file->buf[wr], file->buf_len - wr);
				file->buf_len -= wr;
				if (ret == Z_STREAM_END)
				{
					ret = Z_OK;
					break;
				}
			} while (file->buf_len);
			deflateEnd(&file->stream);
			break;
		}
	}
	fclose(file->fp);
	free(file->buf);
	free(file);
	return Z_OK;
}
