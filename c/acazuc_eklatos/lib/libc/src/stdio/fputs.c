#include <string.h>
#include <stdio.h>

int fputs(const char *s, FILE *fp)
{
	size_t len = strlen(s);
	if (fwrite(s, 1, len, fp) != len)
		return EOF;
	return len;
}
