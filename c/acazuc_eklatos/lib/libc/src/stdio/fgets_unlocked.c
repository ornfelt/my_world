#include <stdio.h>

char *
fgets_unlocked(char *s, int size, FILE *fp)
{
	char *d;

	if (size <= 0)
		return NULL;
	if (size == 1)
	{
		*s = '\0';
		return s;
	}
	size--;
	d = s;
	while (size && fread_unlocked(d, 1, 1, fp))
	{
		if (*d == '\n')
		{
			d[1] = '\0';
			return s;
		}
		d++;
		size--;
	}
	if (size && ferror_unlocked(fp))
		return NULL;
	if (d == s)
		return NULL;
	*d = '\0';
	return s;
}
