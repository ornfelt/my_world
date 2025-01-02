#include <string.h>

size_t strcspn(const char *s, const char *reject)
{
	size_t i = 0;
	while (!strchr(reject, s[i]))
		++i;
	return i;
}
