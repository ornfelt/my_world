#include "_string.h"

#include <string.h>

void *memrchr(const void *s, int c, size_t n)
{
	if (n >= sizeof(size_t))
	{
		size_t cz = fillsize((unsigned char)c);
		do
		{
			size_t nxt = n - sizeof(size_t);
			size_t v = *(size_t*)&((unsigned char*)s)[nxt] ^ cz;
			if (haszero(v))
				break;
			n = nxt;
		} while (n >= sizeof(size_t));
	}
	while (n)
	{
		size_t nxt = n - 1;
		unsigned char *ptr = &((unsigned char*)s)[nxt];
		if (*ptr == (unsigned char)c)
			return ptr;
		n = nxt;
	}
	return NULL;
}
