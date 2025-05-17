#include <string.h>
#include <stdlib.h>

char *
strdup(const char *s)
{
	size_t len;
	char *ret;

	len = strlen(s);
	ret = malloc(len + 1);
	if (!ret)
		return NULL;
	memcpy(ret, s, len + 1);
	return ret;
}
