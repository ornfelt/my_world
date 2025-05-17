#include <string.h>
#include <wchar.h>

size_t
mbrlen(const char *s, size_t n, mbstate_t *ps)
{
	return mbrtowc(NULL, s, n, ps);
}
