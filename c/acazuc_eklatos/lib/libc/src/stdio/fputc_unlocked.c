#include <stdio.h>

int
fputc_unlocked(int c, FILE *fp)
{
	unsigned char cc = c;

	if (fwrite_unlocked(&cc, 1, 1, fp) != 1)
		return EOF;
	return cc;
}
