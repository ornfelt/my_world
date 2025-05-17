#include <string.h>
#include <stdlib.h>

char *
strndup(const char *s, size_t n)
{
	size_t len;
	char *ret;

	len = strnlen(s, n);
	ret = malloc(len + 1);
	if (!ret)
		return NULL;
	memcpy(ret, s, len);
	ret[len] = '\0';
	return ret;
}
