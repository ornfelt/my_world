#include <stdio.h>

int putc_unlocked(int c, FILE *fp)
{
	return fputc_unlocked(c, fp);
}
