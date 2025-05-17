#include <string.h>
#include <stdio.h>

int
puts(const char *s)
{
	static const char nl = '\n';
	int ret = EOF;
	size_t len;

	len = strlen(s);
	flockfile(stdout);
	if (fwrite(s, 1, len, stdout) != len)
		goto end;
	if (fwrite(&nl, 1, 1, stdout) != 1)
		goto end;
	ret = len + 1;

end:
	funlockfile(stdout);
	return ret;
}
