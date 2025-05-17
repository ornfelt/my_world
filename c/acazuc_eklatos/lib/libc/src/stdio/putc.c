#include <stdio.h>

int
putc(int c, FILE *fp)
{
	return fputc(c, fp);
}
