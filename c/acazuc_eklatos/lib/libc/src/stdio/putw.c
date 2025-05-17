#include <stdio.h>

int
putw(int w, FILE *fp)
{
	if (fwrite(&w, 1, sizeof(w), fp) != sizeof(w))
		return EOF;
	return 0;
}
