#include "_pwd.h"

#include <pwd.h>

static int
cmp_uid(struct passwd *pwd, const void *ptr)
{
	return pwd->pw_uid != (uid_t)(intptr_t)ptr;
}

int
getpwuid_r(uid_t uid,
           struct passwd *pwd,
           char *buf,
           size_t buflen,
           struct passwd **result)
{
	return search_pwnam(pwd,
	                    buf,
	                    buflen,
	                    result,
	                    cmp_uid,
	                    (const void*)(intptr_t)uid);
}
