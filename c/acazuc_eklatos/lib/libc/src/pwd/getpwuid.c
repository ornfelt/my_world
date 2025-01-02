#include "_pwd.h"

#include <errno.h>
#include <pwd.h>

struct passwd *getpwuid(uid_t uid)
{
	struct passwd *result;
	int res = getpwuid_r(uid, &pwd_ent, pwd_buf, sizeof(pwd_buf), &result);
	if (!result)
		errno = res;
	return result;
}
