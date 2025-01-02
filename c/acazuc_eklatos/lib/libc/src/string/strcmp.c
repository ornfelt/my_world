#include <string.h>

int strcmp(const char *s1, const char *s2)
{
	size_t i = 0;
	while (s1[i] && s2[i] && s1[i] == s2[i])
		i++;
	return (int)((unsigned char*)s1)[i] - (int)((unsigned char*)s2)[i];
}
