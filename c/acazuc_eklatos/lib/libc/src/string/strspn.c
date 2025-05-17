#include <string.h>

size_t
strspn(const char *s, const char *accept)
{
	size_t i = 0;
	while (strchr(accept, s[i]))
		++i;
	return i;
}
