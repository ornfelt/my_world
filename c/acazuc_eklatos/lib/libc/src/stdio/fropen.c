#include <stdio.h>

FILE *
fropen(void *cookie, int (*readfn)(void *, char *, int))
{
	return funopen(cookie, readfn, NULL, NULL, NULL);
}
