#include "_vexec.h"

#include <unistd.h>

int
execlp(const char *file, const char *arg, ...)
{
	va_list ap;
	int ret;

	va_start(ap, arg);
	ret = vexec(file, 1, arg, ap);
	va_end(ap);
	return ret;
}
