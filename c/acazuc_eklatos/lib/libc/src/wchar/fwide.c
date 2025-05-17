#include "../stdio/_stdio.h"

#include <wchar.h>
#include <stdio.h>

int
fwide(FILE *fp, int mode)
{
	int ret;

	flockfile(fp);
	if (mode && !fp->wide)
		fp->wide = mode;
	ret = fp->wide;
	funlockfile(fp);
	return ret;
}
