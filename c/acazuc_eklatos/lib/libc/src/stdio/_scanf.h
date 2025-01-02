#ifndef _SCANF_H
#define _SCANF_H

#include <stdio.h>

struct buf
{
	enum
	{
		SCANF_STR,
		SCANF_FP,
	} type;
	union
	{
		const char *str;
		FILE *fp;
	};
	int nconv;
	int ninput;
};

int scanf_buf(struct buf *buf, const char *fmt, va_list va_arg);

#endif
