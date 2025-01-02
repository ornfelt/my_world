#include "_stdio.h"

#include <string.h>
#include <stdio.h>

static size_t write_buffered(FILE *fp, const void *data, size_t count)
{
	if (!fp->buf_type)
	{
		if (fflush_unlocked(fp))
		{
			fp->err = 1;
			return 0;
		}
	}
	fp->buf_type = 1;
	size_t ret = 0;
	while (ret < count)
	{
		size_t rem = count - ret;
		/* bypass buffering if we can directly write a full buffer
		 * from src
		 */
		if (!fp->buf_pos && rem >= fp->buf_size)
			return ret + write_data(fp, &((uint8_t*)data)[ret],
			                        rem);
		size_t buf_rem = fp->buf_size - fp->buf_pos;
		if (rem < buf_rem)
		{
			memcpy(fp->buf + fp->buf_pos, &((uint8_t*)data)[ret],
			       rem);
			fp->buf_pos += rem;
			return count;
		}
		memcpy(fp->buf + fp->buf_pos, &((uint8_t*)data)[ret], buf_rem);
		ret += buf_rem;
		size_t wr = write_data(fp, fp->buf, fp->buf_size);
		if (wr != fp->buf_size)
		{
			memmove(fp->buf, fp->buf + wr, fp->buf_pos - wr);
			fp->buf_pos -= wr;
			return ret;
		}
		fp->buf_pos = 0;
	}
	return ret;
}

size_t fwrite_unlocked(const void *ptr, size_t size, size_t nmemb, FILE *fp)
{
	size_t total;
	size_t ret;
	if (__builtin_mul_overflow(size, nmemb, &total))
	{
		fp->err = 1;
		return 0;
	}
	if (!total)
		return 0;
	switch (fp->buf_mode)
	{
		default:
		case _IONBF:
			ret = write_data(fp, ptr, total);
			break;
		case _IOFBF:
			ret = write_buffered(fp, ptr, total);
			break;
		case _IOLBF:
		{
			void *nl = memrchr(ptr, '\n', total);
			if (!nl)
			{
				ret = write_buffered(fp, ptr, total);
				break;
			}
			size_t n = 1 + (uint8_t*)nl - (uint8_t*)ptr;
			if (n == total)
			{
				size_t wr = write_buffered(fp, ptr, total);
				fflush_unlocked(fp);
				ret = wr;
				break;
			}
			if (fflush_unlocked(fp))
			{
				ret = 0;
				break;
			}
			size_t wr = write_data(fp, ptr, n);
			if (wr != n)
			{
				ret = wr;
				break;
			}
			ptr = &((uint8_t*)ptr)[wr];
			total -= wr;
			ret = (wr + write_buffered(fp, ptr, total));
			break;
		}
	}
	return ret / size;
}
