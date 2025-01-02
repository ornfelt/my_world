#ifndef _PRINTF_H
#define _PRINTF_H

#include <stdio.h>

struct buf
{
	enum
	{
		PRINTF_BUF,
		PRINTF_FP,
		PRINTF_FD,
	} type;
	union
	{
		struct
		{
			char *data;
			size_t size;
		};
		FILE *fp;
		struct
		{
			char buf[4096];
			size_t buf_pos;
			int fd;
		} fd;
	};
	size_t len;
};

int printf_buf(struct buf *buf, const char *fmt, va_list va_arg);

#endif
