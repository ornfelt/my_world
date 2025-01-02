#include "_stdio.h"

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

struct files_head files = TAILQ_HEAD_INITIALIZER(files);

static char stdin_buf[BUFSIZ];
static char stdout_buf[BUFSIZ];

static FILE g_stdin =
{
	.io_funcs =
	{
		.read = io_read,
		.write = io_write,
		.seek = io_seek,
	},
	.cookie = &g_stdin,
	.fd = 0,
	.mode = O_RDONLY,
	.buf = stdin_buf,
	.buf_size = BUFSIZ,
	.buf_mode = _IOFBF,
	.unget = EOF,
};

static FILE g_stdout =
{
	.io_funcs =
	{
		.read = io_read,
		.write = io_write,
		.seek = io_seek,
	},
	.cookie = &g_stdout,
	.fd = 1,
	.mode = O_WRONLY,
	.buf = stdout_buf,
	.buf_size = BUFSIZ,
	.buf_mode = _IOLBF,
	.unget = EOF,
};

static FILE g_stderr =
{
	.io_funcs =
	{
		.read = io_read,
		.write = io_write,
		.seek = io_seek,
	},
	.cookie = &g_stderr,
	.fd = 2,
	.mode = O_WRONLY,
	.buf_mode = _IONBF,
	.unget = EOF,
};

FILE *stdin = &g_stdin;
FILE *stdout = &g_stdout;
FILE *stderr = &g_stderr;

ssize_t io_read(void *cookie, char *buf, size_t size)
{
	return read(((FILE*)cookie)->fd, buf, size);
}

ssize_t io_write(void *cookie, const char *buf, size_t size)
{
	return write(((FILE*)cookie)->fd, buf, size);
}

int io_seek(void *cookie, off_t off, int whence)
{
	return lseek(((FILE*)cookie)->fd, off, whence);
}

int io_close(void *cookie)
{
	return close(((FILE*)cookie)->fd);
}

void initfp(FILE *fp)
{
	fp->flush = NULL;
	fp->buf_pos = 0;
	fp->eof = 0;
	fp->err = 0;
	fp->fd = -1;
	fp->wide = 0;
	_libc_lock_init(&fp->lock);
}

FILE *mkfp(void)
{
	FILE *fp = malloc(sizeof(*fp));
	if (!fp)
		return NULL;
	fp->buf = malloc(BUFSIZ);
	if (!fp->buf)
	{
		free(fp);
		return NULL;
	}
	fp->buf_owned = 1;
	fp->buf_mode = _IOFBF;
	fp->buf_size = BUFSIZ;
	fp->buf_type = 0;
	fp->unget = EOF;
	initfp(fp);
	return fp;
}

static void handle_flags_ext(const char *mode, size_t *i, int *flags)
{
	while (1)
	{
		switch (mode[*i])
		{
			case 'b':
				break;
			case 'e':
				*flags |= O_CLOEXEC;
				break;
			case 'x':
				*flags |= O_EXCL;
				break;
			default:
				return;
		}
		(*i)++;
	}
}

int parse_flags(const char *mode, int *flags)
{
	size_t i = 0;
	*flags = 0;
	switch (mode[i])
	{
		case 'a':
			i++;
			handle_flags_ext(mode, &i, flags);
			*flags |= O_CREAT | O_APPEND;
			if (mode[i] == '+')
			{
				*flags |= O_RDWR;
				i++;
			}
			else
			{
				*flags |= O_WRONLY;
			}
			break;
		case 'r':
			i++;
			handle_flags_ext(mode, &i, flags);
			if (mode[i] == '+')
			{
				*flags |= O_RDWR;
				i++;
			}
			else
			{
				*flags |= O_RDONLY;
			}
			break;
		case 'w':
			i++;
			handle_flags_ext(mode, &i, flags);
			*flags |= O_CREAT | O_TRUNC;
			if (mode[i] == '+')
			{
				*flags |= O_RDWR;
				i++;
			}
			else
			{
				*flags |= O_WRONLY;
			}
			break;
		default:
			return 0;
	}
	if (mode[i])
	{
		handle_flags_ext(mode, &i, flags);
		if (mode[i])
			return 0;
	}
	return 1;
}

size_t write_data(FILE *fp, const void *data, size_t count)
{
	if (!fp->io_funcs.write)
		return 0;
	size_t res = 0;
	while (res < count)
	{
		ssize_t wr = fp->io_funcs.write(fp->cookie,
		                                (char*)&((uint8_t*)data)[res],
		                                count - res);
		if (wr == -1)
		{
			if (errno == EINTR)
				continue;
			fp->err = 1;
			return res;
		}
		res += wr;
	}
	return res;
}
