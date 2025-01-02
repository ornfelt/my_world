#undef _FORTIFY_SOURCE
#include "../_chk.h"

#include <string.h>

char *strcat(char *d, const char *s)
{
	size_t i = strlen(d);
	strcpy(&d[i], s);
	return d;
}

char *__strcat_chk(char *d, const char *s, size_t ds)
{
	if (strlcat(d, s, ds) >= ds)
		__chk_fail();
	return strcat(d, s);
}
