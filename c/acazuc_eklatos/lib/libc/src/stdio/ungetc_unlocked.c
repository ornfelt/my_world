#include "_stdio.h"

#include <stdio.h>

int ungetc_unlocked(int c, FILE *fp)
{
	if (fp->unget != EOF)
		return EOF;
	fp->unget = (unsigned char)c;
	return (unsigned char)c;
}
