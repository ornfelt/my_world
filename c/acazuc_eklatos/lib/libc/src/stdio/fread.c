#include <stdio.h>

size_t
fread(void *ptr, size_t size, size_t nmemb, FILE *fp)
{
	size_t ret;

	flockfile(fp);
	ret = fread_unlocked(ptr, size, nmemb, fp);
	funlockfile(fp);
	return ret;
}
