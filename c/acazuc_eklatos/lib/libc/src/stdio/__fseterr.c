#include "_stdio.h"

#include <stdio.h>

void __fseterr(FILE *fp)
{
	fp->err = 1;
}
