#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

int
mkstemp(char *template)
{
	size_t len;
	int fd;

	len = strlen(template);
	if (len < 6 || strcmp(&template[len - 6], "XXXXXX"))
	{
		errno = EINVAL;
		return -1;
	}
	/* XXX better */
	for (size_t i = 0; i < 6; ++i)
		template[len - 6 + i] = 'a' + rand() % ('z' - 'a');
	fd = open(template, O_CREAT | O_EXCL, 0600);
	if (fd == -1)
	{
		template[0] = '\0';
		return -1;
	}
	return fd;
}
