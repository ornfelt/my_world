#include <string.h>

char *
strpbrk(const char *s, const char *accept)
{
	for (size_t i = 0; s[i]; ++i)
	{
		if (strchr(accept, s[i]))
			return (char*)&s[i];
	}
	return NULL;
}
