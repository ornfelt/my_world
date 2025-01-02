#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <eklat/syscall.h>

#include <sys/syscall.h>
#include <sys/types.h>

#include <errno.h>

static inline ssize_t convert_errno(ssize_t v)
{
	if (v < 0 && v >= -4095)
	{
		errno = -v;
		return -1;
	}
	return v;
}

static inline ssize_t syscall0(uintptr_t id)
{
	return convert_errno(syscall0_raw(id));
}

static inline ssize_t syscall1(uintptr_t id, uintptr_t arg1)
{
	return convert_errno(syscall1_raw(id, arg1));
}

static inline ssize_t syscall2(uintptr_t id, uintptr_t arg1, uintptr_t arg2)
{
	return convert_errno(syscall2_raw(id, arg1, arg2));
}

static inline ssize_t syscall3(uintptr_t id, uintptr_t arg1, uintptr_t arg2,
                               uintptr_t arg3)
{
	return convert_errno(syscall3_raw(id, arg1, arg2, arg3));
}

static inline ssize_t syscall4(uintptr_t id, uintptr_t arg1, uintptr_t arg2,
                               uintptr_t arg3, uintptr_t arg4)
{
	return convert_errno(syscall4_raw(id, arg1, arg2, arg3, arg4));
}

static inline ssize_t syscall5(uintptr_t id, uintptr_t arg1, uintptr_t arg2,
                               uintptr_t arg3, uintptr_t arg4, uintptr_t arg5)
{
	return convert_errno(syscall5_raw(id, arg1, arg2, arg3, arg4, arg5));
}

static inline ssize_t syscall6(uintptr_t id, uintptr_t arg1,
                               uintptr_t arg2, uintptr_t arg3,
                               uintptr_t arg4, uintptr_t arg5,
                               uintptr_t arg6)
{
	return convert_errno(syscall6_raw(id, arg1, arg2, arg3, arg4, arg5, arg6));
}

#endif
