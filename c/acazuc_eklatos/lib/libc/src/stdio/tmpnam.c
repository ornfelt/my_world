#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char *tmpnam(char *s)
{
	/* XXX better */
	static char tmpbuf[1024];
	if (!s)
		s = tmpbuf;
	memcpy(s, "/tmp/tmp_", 9);
	for (size_t i = 9; i < 15; ++i)
		s[i] = (rand() % 10) + '0';
	s[15] = '\0';
	return s;
}
