#include <stdio.h>

ssize_t getline(char **lineptr, size_t *n, FILE *fp)
{
	return getdelim(lineptr, n, '\n', fp);
}
