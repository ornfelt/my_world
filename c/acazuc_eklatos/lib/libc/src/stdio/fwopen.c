#include <stdio.h>

FILE *
fwopen(void *cookie, int (*writefn)(void *, const char *, int))
{
	return funopen(cookie, NULL, writefn, NULL, NULL);
}
