#undef _FORTIFY_SOURCE
#include "../_chk.h"
#include "_string.h"

#include <string.h>

void *
memmove(void *d, const void *s, size_t n)
{
	if (!n)
		return d;
	if (d == s)
		return d;
	if (d < s)
		return memcpy(d, s, n);
	if ((size_t)((unsigned char*)d - (unsigned char*)s) >= sizeof(size_t))
	{
		while (n >= sizeof(size_t))
		{
			n -= sizeof(size_t);
			*(size_t*)&((unsigned char*)d)[n] = *(size_t*)&((unsigned char*)s)[n];
		}
	}
	while (n)
	{
		n--;
		((unsigned char*)d)[n] = ((unsigned char*)s)[n];
	}
	return d;
}

void *
__memmove_chk(void *d, const void *s, size_t n, size_t ds)
{
	if (n > ds)
		__chk_fail();
	return memmove(d, s, n);
}
