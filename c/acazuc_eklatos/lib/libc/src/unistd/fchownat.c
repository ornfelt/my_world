#include "../_syscall.h"

#include <unistd.h>

int fchownat(int dirfd, const char *pathname, uid_t uid, gid_t gid, int flags)
{
	return syscall5(SYS_fchownat, dirfd, (uintptr_t)pathname, uid, gid, flags);
}
