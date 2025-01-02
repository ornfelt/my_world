#include "../_syscall.h"

#include <unistd.h>
#include <fcntl.h>

int chown(const char *pathname, uid_t uid, gid_t gid)
{
	return syscall5(SYS_fchownat, AT_FDCWD, (uintptr_t)pathname, uid, gid, 0);
}
