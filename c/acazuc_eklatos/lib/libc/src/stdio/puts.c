#include <string.h>
#include <stdio.h>

int puts(const char *s)
{
	int ret = EOF;
	size_t len = strlen(s);
	flockfile(stdout);
	if (fwrite(s, 1, len, stdout) != len)
		goto end;
	static const char nl = '\n';
	if (fwrite(&nl, 1, 1, stdout) != 1)
		goto end;
	ret = len + 1;

end:
	funlockfile(stdout);
	return ret;
}
