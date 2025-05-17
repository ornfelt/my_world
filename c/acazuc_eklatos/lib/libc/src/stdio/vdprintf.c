#include "_printf.h"

#include <unistd.h>
#include <stdio.h>

int
vdprintf(int fd, const char *fmt, va_list va_arg)
{
	struct buf buf;
	int ret;

	buf.type = PRINTF_FD;
	buf.fd.fd = fd;
	buf.fd.buf_pos = 0;
	ret = printf_buf(&buf, fmt, va_arg);
	if (buf.fd.buf_pos)
	{
		write(buf.fd.fd, buf.fd.buf, buf.fd.buf_pos);
		ret += buf.fd.buf_pos;
	}
	return ret;
}
