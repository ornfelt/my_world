#include <string.h>

char *strnstr(const char *s1, const char *s2, size_t n)
{
	if (!*s2)
		return (char*)s1;
	for (size_t i = 0; i < n && s1[i]; ++i)
	{
		for (size_t j = 0; (s1[i + j] == s2[j] || !s2[j]); ++j)
		{
			if (!s2[j])
				return (char*)s1 + i;
			if (i + j >= n)
				return NULL;
		}
	}
	return NULL;
}
