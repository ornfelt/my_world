#include <stdio.h>

int
fputc(int c, FILE *fp)
{
	unsigned char cc = c;

	if (fwrite(&cc, 1, 1, fp) != 1)
		return EOF;
	return cc;
}
