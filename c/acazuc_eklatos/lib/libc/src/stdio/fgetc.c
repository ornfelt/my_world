#include <stdio.h>

int fgetc(FILE *fp)
{
	unsigned char c;
	if (fread(&c, 1, 1, fp) != 1)
		return EOF;
	return c;
}
