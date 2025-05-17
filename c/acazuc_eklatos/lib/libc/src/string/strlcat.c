#include "_string.h"

#include <string.h>

size_t
strlcat(char *d, const char *s, size_t n)
{
	if (!n)
		return strlen(d) + strlen(s);
	size_t i = strlen(d);
	if (i >= n)
		return i + strlen(s);
	size_t j = 0;
	while (1)
	{
		if (i >= n - 1)
			goto end;
		if (!(((size_t)s + j) % sizeof(size_t)))
			break;
		unsigned char v = s[j];
		d[i] = v;
		if (!v)
			return i;
		i++;
		j++;
	}
	while (i + sizeof(size_t) < n)
	{
		size_t v = *(size_t*)&s[j];
		if (haszero(v))
			break;
		*(size_t*)&d[i] = v;
		i += sizeof(size_t);
		j += sizeof(size_t);
	}
	while (i < n - 1)
	{
		unsigned char v = s[j];
		d[i] = v;
		if (!v)
			return i;
		i++;
		j++;
	}
end:
	d[i] = '\0';
	return i + strlen(&s[j]);
}
