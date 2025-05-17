#include <wchar.h>

wchar_t *
wmemrchr(const wchar_t *s, wchar_t wc, size_t n)
{
	wchar_t *last = NULL;

	for (size_t i = 0; i < n; ++i)
	{
		if (s[i] == wc)
			last = (wchar_t*)&s[i];
	}
	return last;
}
