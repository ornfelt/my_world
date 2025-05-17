#include "_syscall.h"

#include <sys/mount.h>

int
mount(const char *source,
      const char *target,
      const char *type,
      unsigned long flags,
      const void *data)
{
	return syscall5(SYS_mount,
	                (uintptr_t)source,
	                (uintptr_t)target,
	                (uintptr_t)type,
	                flags,
	                (uintptr_t)data);
}
