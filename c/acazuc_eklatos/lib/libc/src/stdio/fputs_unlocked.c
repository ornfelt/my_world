#include <string.h>
#include <stdio.h>

int fputs_unlocked(const char *s, FILE *fp)
{
	size_t len = strlen(s);
	if (fwrite_unlocked(s, 1, len, fp) != len)
		return EOF;
	return len;
}
