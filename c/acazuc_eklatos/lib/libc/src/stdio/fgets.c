#include <stdio.h>

char *
fgets(char *s, int size, FILE *fp)
{
	char *ret;

	flockfile(fp);
	ret = fgets_unlocked(s, size, fp);
	funlockfile(fp);
	return ret;
}
