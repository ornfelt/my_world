#include "_syscall.h"

#include <sys/ptrace.h>

#include <stdarg.h>

long
ptrace(enum __ptrace_request request, ...)
{
	va_list va_arg;
	va_start(va_arg, request);
	pid_t pid = va_arg(va_arg, pid_t);
	uintptr_t addr = va_arg(va_arg, uintptr_t);
	if (request == PTRACE_PEEKDATA)
	{
		uintptr_t v;
		long ret = syscall4(SYS_ptrace, request, pid, addr,
		                    (uintptr_t)&v);
		va_end(va_arg);
		if (ret)
			return ret;
		return v;
	}
	long ret = syscall4(SYS_ptrace, request, pid, addr,
	                    va_arg(va_arg, uintptr_t));
	va_end(va_arg);
	return ret;
}
