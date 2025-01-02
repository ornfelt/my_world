#include <stdio.h>
#include <wchar.h>

wint_t fgetwc(FILE *fp)
{
	flockfile(fp);
	wint_t ret = getwc_unlocked(fp);
	funlockfile(fp);
	return ret;
}
