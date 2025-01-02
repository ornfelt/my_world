#include <stdio.h>

int fgetpos(FILE *fp, fpos_t *pos)
{
	long res = ftell(fp);
	if (res == -1)
		return -1;
	*pos = res;
	return 0;
}
