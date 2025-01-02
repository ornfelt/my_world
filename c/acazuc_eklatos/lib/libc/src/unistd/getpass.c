#include <unistd.h>
#include <stdio.h>

char *getpass(const char *prompt)
{
	char *line = NULL;
	size_t n = 0;
	fputs(prompt, stdout);
	/* XXX disable O_ECHO */
	ssize_t len = getline(&line, &n, stdin);
	/* XXX enable O_ECHO */
	if (len < 0)
		return NULL;
	return line;
}
