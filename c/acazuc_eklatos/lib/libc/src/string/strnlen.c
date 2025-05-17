#include "_string.h"

#include <string.h>

size_t
strnlen(const char *s, size_t maxlen)
{
	size_t i = 0;
	if (i < maxlen && ((size_t)s + i) % sizeof(size_t))
	{
		if (!s[i])
			return i;
		i++;
	}
	while (i + sizeof(size_t) <= maxlen)
	{
		size_t v = *(size_t*)&s[i];
		if (haszero(v))
			break;
		i += sizeof(size_t);
	}
	while (i < maxlen && s[i])
		i++;
	return i;
}
