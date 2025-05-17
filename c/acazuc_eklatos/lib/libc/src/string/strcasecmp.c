#include <string.h>
#include <ctype.h>

int
strcasecmp(const char *s1, const char *s2)
{
	size_t i = 0;
	while (s1[i] && s2[i])
	{
		char c1 = tolower(s1[i]);
		char c2 = tolower(s2[i]);
		if (c1 != c2)
			return c1 - c2;
		i++;
	}
	return (((unsigned char*)s1)[i] - ((unsigned char*)s2)[i]);
}
