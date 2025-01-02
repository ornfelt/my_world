#undef _FORTIFY_SOURCE
#include "../_chk.h"
#include "_string.h"

#include <string.h>

char *strncat(char *d, const char *s, size_t n)
{
	size_t i = strlen(d);
	size_t j = 0;
	while (1)
	{
		if (j >= n)
			goto end;
		if (!(((size_t)s + j) % sizeof(size_t)))
			break;
		unsigned char v = s[j];
		d[i] = v;
		if (!v)
			return d;
		i++;
		j++;
	}
	while (j + sizeof(size_t) <= n)
	{
		size_t v = *(size_t*)&s[j];
		if (haszero(v))
			break;
		*(size_t*)&d[i] = v;
		i += sizeof(size_t);
		j += sizeof(size_t);
	}
	while (j < n)
	{
		unsigned char v = s[j];
		d[i] = v;
		if (!v)
			return d;
		i++;
		j++;
	}
end:
	d[i] = '\0';
	return d;
}

char *__strncat_chk(char *d, const char *s, size_t n, size_t ds)
{
	if (n >= ds)
		__chk_fail();
	return strncat(d, s, n);
}
