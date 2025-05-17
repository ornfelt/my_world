#include "_grp.h"

#include <stdlib.h>
#include <errno.h>
#include <grp.h>

struct group *
fgetgrent(FILE *fp)
{
	char *line = NULL;
	size_t line_size = 0;
	ssize_t res;

	res = getline(&line, &line_size, fp);
	if (res < 0 && errno)
	{
		errno = res;
		free(line);
		return NULL;
	}
	res = parse_grline(&grp_ent, grp_buf, sizeof(grp_buf), line);
	if (res)
	{
		errno = res;
		free(line);
		return NULL;
	}
	return &grp_ent;
}
