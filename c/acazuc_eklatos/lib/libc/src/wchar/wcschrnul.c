#include <wchar.h>

wchar_t *
wcschrnul(const wchar_t *s, wchar_t wc)
{
	while (*s)
	{
		if (*s == wc)
			return (wchar_t*)s;
		s++;
	}
	return (wchar_t*)s;
}
