#include "_grp.h"

#include <errno.h>
#include <grp.h>

struct group *getgrgid(gid_t gid)
{
	struct group *result;
	int res = getgrgid_r(gid, &grp_ent, grp_buf, sizeof(grp_buf), &result);
	if (!result)
		errno = res;
	return result;
}
