#include <wchar.h>

wchar_t *
wmemset(wchar_t *d, wchar_t wc, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		d[i] = wc;
	return d;
}
