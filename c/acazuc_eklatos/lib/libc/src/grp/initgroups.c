#include <grp.h>

int
initgroups(const char *user, gid_t group)
{
	(void)user;
	(void)group;
	/* XXX */
	return 0;
}
