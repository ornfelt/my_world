#include "_grp.h"

#include <grp.h>

static int
cmp_gid(struct group *grp, const void *ptr)
{
	return grp->gr_gid != (gid_t)(intptr_t)ptr;
}

int
getgrgid_r(gid_t gid,
           struct group *grp,
           char *buf,
           size_t buflen,
           struct group **result)
{
	return search_grnam(grp,
	                    buf,
	                    buflen,
	                    result,
	                    cmp_gid,
	                    (const void*)(intptr_t)gid);
}
