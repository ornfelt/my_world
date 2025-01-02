#include <string.h>

size_t strxfrm(char *dst, const char *src, size_t n)
{
	/* XXX convert */
	return strlcpy(dst, src, n);
}
