#include <stdio.h>

char *fgets(char *s, int size, FILE *fp)
{
	flockfile(fp);
	char *ret = fgets_unlocked(s, size, fp);
	funlockfile(fp);
	return ret;
}
