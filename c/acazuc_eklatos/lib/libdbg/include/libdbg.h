#ifndef LIBDBG_H
#define LIBDBG_H

#include <sys/types.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum dbg_syscall_arg_type
{
	DBG_SYSCALL_ARG_INT,
	DBG_SYSCALL_ARG_PID,
	DBG_SYSCALL_ARG_PGID,
	DBG_SYSCALL_ARG_PTR,
	DBG_SYSCALL_ARG_FD,
	DBG_SYSCALL_ARG_SIGNAL,
	DBG_SYSCALL_ARG_WSTATUS,
	DBG_SYSCALL_ARG_DIRFD,
	DBG_SYSCALL_ARG_PATH,
	DBG_SYSCALL_ARG_STRA,
	DBG_SYSCALL_ARG_AT_FLAGS,
	DBG_SYSCALL_ARG_SIGACTION,
	DBG_SYSCALL_ARG_PTRACE_REQUEST,
	DBG_SYSCALL_ARG_SIGHOW,
	DBG_SYSCALL_ARG_SIGSET,
	DBG_SYSCALL_ARG_PRIO_WHICH,
	DBG_SYSCALL_ARG_PRIO_ID,
	DBG_SYSCALL_ARG_RLIMIT_RES,
	DBG_SYSCALL_ARG_RLIMIT,
	DBG_SYSCALL_ARG_RUSAGE_WHO,
	DBG_SYSCALL_ARG_RUSAGE,
	DBG_SYSCALL_ARG_TMS,
	DBG_SYSCALL_ARG_CLOCK,
	DBG_SYSCALL_ARG_OPEN_FLAGS,
	DBG_SYSCALL_ARG_MODE,
	DBG_SYSCALL_ARG_IOV,
	DBG_SYSCALL_ARG_RDEV,
	DBG_SYSCALL_ARG_UID,
	DBG_SYSCALL_ARG_GID,
	DBG_SYSCALL_ARG_STAT,
	DBG_SYSCALL_ARG_OFF,
	DBG_SYSCALL_ARG_WHENCE,
	DBG_SYSCALL_ARG_FDS,
	DBG_SYSCALL_ARG_IOCTL_REQ,
	DBG_SYSCALL_ARG_DIRENT,
	DBG_SYSCALL_ARG_ULONG,
	DBG_SYSCALL_ARG_TIMES,
	DBG_SYSCALL_ARG_MOUNT_FLAGS,
	DBG_SYSCALL_ARG_STATVFS,
	DBG_SYSCALL_ARG_FCNTL_CMD,
	DBG_SYSCALL_ARG_STR,
	DBG_SYSCALL_ARG_KMLOAD_FLAGS,
	DBG_SYSCALL_ARG_KMUNLOAD_FLAGS,
	DBG_SYSCALL_ARG_FDSET,
	DBG_SYSCALL_ARG_TIMEVAL,
	DBG_SYSCALL_ARG_POLLFD,
	DBG_SYSCALL_ARG_UINT,
	DBG_SYSCALL_ARG_GIDLIST,
	DBG_SYSCALL_ARG_SOCK_FAMILY,
	DBG_SYSCALL_ARG_SOCK_TYPE,
	DBG_SYSCALL_ARG_SOCK_PROTO,
	DBG_SYSCALL_ARG_SOCKADDR,
	DBG_SYSCALL_ARG_ULONGP,
	DBG_SYSCALL_ARG_MSGHDR,
	DBG_SYSCALL_ARG_MSG_FLAGS,
	DBG_SYSCALL_ARG_SOCK_LEVEL,
	DBG_SYSCALL_ARG_SOCK_OPT,
	DBG_SYSCALL_ARG_SHUTDOWN_HOW,
	DBG_SYSCALL_ARG_TIME,
	DBG_SYSCALL_ARG_PROT,
	DBG_SYSCALL_ARG_MMAP_FLAGS,
	DBG_SYSCALL_ARG_MSYNC,
	DBG_SYSCALL_ARG_TIMESPEC,
	DBG_SYSCALL_ARG_CLOCKID,
	DBG_SYSCALL_ARG_UTSNAME,
	DBG_SYSCALL_ARG_INTP,
	DBG_SYSCALL_ARG_FUTEX_OP,
	DBG_SYSCALL_ARG_IPC_FLAGS,
	DBG_SYSCALL_ARG_SHMID,
	DBG_SYSCALL_ARG_SHM_FLAGS,
	DBG_SYSCALL_ARG_SHM_CMD,
	DBG_SYSCALL_ARG_KEY,
	DBG_SYSCALL_ARG_SEMID,
	DBG_SYSCALL_ARG_SEMBUF,
	DBG_SYSCALL_ARG_SEM_CMD,
	DBG_SYSCALL_ARG_MSGID,
	DBG_SYSCALL_ARG_MSG_CMD,
	DBG_SYSCALL_ARG_LONG,
	DBG_SYSCALL_ARG_WAIT_OPTIONS,
	DBG_SYSCALL_ARG_STACK,
	DBG_SYSCALL_ARG_ADVISE,
	DBG_SYSCALL_ARG_REBOOT_CMD,
	DBG_SYSCALL_ARG_IOVCNT,
	DBG_SYSCALL_ARG_NFDS,
};

enum dbg_syscall_arg_inout
{
	DBG_SYSCALL_ARG_IN,
	DBG_SYSCALL_ARG_OUT,
	DBG_SYSCALL_ARG_INOUT,
};

struct dbg_syscall_arg
{
	char name[16];
	enum dbg_syscall_arg_type type;
	enum dbg_syscall_arg_inout inout;
};

enum dbg_syscall_ret_type
{
	DBG_SYSCALL_RET_INT,
	DBG_SYSCALL_RET_PTR,
	DBG_SYSCALL_RET_PID,
	DBG_SYSCALL_RET_PGID,
	DBG_SYSCALL_RET_SID,
	DBG_SYSCALL_RET_FD,
	DBG_SYSCALL_RET_MODE,
	DBG_SYSCALL_RET_UID,
	DBG_SYSCALL_RET_GID,
	DBG_SYSCALL_RET_SSIZE,
	DBG_SYSCALL_RET_SHMID,
	DBG_SYSCALL_RET_SEMID,
	DBG_SYSCALL_RET_MSGID,
};

struct dbg_syscall
{
	char name[16];
	enum dbg_syscall_ret_type return_type;
	uint8_t params_nb;
	struct dbg_syscall_arg params[6];
};

struct dbg_signal
{
	char name[16];
};

struct dbg_errno
{
	char name[16];
};

const struct dbg_syscall *dbg_syscall_get(int syscall);
int dbg_syscall_from_name(const char *name);
const struct dbg_signal *dbg_signal_get(int signum);
int dbg_signal_from_name(const char *name);
const struct dbg_errno *dbg_errno_get(int err);
int dbg_errno_from_name(const char *name);

typedef int (*dbg_peekdata_fn_t)(void *buf,
                                 size_t size,
                                 uintptr_t addr,
                                 void *userptr);

int dbg_syscall_arg_print(char *buf,
                          size_t size,
                          const struct dbg_syscall *syscall,
                          const uintptr_t *values,
                          size_t param,
                          dbg_peekdata_fn_t peekdata,
                          void *userptr);
void dbg_syscall_ret_print(char *buf,
                           size_t size,
                           const struct dbg_syscall *syscall,
                           uintptr_t value);

#ifdef __cplusplus
}
#endif

#endif
