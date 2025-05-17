#include "_vexec.h"

#include <unistd.h>

int
execl(const char *pathname, const char *arg, ...)
{
	va_list ap;
	int ret;

	va_start(ap, arg);
	ret = vexec(pathname, 0, arg, ap);
	va_end(ap);
	return ret;
}
