#include <wchar.h>

wchar_t *wcspbrk(const wchar_t *s, const wchar_t *accept)
{
	while (*s)
	{
		if (wcschr(accept, *s))
			return (wchar_t*)s;
		s++;
	}
	return NULL;
}
