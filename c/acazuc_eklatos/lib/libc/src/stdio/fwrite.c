#include <stdio.h>

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *fp)
{
	flockfile(fp);
	size_t ret = fwrite_unlocked(ptr, size, nmemb, fp);
	funlockfile(fp);
	return ret;
}
