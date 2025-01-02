#undef _FORTIFY_SOURCE
#include "../_chk.h"
#include "_string.h"

#include <string.h>

char *stpcpy(char *d, const char *s)
{
	size_t i = 0;
	while (((size_t)s + i) % sizeof(size_t))
	{
		unsigned char v = s[i];
		d[i] = v;
		if (!v)
			return &d[i];
		i++;
	}
	while (1)
	{
		size_t v = *(size_t*)&s[i];
		if (haszero(v))
			break;
		*(size_t*)&d[i] = v;
		i += sizeof(size_t);
	}
	while (1)
	{
		unsigned char v = s[i];
		d[i] = v;
		if (!v)
			break;
		i++;
	}
	return &d[i];
}

char *__stpcpy_chk(char *d, const char *s, size_t ds)
{
	size_t n = strlcpy(d, s, ds);
	if (n >= ds)
		__chk_fail();
	return &d[n];
}
