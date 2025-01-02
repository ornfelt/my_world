#include "_string.h"

#include <string.h>

void *memccpy(void *d, const void *s, int c, size_t n)
{
	size_t i = 0;
	if (n >= sizeof(size_t))
	{
		size_t cz = fillsize((unsigned char)c);
		size_t end = n - n % sizeof(size_t);
		do
		{
			size_t v = *(size_t*)&((unsigned char*)s)[i];
			size_t x = v ^ cz;
			if (haszero(x))
				break;
			*(size_t*)&((unsigned char*)d)[i] = v;
			i += sizeof(size_t);
		} while (i < end);
	}
	while (i < n)
	{
		unsigned char v = ((unsigned char*)s)[i];
		((unsigned char*)d)[i] = v;
		if (v == (unsigned char)c)
			return (unsigned char*)d + i + 1;
		i++;
	}
	return NULL;
}
