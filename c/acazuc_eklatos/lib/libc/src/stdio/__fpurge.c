#include <stdio.h>

void
__fpurge(FILE *fp)
{
	fpurge(fp);
}
