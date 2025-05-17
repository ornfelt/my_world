#include <string.h>
#include <stdio.h>
#include <errno.h>

void
perror(const char *s)
{
	if (s && *s)
		fprintf(stderr, "%s: %s\n", s, strerror(errno));
	else
		fprintf(stderr, "%s\n", strerror(errno));
}
