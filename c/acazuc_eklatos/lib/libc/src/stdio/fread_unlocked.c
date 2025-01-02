#include "_stdio.h"

#include <string.h>
#include <stdio.h>
#include <errno.h>

static size_t read_data(FILE *fp, void *data, size_t count, size_t threshold)
{
	if (!fp->io_funcs.read)
	{
		fp->eof = 1;
		return 0;
	}
	size_t ret = 0;
	while (ret < threshold)
	{
		ssize_t rd = fp->io_funcs.read(fp->cookie,
		                               (char*)&((uint8_t*)data)[ret],
		                               count - ret);
		if (rd == -1)
		{
			if (errno == EINTR)
				continue;
			fp->err = 1;
			return ret;
		}
		if (rd == 0)
		{
			fp->eof = 1;
			return ret;
		}
		ret += rd;
	}
	return ret;
}

size_t fread_unlocked(void *ptr, size_t size, size_t nmemb, FILE *fp)
{
	size_t total;
	size_t ret = 0;
	if (__builtin_mul_overflow(size, nmemb, &total))
	{
		fp->err = 1;
		return 0;
	}
	if (!total)
		return 0;
	if (fp->unget != EOF)
	{
		*(uint8_t*)ptr = (uint8_t)fp->unget;
		total--;
		fp->unget = EOF;
		if (!total)
			return 1;
		ret = 1;
	}
	switch (fp->buf_mode)
	{
		default:
		case _IONBF:
			ret += read_data(fp, &((uint8_t*)ptr)[ret], total, total);
			break;
		case _IOFBF:
			if (fp->buf_type)
			{
				if (fflush_unlocked(fp))
				{
					fp->err = 1;
					ret = 0;
					break;
				}
			}
			fp->buf_type = 0;
			while (ret < total)
			{
				size_t rem = total - ret;
				/* bypass buffering if we can directly write a full
				 * buffer from src
				 */
				if (!fp->buf_pos && rem >= fp->buf_size)
				{
					ret += read_data(fp,
					                 &((uint8_t*)ptr)[ret],
					                 rem, rem);
					break;
				}
				size_t buf_rem = fp->buf_pos;
				if (rem < buf_rem)
				{
					memcpy(&((uint8_t*)ptr)[ret], fp->buf, rem);
					memcpy(fp->buf, fp->buf + rem, buf_rem - rem);
					fp->buf_pos -= rem;
					ret += rem;
					break;
				}
				if (buf_rem)
				{
					memcpy(&((uint8_t*)ptr)[ret], fp->buf, buf_rem);
					ret += buf_rem;
					fp->buf_pos = 0;
					continue;
				}
				size_t rd = read_data(fp, fp->buf, fp->buf_size, rem);
				fp->buf_pos = rd;
				if (!rd)
					break;
			}
			break;
		case _IOLBF:
			/* XXX */
			ret += read_data(fp, &((uint8_t*)ptr)[ret], total, total);
			break;
	}
	return ret / size;
}
