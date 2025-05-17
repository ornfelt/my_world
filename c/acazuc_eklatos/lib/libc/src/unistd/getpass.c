#include <unistd.h>
#include <stdio.h>

char *
getpass(const char *prompt)
{
	char *line = NULL;
	size_t n = 0;
	ssize_t len;

	fputs(prompt, stdout);
	/* XXX disable O_ECHO */
	len = getline(&line, &n, stdin);
	/* XXX enable O_ECHO */
	if (len < 0)
		return NULL;
	return line;
}
