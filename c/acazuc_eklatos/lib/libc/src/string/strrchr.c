# include "_string.h"

#include <string.h>

char *strrchr(const char *s, int c)
{
	char *ret = NULL;
	size_t i = 0;
	while (((size_t)s + i) % sizeof(size_t))
	{
		unsigned char v = (unsigned char)s[i];
		if (v == (unsigned char)c)
			ret = (char*)&s[i];
		if (!v)
			return (unsigned char)c ? ret : (char*)&s[i];
		i++;
	}
	size_t cz = fillsize((unsigned char)c);
	while (1)
	{
		size_t v = *(size_t*)&s[i];
		if (haszero(v))
			break;
		v ^= cz;
		if (haszero(v))
			break;
		i += sizeof(size_t);
	}
	while (1)
	{
		unsigned char v = (unsigned char)s[i];
		if (v == (unsigned char)c)
			ret = (char*)&s[i];
		if (!v)
			break;
		i++;
	}
	return ret;
}