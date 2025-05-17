#include "_pwd.h"

#include <string.h>
#include <pwd.h>

static int
cmp_name(struct passwd *pwd, const void *ptr)
{
	return strcmp(pwd->pw_name, (const char*)ptr);
}

int
getpwnam_r(const char *name,
           struct passwd *pwd,
           char *buf,
           size_t buflen,
           struct passwd **result)
{
	return search_pwnam(pwd, buf, buflen, result, cmp_name, name);
}
