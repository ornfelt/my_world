#include <stdlib.h>

int
atoi(const char *s)
{
	return strtol(s, NULL, 10);
}
