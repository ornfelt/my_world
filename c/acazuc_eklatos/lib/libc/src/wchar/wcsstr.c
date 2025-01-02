#include <string.h>
#include <wchar.h>

wchar_t *wcsstr(const wchar_t *s, const wchar_t *needle)
{
	size_t n = wcslen(needle) * sizeof(*needle);
	while (*s)
	{
		if (!memcmp(s, needle, n))
			return (wchar_t*)s;
		s++;
	}
	return NULL;
}
