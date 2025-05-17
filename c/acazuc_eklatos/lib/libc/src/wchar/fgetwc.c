#include <stdio.h>
#include <wchar.h>

wint_t
fgetwc(FILE *fp)
{
	wint_t ret;

	flockfile(fp);
	ret = getwc_unlocked(fp);
	funlockfile(fp);
	return ret;
}
