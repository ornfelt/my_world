#include "_string.h"

#include <string.h>

void *
memchr(const void *s, int c, size_t n)
{
	size_t i = 0;
	if (n >= sizeof(size_t))
	{
		size_t cz = fillsize((unsigned char)c);
		size_t end = n - n % sizeof(size_t);
		do
		{
			size_t v = *(size_t*)&((unsigned char*)s)[i] ^ cz;
			if (haszero(v))
				break;
			i += sizeof(size_t);
		}
		while (i < end);
	}
	while (i < n)
	{
		if (((unsigned char*)s)[i] == (unsigned char)c)
			return (unsigned char*)s + i;
		i++;
	}
	return NULL;
}
