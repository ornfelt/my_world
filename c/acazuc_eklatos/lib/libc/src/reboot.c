#include "_syscall.h"

#include <eklat/reboot.h>

int reboot(uint32_t cmd)
{
	return syscall1(SYS_reboot, cmd);
}
