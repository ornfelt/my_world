#include <wchar.h>

wchar_t *
wmemccpy(wchar_t *d, const wchar_t *s, wchar_t wc, size_t n)
{
	size_t i = 0;

	while (i < n)
	{
		wchar_t v = s[i];
		d[i] = v;
		if (v == wc)
			return d + i + 1;
		i++;
	}
	return NULL;
}
