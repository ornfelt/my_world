#include "_syscall.h"

#include <sys/utsname.h>

int
uname(struct utsname *buf)
{
	return syscall1(SYS_uname, (uintptr_t)buf);
}
