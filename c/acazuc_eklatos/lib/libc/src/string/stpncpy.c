#include "_string.h"

#include <string.h>

char *
stpncpy(char *d, const char *s, size_t n)
{
	size_t i = 0;
	while (((size_t)s + i) % sizeof(size_t))
	{
		if (!n)
			return &d[i];
		unsigned char v = s[i];
		d[i] = v;
		if (!v)
			goto end;
		i++;
		n--;
	}
	while (n >= sizeof(size_t))
	{
		size_t v = *(size_t*)&s[i];
		if (haszero(v))
			break;
		*(size_t*)&d[i] = v;
		i += sizeof(size_t);
		n -= sizeof(size_t);
	}
	while (n)
	{
		unsigned char v = s[i];
		d[i] = v;
		if (!v)
			break;
		i++;
		n--;
	}
end:
	memset(&d[i], 0, n);
	return &d[i];
}
