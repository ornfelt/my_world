#include <sys/stat.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

char *mkdtemp(char *template)
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
	int fd = mkdir(template, 0700);
	if (fd == -1)
		return NULL;
	close(fd);
	return template;
}
