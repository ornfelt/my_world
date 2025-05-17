#include <stdlib.h>
#include <wchar.h>

int
mbtowc(wchar_t *wc, const char *s, size_t n)
{
	return mbrtowc(wc, s, n, NULL);
}
