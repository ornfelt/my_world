#include <stdio.h>

int
fsetpos(FILE *fp, const fpos_t *pos)
{
	return fseek(fp, *pos, SEEK_SET);
}
