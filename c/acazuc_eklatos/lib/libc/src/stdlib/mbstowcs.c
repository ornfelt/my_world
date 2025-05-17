#include <stdlib.h>
#include <wchar.h>

size_t
mbstowcs(wchar_t *d, const char *s, size_t n)
{
	return mbsrtowcs(d, &s, n, NULL);
}
