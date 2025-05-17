#include <wchar.h>

size_t
wcsnlen(const wchar_t *s, size_t n)
{
	size_t org = n;

	while (n && *s)
	{
		s++;
		n--;
	}
	return org - n;
}
