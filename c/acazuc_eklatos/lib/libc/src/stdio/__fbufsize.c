#include "_stdio.h"

#include <stdio.h>

size_t
__fbufsize(FILE *fp)
{
	return fp->buf_size;
}
