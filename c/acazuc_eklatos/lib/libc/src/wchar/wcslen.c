#include <wchar.h>

size_t wcslen(const wchar_t *s)
{
	size_t n = 0;
	while (*(s++))
		n++;
	return n;
}
