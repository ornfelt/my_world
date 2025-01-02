#include <stdio.h>
#include <wchar.h>

int fputws(const wchar_t *ws, FILE *fp)
{
	flockfile(fp);
	int ret = fputws_unlocked(ws, fp);
	funlockfile(fp);
	return ret;
}
