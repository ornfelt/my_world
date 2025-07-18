#include "../_syscall.h"

#include <sys/sem.h>

#include <stdarg.h>

int
semctl(int semid, int semnum, int cmd, ...)
{
	va_list va_arg;
	int ret;

	va_start(va_arg, cmd);
	ret = syscall4(SYS_semctl,
	               semid,
	               semnum,
	               cmd,
	               va_arg(va_arg, uintptr_t));
	va_end(va_arg);
	return ret;
}
