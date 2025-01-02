#include <stdio.h>

int putw_unlocked(int w, FILE *fp)
{
	if (fwrite_unlocked(&w, 1, sizeof(w), fp) != sizeof(w))
		return EOF;
	return 0;
}
