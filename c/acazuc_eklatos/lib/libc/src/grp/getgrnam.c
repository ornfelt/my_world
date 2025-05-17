#include "_grp.h"

#include <errno.h>
#include <grp.h>

struct group *
getgrnam(const char *name)
{
	struct group *result;
	int ret;

	ret = getgrnam_r(name, &grp_ent, grp_buf, sizeof(grp_buf), &result);
	if (!result)
		errno = ret;
	return result;
}
