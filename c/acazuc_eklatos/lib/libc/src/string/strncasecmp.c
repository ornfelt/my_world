#include <string.h>
#include <ctype.h>

int strncasecmp(const char *s1, const char *s2, size_t n)
{
	size_t i = 0;
	while (i < n && s1[i] && s2[i])
	{
		char c1 = tolower(s1[i]);
		char c2 = tolower(s2[i]);
		if (c1 != c2)
			return c1 - c2;
		i++;
	}
	if (i == n)
		return 0;
	return (((unsigned char*)s1)[i] - ((unsigned char*)s2)[i]);
}
