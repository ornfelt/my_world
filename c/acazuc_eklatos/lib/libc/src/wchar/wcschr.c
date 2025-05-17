#include <wchar.h>

wchar_t *
wcschr(const wchar_t *s, wchar_t wc)
{
	while (1)
	{
		if (*s == wc)
			return (wchar_t*)s;
		if (!*s)
			break;
		s++;
	}
	return NULL;
}
