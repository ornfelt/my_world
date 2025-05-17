#include <string.h>

int
strcoll(const char *s1, const char *s2)
{
	/* XXX use locale */
	return strcmp(s1, s2);
}
