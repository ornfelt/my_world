#include "_pwd.h"

#include <stdlib.h>
#include <errno.h>
#include <pwd.h>

struct passwd *
fgetpwent(FILE *fp)
{
	char *line = NULL;
	size_t line_size = 0;
	ssize_t ret;

	ret = getline(&line, &line_size, fp);
	if (ret < 0 && errno)
	{
		errno = ret;
		free(line);
		return NULL;
	}
	ret = parse_pwline(&pwd_ent, pwd_buf, sizeof(pwd_buf), line);
	if (ret)
	{
		errno = ret;
		free(line);
		return NULL;
	}
	return &pwd_ent;
}
