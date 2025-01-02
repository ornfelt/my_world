#include "../_syscall.h"

#include <unistd.h>
#include <fcntl.h>

int fchown(int fd, uid_t uid, gid_t gid)
{
	return syscall5(SYS_fchownat, fd, (uintptr_t)"", uid, gid, AT_EMPTY_PATH);
}
