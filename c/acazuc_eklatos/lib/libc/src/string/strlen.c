#include "_string.h"

#include <string.h>

size_t
strlen(const char *s)
{
	size_t i = 0;

	while (((size_t)s + i) % sizeof(size_t))
	{
		if (!s[i])
			return i;
		i++;
	}
	while (1)
	{
		size_t v = *(size_t*)&s[i];
		if (haszero(v))
			break;
		i += sizeof(size_t);
	}
	while (s[i])
		i++;
	return i;
}
