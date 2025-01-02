#include "_string.h"

#include <string.h>

size_t strlcpy(char *d, const char *s, size_t n)
{
	if (!n)
		return strlen(s);
	size_t i = 0;
	while (1)
	{
		if (i >= n - 1)
			goto end;
		if (!(((size_t)s + i) % sizeof(size_t)))
			break;
		unsigned char v = s[i];
		d[i] = v;
		if (!v)
			return i;
		i++;
	}
	while (i + sizeof(size_t) < n)
	{
		size_t v = *(size_t*)&s[i];
		if (haszero(v))
			break;
		*(size_t*)&d[i] = v;
		i += sizeof(size_t);
	}
	while (i < n - 1)
	{
		unsigned char v = s[i];
		d[i] = v;
		if (!v)
			return i;
		i++;
	}
end:
	d[i] = '\0';
	return i + strlen(&s[i]);
}
