#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

char *mktemp(char *template)
{
	size_t len = strlen(template);
	if (len < 6 || strcmp(&template[len - 6], "XXXXXX"))
	{
		errno = EINVAL;
		return NULL;
	}
	/* XXX better */
	for (size_t i = 0; i < 6; ++i)
		template[len - 6 + i] = 'a' + rand() % ('z' - 'a');
	int fd = open(template, O_CREAT | O_EXCL, 0666);
	if (fd == -1)
	{
		template[0] = '\0';
		return template;
	}
	/* XXX cleanup at exit */
	close(fd);
	return template;
}
