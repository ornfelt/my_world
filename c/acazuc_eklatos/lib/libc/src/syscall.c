#include "_syscall.h"

#include <sys/syscall.h>

#include <stdarg.h>

ssize_t syscall(size_t id, ...)
{
	va_list va_arg;
	va_start(va_arg, id);
	uintptr_t arg1 = va_arg(va_arg, uintptr_t);
	uintptr_t arg2 = va_arg(va_arg, uintptr_t);
	uintptr_t arg3 = va_arg(va_arg, uintptr_t);
	uintptr_t arg4 = va_arg(va_arg, uintptr_t);
	uintptr_t arg5 = va_arg(va_arg, uintptr_t);
	uintptr_t arg6 = va_arg(va_arg, uintptr_t);
	ssize_t ret = syscall6(id, arg1, arg2, arg3, arg4, arg5, arg6);
	va_end(va_arg);
	return ret;
}
