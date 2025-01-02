#include "_stdio.h"

#include <stdio.h>

size_t __fpending(FILE *fp)
{
	if (!fp->buf_type)
		return fp->buf_pos;
	return 0;
}
