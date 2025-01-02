#include "_pwd.h"

#include <stdlib.h>
#include <errno.h>
#include <pwd.h>

struct passwd *fgetpwent(FILE *fp)
{
	char *line = NULL;
	size_t line_size = 0;
	ssize_t res = getline(&line, &line_size, fp);
	if (res < 0 && errno)
	{
		errno = res;
		free(line);
		return NULL;
	}
	res = parse_pwline(&pwd_ent, pwd_buf, sizeof(pwd_buf), line);
	if (res)
	{
		errno = res;
		free(line);
		return NULL;
	}
	return &pwd_ent;
}
