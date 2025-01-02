#include "../_syscall.h"

#include <stdarg.h>
#include <fcntl.h>

int fcntl(int fd, int cmd, ...)
{
	va_list va_arg;
	va_start(va_arg, cmd);
	int res = syscall3(SYS_fcntl, fd, cmd, va_arg(va_arg, uintptr_t));
	va_end(va_arg);
	return res;
}
