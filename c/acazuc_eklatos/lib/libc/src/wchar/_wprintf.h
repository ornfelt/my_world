#ifndef _PRINTF_H
#define _PRINTF_H

#include <stdio.h>

struct buf
{
	enum
	{
		PRINTF_BUF,
		PRINTF_FP,
	} type;
	union
	{
		struct
		{
			wchar_t *data;
			size_t size;
		};
		FILE *fp;
	};
	size_t len;
};

int wprintf_buf(struct buf *buf, const wchar_t *fmt, va_list va_arg);

#endif
