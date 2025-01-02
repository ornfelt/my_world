#include <string.h>

char *strstr(const char *s1, const char *s2)
{
	if (!*s2)
		return (char*)s1;
	for (size_t i = 0; s1[i]; ++i)
	{
		size_t j;
		for (j = 0; s1[i + j] == s2[j]; ++j)
		{
			if (!s2[j])
				return (char*)s1 + i;
		}
		if (!s2[j])
			return (char*)s1 + i;
	}
	return NULL;
}
