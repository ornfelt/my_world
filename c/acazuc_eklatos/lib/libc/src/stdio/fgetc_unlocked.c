#include <stdio.h>

int
fgetc_unlocked(FILE *fp)
{
	unsigned char c;

	if (fread_unlocked(&c, 1, 1, fp) != 1)
		return EOF;
	return c;
}
