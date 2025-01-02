#include <stdio.h>
#include <wchar.h>
#include <errno.h>

wint_t fputwc_unlocked(wchar_t wc, FILE *fp)
{
	char buf[4];
	size_t wn = wcrtomb(buf, wc, NULL);
	if (wn == (size_t)-1)
		return WEOF;
	if (fwrite_unlocked(buf, wn, 1, fp) != wn)
		return WEOF;
	return wc;
}
