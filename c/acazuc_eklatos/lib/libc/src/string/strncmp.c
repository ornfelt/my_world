#include <string.h>

int
strncmp(const char *s1, const char *s2, size_t n)
{
	size_t i;
	for (i = 0; i < n && s1[i] && s2[i]; ++i)
	{
		int d = (int)((unsigned char*)s1)[i] - (int)((unsigned char*)s2)[i];
		if (d)
			return d;
	}
	if (i == n)
		return 0;
	return (int)((unsigned char*)s1)[i] - (int)((unsigned char*)s2)[i];
}
