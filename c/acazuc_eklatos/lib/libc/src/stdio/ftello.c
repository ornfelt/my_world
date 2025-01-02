#include <stdio.h>

off_t ftello(FILE *fp)
{
	return ftell(fp);
}
