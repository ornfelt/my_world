#include <wchar.h>
#include <stdio.h>

wchar_t *fgetws(wchar_t *ws, int size, FILE *fp)
{
	flockfile(fp);
	wchar_t *ret = fgetws_unlocked(ws, size, fp);
	funlockfile(fp);
	return ret;
}
