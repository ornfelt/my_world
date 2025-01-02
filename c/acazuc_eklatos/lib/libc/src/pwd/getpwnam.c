#include "_pwd.h"

#include <errno.h>
#include <pwd.h>

struct passwd *getpwnam(const char *name)
{
	struct passwd *result;
	int res = getpwnam_r(name, &pwd_ent, pwd_buf, sizeof(pwd_buf), &result);
	if (!result)
		errno = res;
	return result;
}
