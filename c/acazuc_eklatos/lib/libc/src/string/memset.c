#undef _FORTIFY_SOURCE
#include "../_chk.h"

#include "_string.h"

#include <string.h>

void *
memset(void *d, int c, size_t n)
{
	size_t i = 0;
	if (n >= sizeof(size_t))
	{
		size_t cz = fillsize((unsigned char)c);
		size_t end = n - n % sizeof(size_t);
		do
		{
			*(size_t*)&((unsigned char*)d)[i] = cz;
			i += sizeof(size_t);
		}
		while (i < end);
	}
	while (i < n)
	{
		((unsigned char*)d)[i] = c;
		i++;
	}
	return d;
}

void *
__memset_chk(void *d, int c, size_t n, size_t ds)
{
	if (n > ds)
		__chk_fail();
	return memset(d, c, n);
}
