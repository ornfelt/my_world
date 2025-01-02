#include "_grp.h"

#include <string.h>
#include <grp.h>

static int cmp_name(struct group *grp, const void *ptr)
{
	return strcmp(grp->gr_name, (const char*)ptr);
}

int getgrnam_r(const char *name, struct group *grp, char *buf,
               size_t buflen, struct group **result)
{
	return search_grnam(grp, buf, buflen, result, cmp_name, name);
}
