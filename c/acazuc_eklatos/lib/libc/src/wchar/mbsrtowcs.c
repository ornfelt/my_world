#include <wchar.h>

size_t
mbsrtowcs(wchar_t *d, const char **s, size_t n, mbstate_t *ps)
{
	return mbsnrtowcs(d, s, (size_t)-1, n, ps);
}
