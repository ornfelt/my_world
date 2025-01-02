#include <wchar.h>
#include <stdio.h>

wchar_t *fgetws_unlocked(wchar_t *ws, int size, FILE *fp)
{
	if (size <= 0)
		return NULL;
	if (size == 1)
	{
		*ws = L'\0';
		return ws;
	}
	size--;
	wchar_t *d = ws;
	while (size)
	{
		wint_t wc = fgetwc_unlocked(fp);
		if (wc == WEOF)
			break;
		if (*d == L'\n')
		{
			d[1] = L'\0';
			return ws;
		}
		*d = wc;
		d++;
		size--;
	}
	if (size && ferror_unlocked(fp))
		return NULL;
	if (d == ws)
		return NULL;
	*d = L'\0';
	return ws;
}
