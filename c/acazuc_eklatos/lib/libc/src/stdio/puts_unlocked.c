#include <string.h>
#include <stdio.h>

int puts_unlocled(const char *s)
{
	size_t len = strlen(s);
	if (fwrite_unlocked(s, 1, len, stdout) != len)
		return EOF;
	static const char nl = '\n';
	if (fwrite_unlocked(&nl, 1, 1, stdout) != 1)
		return EOF;
	return len + 1;
}
