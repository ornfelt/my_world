#include <stdio.h>

int
fseeko(FILE *fp, off_t offset, int whence)
{
	return fseek(fp, offset, whence);
}
