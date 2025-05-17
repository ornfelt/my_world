#include <stdio.h>
#include <wchar.h>

int
fputws(const wchar_t *ws, FILE *fp)
{
	int ret;

	flockfile(fp);
	ret = fputws_unlocked(ws, fp);
	funlockfile(fp);
	return ret;
}
