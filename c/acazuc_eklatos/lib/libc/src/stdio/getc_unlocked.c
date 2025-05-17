#include <stdio.h>

int
getc_unlocked(FILE *fp)
{
	return fgetc_unlocked(fp);
}
