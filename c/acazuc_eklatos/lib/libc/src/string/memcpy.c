#undef _FORTIFY_SOURCE
#include "../_chk.h"

#include <string.h>

void *memcpy(void *d, const void *s, size_t n)
{
	size_t i = 0;
	if (n >= sizeof(size_t))
	{
		size_t end = n - n % sizeof(size_t);
		do
		{
			*(size_t*)&((unsigned char*)d)[i] = *(size_t*)&((unsigned char*)s)[i];
			i += sizeof(size_t);
		} while (i < end);
	}
	while (i < n)
	{
		((unsigned char*)d)[i] = ((const unsigned char*)s)[i];
		i++;
	}
	return d;
}

void *__memcpy_chk(void *d, const void *s, size_t n, size_t ds)
{
	if (n > ds)
		__chk_fail();
	return memcpy(d, s, n);
}
