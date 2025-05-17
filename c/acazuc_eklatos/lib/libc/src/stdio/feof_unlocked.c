#include "_stdio.h"

#include <stdio.h>

int
feof_unlocked(FILE *fp)
{
	return fp->eof;
}
