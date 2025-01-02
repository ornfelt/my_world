#include <wchar.h>

wchar_t *wmemcpy(wchar_t *d, const wchar_t *s, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		d[i] = s[i];
	return d;
}
