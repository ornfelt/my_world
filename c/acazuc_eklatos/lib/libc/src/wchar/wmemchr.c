#include <wchar.h>

wchar_t *
wmemchr(const wchar_t *s, wchar_t wc, size_t n)
{
	for (size_t i = 0; i < n; ++i)
	{
		if (s[i] == wc)
			return (wchar_t*)&s[i];
	}
	return NULL;
}
