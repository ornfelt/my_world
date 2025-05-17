#undef _FORTIFY_SOURCE
#include "../_chk.h"
#include "_string.h"

#include <string.h>

char *
strncpy(char *d, const char *s, size_t n)
{
	size_t i = 0;
	while (1)
	{
		if (i >= n)
			return d;
		if (!(((size_t)s + i) % sizeof(size_t)))
			break;
		unsigned char v = s[i];
		d[i] = v;
		i++;
		if (!v)
			goto end;
	}
	while (i + sizeof(size_t) <= n)
	{
		size_t v = *(size_t*)&s[i];
		if (haszero(v))
			break;
		*(size_t*)&d[i] = v;
		i += sizeof(size_t);
	}
	while (1)
	{
		if (i >= n)
			return d;
		unsigned char v = s[i];
		d[i] = v;
		i++;
		if (!v)
			break;
	}
end:
	memset(&d[i], 0, n - i);
	return d;
}

char *
__strncpy_chk(char *d, const char *s, size_t n, size_t ds)
{
	if (n >= ds)
		__chk_fail();
	return strncpy(d, s, n);
}
