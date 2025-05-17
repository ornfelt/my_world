#include <wchar.h>

wchar_t *
wcsrchr(const wchar_t *s, wchar_t wc)
{
	wchar_t *last = NULL;

	while (1)
	{
		if (*s == wc)
			last = (wchar_t*)s;
		if (!*s)
			return last;
		s++;
	}
	return last;
}
