#include <string.h>
#include <stdlib.h>

char *strdup(const char *s)
{
	size_t len = strlen(s);
	char *ret = malloc(len + 1);
	if (!ret)
		return NULL;
	memcpy(ret, s, len + 1);
	return ret;
}
