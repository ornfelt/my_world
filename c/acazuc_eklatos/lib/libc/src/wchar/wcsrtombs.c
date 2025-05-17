#include <wchar.h>

size_t
wcsrtombs(char *d, const wchar_t **s, size_t n, mbstate_t *ps)
{
	return wcsnrtombs(d, s, (size_t)-1, n, ps);
}
