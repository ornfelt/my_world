#include <stdio.h>
#include <wchar.h>
#include <errno.h>

wint_t
fputwc(wchar_t wc, FILE *fp)
{
	char buf[4];
	size_t wn;

	wn = wcrtomb(buf, wc, NULL);
	if (wn == (size_t)-1)
		return WEOF;
	if (fwrite(buf, wn, 1, fp) != wn)
		return WEOF;
	return wc;
}
