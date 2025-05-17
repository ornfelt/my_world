#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

ssize_t
getdelim(char **lineptr, size_t *n, int delim, FILE *fp)
{
	size_t rd = 0;
	int c;

	if (lineptr == NULL || n == NULL)
	{
		errno = EINVAL;
		return -1;
	}
	flockfile(fp);
	do
	{
		if (*n <= rd + 1)
		{
			char *newptr = realloc(*lineptr, *n + 1024);
			if (!newptr)
			{
				funlockfile(fp);
				return -1;
			}
			*lineptr = newptr;
			*n += 1024;
		}
		c = fgetc_unlocked(fp);
		if (c == EOF)
		{
			if (rd)
				break;
			(*lineptr)[rd] = '\0';
			errno = 0;
			funlockfile(fp);
			return -1;
		}
		(*lineptr)[rd] = c;
		rd++;
	} while (c != delim);
	funlockfile(fp);
	(*lineptr)[rd] = '\0';
	return rd;
}
