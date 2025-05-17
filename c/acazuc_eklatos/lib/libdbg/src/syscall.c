#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <sys/ptrace.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/futex.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/un.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <eklat/reboot.h>

#include <inttypes.h>
#include <strings.h>
#include <libdbg.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <poll.h>

static void
pushstr(char **buf, size_t *size, const char *s)
{
	if (!*buf || !*size)
		return;
	while (*size > 1 && *s)
	{
		**buf = *(s++);
		(*buf)++;
		(*size)--;
	}
	**buf = '\0';
}

struct bitmask_value
{
	uintptr_t mask;
	char text[32];
};

#define BITMASK_VALUE(v) {v, #v}
#define BITMASK_END {0, ""}

static int
print_bitmask(char *buf,
              size_t size,
              const struct bitmask_value *values,
              uintptr_t value,
              int first)
{
	for (size_t i = 0;;  ++i)
	{
		const struct bitmask_value *v = &values[i];
		if (!v->text[0])
			break;
		if ((value & v->mask) != v->mask)
			continue;
		value &= ~v->mask;
		if (first)
			first = 0;
		else
			pushstr(&buf, &size, " | ");
		pushstr(&buf, &size, v->text);
	}
	if (value)
	{
		if (first)
			first = 0;
		else
			pushstr(&buf, &size, " | ");
		char tmp[32];
		snprintf(tmp, sizeof(tmp), "0x%zx", value);
		pushstr(&buf, &size, tmp);
	}
	else if (first)
	{
		pushstr(&buf, &size, "0x0");
		first = 0;
	}
	return first;
}

struct enum_value
{
	uintptr_t value;
	char text[32];
};

#define ENUM_VALUE(v) {v, #v}
#define ENUM_END {0, ""}

static void
print_enum(char *buf,
           size_t size,
           const struct enum_value *values,
           uintptr_t value)
{
	for (size_t i = 0;; ++i)
	{
		const struct enum_value *v = &values[i];
		if (!v->text[0])
			break;
		if (v->value != value)
			continue;
		snprintf(buf, size, v->text);
		return;
	}
	snprintf(buf, size, "0x%zx", value);
}

static int
read_str(char **buf,
         size_t *size,
         uintptr_t value,
         dbg_peekdata_fn_t peekdata,
         void *userptr)
{
	ssize_t i;
	uintptr_t tmp;

	if (!value)
	{
		pushstr(buf, size, "NULL");
		return 0;
	}
	pushstr(buf, size, "\"");
	for (i = -(value % sizeof(tmp)); i < 32; i += sizeof(tmp))
	{
		if (peekdata(&tmp, sizeof(tmp), value + i, userptr))
			return 1;
		char *ptr = (char*)&tmp;
		size_t len = sizeof(tmp);
		if (i < 0)
		{
			ptr -= i;
			len += i;
		}
		for (size_t j = 0; j < len; ++j)
		{
			if (!ptr[j])
				goto end;
			if (*size < 2)
				continue;
			char tmpbuf[5];
			if (isprint(ptr[j]))
			{
				tmpbuf[0] = ptr[j];
				tmpbuf[1] = '\0';
			}
			else
			{
				snprintf(tmpbuf,
				         sizeof(tmpbuf),
				         "\\x%02x",
				         (uint8_t)ptr[j]);
			}
			pushstr(buf, size, tmpbuf);
		}
	}
end:
	pushstr(buf, size, "\"");
	if (i == 32)
		pushstr(buf, size, "...");
	return 0;
}

static int
read_stra(char *buf,
          size_t size,
          uintptr_t value,
          dbg_peekdata_fn_t peekdata,
          void *userptr)
{
	size_t i;
	uintptr_t tmp;

	if (!value)
	{
		snprintf(buf, size, "NULL");
		return 0;
	}
	pushstr(&buf, &size, "[");
	for (i = 0; i < 32 * sizeof(tmp); i += sizeof(tmp))
	{
		if (peekdata(&tmp, sizeof(tmp), value + i, userptr))
			return 1;
		if (!tmp)
			break;
		if (i)
			pushstr(&buf, &size, ", ");
		if (read_str(&buf, &size, tmp, peekdata, userptr))
			return 1;
	}
	pushstr(&buf, &size, "]");
	return 0;
}

static void
print_rdev(char *buf, size_t size, dev_t rdev)
{
	snprintf(buf,
	         size,
	         "{%" PRIu32 ", %" PRIu32 "}",
	         (rdev >> 16) & 0xFFFF,
	         rdev & 0xFFFF);
}

static void
print_timespec(char *buf, size_t size, struct timespec *ts)
{
	snprintf(buf,
	         size,
	         "{tv_sec=%" PRId64 ", tv_nsec=%" PRId64 "}",
	         ts->tv_sec,
	         ts->tv_nsec);
}

static void
print_uid(char *buf, size_t size, uid_t uid)
{
	snprintf(buf, size, "%" PRId32, uid);
}

static void
print_gid(char *buf, size_t size, gid_t gid)
{
	snprintf(buf, size, "%" PRId32, gid);
}

static void
print_timeval(char *buf, size_t size, struct timeval *tv)
{
	snprintf(buf,
	         size,
	         "{tv_sec=%" PRId64 ", tv_usec=%" PRId64 "}",
	         tv->tv_sec,
	         tv->tv_usec);
}

static void
print_sigset(char *buf, size_t size, const sigset_t *sigset)
{
	if (buf && size)
		*buf = '\0';
	pushstr(&buf, &size, "[");
	int first = 1;
	for (size_t i = 0; i < NSIG; ++i)
	{
		if (sigismember(sigset, i) != 1)
			continue;
		if (first)
			first = 0;
		else
			pushstr(&buf, &size, ", ");
		const struct dbg_signal *sig = dbg_signal_get(i);
		if (sig)
			pushstr(&buf, &size, sig->name);
		else
			pushstr(&buf, &size, "SIGUNKNOWN");
	}
	pushstr(&buf, &size, "]");
}

static const struct enum_value
ptrace_request_values[] =
{
	ENUM_VALUE(PTRACE_TRACEME),
	ENUM_VALUE(PTRACE_PEEKTEXT),
	ENUM_VALUE(PTRACE_PEEKDATA),
	ENUM_VALUE(PTRACE_PEEKUSER),
	ENUM_VALUE(PTRACE_POKETEXT),
	ENUM_VALUE(PTRACE_POKEDATA),
	ENUM_VALUE(PTRACE_POKEUSER),
	ENUM_VALUE(PTRACE_GETREGS),
	ENUM_VALUE(PTRACE_GETFPREGS),
	ENUM_VALUE(PTRACE_SETREGS),
	ENUM_VALUE(PTRACE_SETFPREGS),
	ENUM_VALUE(PTRACE_CONT),
	ENUM_VALUE(PTRACE_SYSCALL),
	ENUM_VALUE(PTRACE_SINGLESTEP),
	ENUM_VALUE(PTRACE_KILL),
	ENUM_VALUE(PTRACE_ATTACH),
	ENUM_VALUE(PTRACE_DETACH),
	ENUM_VALUE(PTRACE_SEIZE),
	ENUM_VALUE(PTRACE_LISTEN),
	ENUM_VALUE(PTRACE_GETSIGINFO),
	ENUM_END
};

static const struct enum_value
sighow_values[] =
{
	ENUM_VALUE(SIG_BLOCK),
	ENUM_VALUE(SIG_UNBLOCK),
	ENUM_VALUE(SIG_SETMASK),
	ENUM_END
};

static const struct enum_value
prio_which_values[] =
{
	ENUM_VALUE(PRIO_PROCESS),
	ENUM_VALUE(PRIO_PGRP),
	ENUM_VALUE(PRIO_USER),
	ENUM_END
};

static const struct enum_value
rlimit_res_values[] =
{
	ENUM_VALUE(RLIMIT_AS),
	ENUM_VALUE(RLIMIT_CORE),
	ENUM_VALUE(RLIMIT_CPU),
	ENUM_VALUE(RLIMIT_DATA),
	ENUM_VALUE(RLIMIT_FSIZE),
	ENUM_VALUE(RLIMIT_MEMLOCK),
	ENUM_VALUE(RLIMIT_NOFILE),
	ENUM_VALUE(RLIMIT_NPROC),
	ENUM_VALUE(RLIMIT_RSS),
	ENUM_VALUE(RLIMIT_STACK),
	ENUM_END
};

static const struct enum_value
rusage_who_values[] =
{
	ENUM_VALUE(RUSAGE_SELF),
	ENUM_VALUE(RUSAGE_CHILDREN),
	ENUM_VALUE(RUSAGE_THREAD),
	ENUM_END
};

static const struct bitmask_value
mode_values[] =
{
	BITMASK_VALUE(S_ISUID),
	BITMASK_VALUE(S_ISGID),
	BITMASK_VALUE(S_ISVTX),
	BITMASK_VALUE(S_IRUSR),
	BITMASK_VALUE(S_IWUSR),
	BITMASK_VALUE(S_IXUSR),
	BITMASK_VALUE(S_IRGRP),
	BITMASK_VALUE(S_IWGRP),
	BITMASK_VALUE(S_IXGRP),
	BITMASK_VALUE(S_IROTH),
	BITMASK_VALUE(S_IWOTH),
	BITMASK_VALUE(S_IXOTH),
	BITMASK_END
};

static const struct bitmask_value
at_flags_values[] =
{
	BITMASK_VALUE(AT_SYMLINK_NOFOLLOW),
	BITMASK_VALUE(AT_REMOVEDIR),
	BITMASK_VALUE(AT_SYMLINK_FOLLOW),
	BITMASK_VALUE(AT_EMPTY_PATH),
	BITMASK_VALUE(AT_EACCESS),
	BITMASK_END
};

static const struct bitmask_value
open_flags_values[] =
{
	BITMASK_VALUE(O_APPEND),
	BITMASK_VALUE(O_ASYNC),
	BITMASK_VALUE(O_CREAT),
	BITMASK_VALUE(O_TRUNC),
	BITMASK_VALUE(O_NOFOLLOW),
	BITMASK_VALUE(O_DIRECTORY),
	BITMASK_VALUE(O_EXCL),
	BITMASK_VALUE(O_NONBLOCK),
	BITMASK_VALUE(O_CLOEXEC),
	BITMASK_VALUE(O_NOCTTY),
	BITMASK_END
};

static const struct enum_value
whence_values[] =
{
	ENUM_VALUE(SEEK_SET),
	ENUM_VALUE(SEEK_CUR),
	ENUM_VALUE(SEEK_END),
	ENUM_END
};

static const struct enum_value
ioctl_req_values[] =
{
	ENUM_VALUE(TCGETS),
	ENUM_VALUE(TCSETS),
	ENUM_VALUE(TCSETSW),
	ENUM_VALUE(TCSETSF),
	ENUM_VALUE(TCSBRK),
	ENUM_VALUE(TCFLSH),
	ENUM_VALUE(TCXONC),
	ENUM_VALUE(TIOCGWINSZ),
	ENUM_VALUE(TIOCSWINSZ),
	ENUM_VALUE(TIOCSPTLCK),
	ENUM_VALUE(TIOCGPTLCK),
	ENUM_VALUE(TIOCGPTN),
	ENUM_VALUE(TIOCGPTPEER),
	ENUM_VALUE(TIOCGPGRP),
	ENUM_VALUE(TIOCSPGRP),
	ENUM_VALUE(TIOCGSID),
	ENUM_VALUE(SIOCGIFADDR),
	ENUM_VALUE(SIOCSIFADDR),
	ENUM_VALUE(SIOCDIFADDR),
	ENUM_VALUE(SIOCGIFNETMASK),
	ENUM_VALUE(SIOCSIFNETMASK),
	ENUM_VALUE(SIOCGIFHWADDR),
	ENUM_VALUE(SIOCSIFHWADDR),
	ENUM_VALUE(SIOCGIFCONF),
	ENUM_VALUE(SIOCGIFNAME),
	ENUM_VALUE(SIOCGIFFLAGS),
	ENUM_VALUE(SIOCSIFFLAGS),
	ENUM_VALUE(SIOCSGATEWAY),
	ENUM_VALUE(SIOCGGATEWAY),
	ENUM_END
};

static const struct enum_value
fcntl_cmd_values[] =
{
	ENUM_VALUE(F_DUPFD),
	ENUM_VALUE(F_GETFD),
	ENUM_VALUE(F_SETFD),
	ENUM_VALUE(F_GETFL),
	ENUM_VALUE(F_SETFL),
	ENUM_VALUE(F_SETLK),
	ENUM_VALUE(F_SETLKW),
	ENUM_VALUE(F_GETLK),
	ENUM_END
};

static const struct enum_value
sock_family_values[] =
{
	ENUM_VALUE(AF_UNSPEC),
	ENUM_VALUE(AF_UNIX),
	ENUM_VALUE(AF_LOCAL),
	ENUM_VALUE(AF_INET),
	ENUM_VALUE(AF_INET6),
	ENUM_VALUE(AF_PACKET),
	ENUM_END
};

static const struct enum_value
sock_type_values[] =
{
	ENUM_VALUE(SOCK_STREAM),
	ENUM_VALUE(SOCK_DGRAM),
	ENUM_VALUE(SOCK_RAW),
	ENUM_END
};

static const struct enum_value
msg_flags_values[] =
{
	ENUM_VALUE(MSG_DONTWAIT),
	ENUM_END
};

static const struct enum_value
sock_level_values[] =
{
	ENUM_VALUE(SOL_SOCKET),
	ENUM_END
};

static const struct bitmask_value
prot_values[] =
{
	BITMASK_VALUE(PROT_EXEC),
	BITMASK_VALUE(PROT_READ),
	BITMASK_VALUE(PROT_WRITE),
	BITMASK_END
};

static const struct bitmask_value
mmap_flags_values[] =
{
	BITMASK_VALUE(MAP_ANONYMOUS),
	BITMASK_VALUE(MAP_SHARED),
	BITMASK_VALUE(MAP_PRIVATE),
	BITMASK_VALUE(MAP_FIXED),
	BITMASK_VALUE(MAP_FIXED_NOREPLACE),
	BITMASK_VALUE(MAP_POPULATE),
	BITMASK_END
};

static const struct enum_value
shutdown_how_values[] =
{
	ENUM_VALUE(SHUT_RD),
	ENUM_VALUE(SHUT_WR),
	ENUM_VALUE(SHUT_RDWR),
	ENUM_END
};

static const struct bitmask_value
msync_values[] =
{
	BITMASK_VALUE(MS_ASYNC),
	BITMASK_VALUE(MS_SYNC),
	BITMASK_VALUE(MS_INVALIDATE),
	BITMASK_END
};

static const struct bitmask_value
ipc_flags_values[] =
{
	BITMASK_VALUE(IPC_CREAT),
	BITMASK_VALUE(IPC_EXCL),
	BITMASK_VALUE(IPC_NOWAIT),
	BITMASK_END
};

static const struct bitmask_value
shm_flags_values[] =
{
	BITMASK_VALUE(SHM_RND),
	BITMASK_VALUE(SHM_RDONLY),
	BITMASK_END
};

static const struct enum_value
clockid_values[] =
{
	ENUM_VALUE(CLOCK_REALTIME),
	ENUM_VALUE(CLOCK_MONOTONIC),
	ENUM_END
};

static const struct enum_value
futex_op_values[] =
{
	ENUM_VALUE(FUTEX_WAIT),
	ENUM_VALUE(FUTEX_WAKE),
	ENUM_END
};

static const struct bitmask_value
wait_options_values[] =
{
	BITMASK_VALUE(WNOHANG),
	BITMASK_VALUE(WUNTRACED),
	BITMASK_VALUE(WCONTINUED),
	BITMASK_END
};

static const struct bitmask_value
statvfs_flag_values[] =
{
	BITMASK_VALUE(ST_RDONLY),
	BITMASK_VALUE(ST_NOSUID),
	BITMASK_VALUE(ST_NOEXEC),
	BITMASK_END
};

static const struct enum_value
statvfs_magic_values[] =
{
	ENUM_VALUE(DEVFS_MAGIC),
	ENUM_VALUE(RAMFS_MAGIC),
	ENUM_VALUE(SYSFS_MAGIC),
	ENUM_VALUE(PROCFS_MAGIC),
	ENUM_VALUE(TARFS_MAGIC),
	ENUM_VALUE(EXT2FS_MAGIC),
	ENUM_VALUE(ISO9660_MAGIC),
	ENUM_END
};

static const struct enum_value
shm_cmd_values[] =
{
	ENUM_VALUE(IPC_STAT),
	ENUM_VALUE(IPC_SET),
	ENUM_VALUE(IPC_RMID),
	ENUM_END
};

static const struct enum_value
sem_cmd_values[] =
{
	ENUM_VALUE(IPC_STAT),
	ENUM_VALUE(IPC_SET),
	ENUM_VALUE(IPC_RMID),
	ENUM_VALUE(GETALL),
	ENUM_VALUE(GETNCNT),
	ENUM_VALUE(GETPID),
	ENUM_VALUE(GETVAL),
	ENUM_VALUE(GETZCNT),
	ENUM_VALUE(SETALL),
	ENUM_VALUE(SETVAL),
	ENUM_END
};

static const struct enum_value
msg_cmd_values[] =
{
	ENUM_VALUE(IPC_STAT),
	ENUM_VALUE(IPC_SET),
	ENUM_VALUE(IPC_RMID),
	ENUM_END
};

static const struct bitmask_value
sigaction_flags_values[] =
{
	BITMASK_VALUE(SA_NOCLDSTOP),
	BITMASK_VALUE(SA_NODEFER),
	BITMASK_VALUE(SA_ONSTACK),
	BITMASK_VALUE(SA_RESETHAND),
	BITMASK_VALUE(SA_RESTART),
	BITMASK_VALUE(SA_RESTORER),
	BITMASK_END
};

static const struct bitmask_value
advise_values[] =
{
	BITMASK_VALUE(MADV_NORMAL),
	BITMASK_VALUE(MADV_DONTNEED),
	BITMASK_END
};

static const struct enum_value
reboot_cmd_values[] =
{
	ENUM_VALUE(REBOOT_SHUTDOWN),
	ENUM_VALUE(REBOOT_REBOOT),
	ENUM_VALUE(REBOOT_SUSPEND),
	ENUM_VALUE(REBOOT_HIBERNATE),
	ENUM_END
};

int
dbg_syscall_arg_print(char *buf,
                      size_t size,
                      const struct dbg_syscall *syscall,
                      const uintptr_t *values,
                      size_t param,
                      dbg_peekdata_fn_t peekdata,
                      void *userptr)
{
	if (param >= syscall->params_nb)
		return -1;
	uintptr_t value = values[param];
	switch (syscall->params[param].type)
	{
		case DBG_SYSCALL_ARG_INT:
		case DBG_SYSCALL_ARG_PID:
		case DBG_SYSCALL_ARG_PGID:
		case DBG_SYSCALL_ARG_FD:
		case DBG_SYSCALL_ARG_MSGID:
		case DBG_SYSCALL_ARG_SEMID:
		case DBG_SYSCALL_ARG_SHMID:
		case DBG_SYSCALL_ARG_PRIO_ID:
		case DBG_SYSCALL_ARG_IOVCNT:
			snprintf(buf, size, "%ld", (long)value);
			break;
		case DBG_SYSCALL_ARG_UID:
			print_uid(buf, size, value);
			break;
		case DBG_SYSCALL_ARG_GID:
			print_gid(buf, size, value);
			break;
		case DBG_SYSCALL_ARG_ULONG:
			snprintf(buf, size, "%lu", (unsigned long)value);
			break;
		case DBG_SYSCALL_ARG_LONG:
			snprintf(buf, size, "%ld", (long)value);
			break;
		case DBG_SYSCALL_ARG_UINT:
		case DBG_SYSCALL_ARG_KEY:
		case DBG_SYSCALL_ARG_NFDS:
			snprintf(buf, size, "%u", (unsigned)value);
			break;
		case DBG_SYSCALL_ARG_PTR:
			if (value)
				snprintf(buf, size, "%p", (void*)value);
			else
				snprintf(buf, size, "NULL");
			break;
		case DBG_SYSCALL_ARG_DIRFD:
			if (value == (uintptr_t)AT_FDCWD)
				snprintf(buf, size, "AT_FDCWD");
			else
				snprintf(buf, size, "%ld", (long)value);
			break;
		case DBG_SYSCALL_ARG_RDEV:
			print_rdev(buf, size, value);
			break;
		case DBG_SYSCALL_ARG_SIGNAL:
		{
			const struct dbg_signal *def = dbg_signal_get(value);
			if (def)
				snprintf(buf, size, "%s", def->name);
			else
				snprintf(buf, size, "%d", (int)value);
			break;
		}
		case DBG_SYSCALL_ARG_PTRACE_REQUEST:
			print_enum(buf, size, ptrace_request_values, value);
			break;
		case DBG_SYSCALL_ARG_SIGHOW:
			print_enum(buf, size, sighow_values, value);
			break;
		case DBG_SYSCALL_ARG_PRIO_WHICH:
			print_enum(buf, size, prio_which_values, value);
			break;
		case DBG_SYSCALL_ARG_RLIMIT_RES:
			print_enum(buf, size, rlimit_res_values, value);
			break;
		case DBG_SYSCALL_ARG_RUSAGE_WHO:
			print_enum(buf, size, rusage_who_values, value);
			break;
		case DBG_SYSCALL_ARG_MODE:
			print_bitmask(buf, size, mode_values, value, 1);
			break;
		case DBG_SYSCALL_ARG_AT_FLAGS:
			print_bitmask(buf, size, at_flags_values, value, 1);
			break;
		case DBG_SYSCALL_ARG_IOV:
		{
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (param + 1 >= syscall->params_nb
			 || syscall->params[param + 1].type != DBG_SYSCALL_ARG_IOVCNT)
				return -1;
			int iovcnt = values[param + 1];
			pushstr(&buf, &size, "{");
			for (int i = 0; i < iovcnt; ++i)
			{
				struct iovec iov;
				if (peekdata(&iov, sizeof(iov), value, userptr))
					return -1;
				char tmp[1024];
				snprintf(tmp, sizeof(tmp), "{iov_base=%p,"
				                           " iov_len=%zu}",
				         iov.iov_base,
				         iov.iov_len);
				pushstr(&buf, &size, tmp);
			}
			pushstr(&buf, &size, "}");
			break;
		}
		case DBG_SYSCALL_ARG_RLIMIT:
		{
			struct rlimit v;
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&v, sizeof(v), value, userptr))
				return -1;
			snprintf(buf, size, "{rlim_cur=%" PRId64 ","
			                    " rlim_max=%" PRId64"}",
			         v.rlim_cur,
			         v.rlim_max);
			break;
		}
		case DBG_SYSCALL_ARG_SIGSET:
		{
			sigset_t v;
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&v, sizeof(v), value, userptr))
				return -1;
			print_sigset(buf, size, &v);
			break;
		}
		case DBG_SYSCALL_ARG_SIGACTION:
		{
			struct sigaction v;
			char sigset[512];
			char flags[128];
			char handler[32];
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&v, sizeof(v), value, userptr))
				return -1;
			if (v.sa_handler == SIG_ERR)
				snprintf(handler, sizeof(handler), "SIG_ERR");
			else if (v.sa_handler == SIG_IGN)
				snprintf(handler, sizeof(handler), "SIG_IGN");
			else if (v.sa_handler == SIG_DFL)
				snprintf(handler, sizeof(handler), "SIG_DFL");
			else
				snprintf(handler, sizeof(handler), "%p", v.sa_handler);
			print_sigset(sigset, sizeof(sigset), &v.sa_mask);
			print_bitmask(flags, sizeof(flags), sigaction_flags_values, v.sa_flags, 1);
			snprintf(buf, size, "{sa_handler=%s,"
			                    " sa_mask=%s,"
			                    " sa_flags=%s,"
			                    " sa_restorer=%p}",
			                    handler,
			                    sigset,
			                    flags,
			                    v.sa_restorer);
			break;
		}
		case DBG_SYSCALL_ARG_STATVFS:
		{
			struct statvfs v;
			char flag[128];
			char magic[32];
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&v, sizeof(v), value, userptr))
				return -1;
			print_bitmask(flag, sizeof(flag), statvfs_flag_values, value, 1);
			print_enum(magic, sizeof(magic), statvfs_magic_values, value);
			snprintf(buf, size, "{f_bsize=%lu,"
			                    " f_frsize=%lu,"
			                    " f_blocks=%" PRIu64 ","
			                    " f_bfree=%" PRIu64 ","
			                    " f_bavail=%" PRIu64 ","
			                    " f_files=%" PRIu64 ","
			                    " f_ffree=%" PRIu64 ","
			                    " f_favail=%" PRIu64 ","
			                    " f_fsid=%lu,"
			                    " f_flag=%s,"
			                    " f_namemax=%lu,"
			                    " f_magic=%s}",
			                    v.f_bsize,
			                    v.f_frsize,
			                    v.f_blocks,
			                    v.f_bfree,
			                    v.f_bavail,
			                    v.f_files,
			                    v.f_ffree,
			                    v.f_favail,
			                    v.f_fsid,
			                    flag,
			                    v.f_namemax,
			                    magic);
			break;
		}
		case DBG_SYSCALL_ARG_POLLFD:
		{
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (param + 1 >= syscall->params_nb
			 || syscall->params[param + 1].type != DBG_SYSCALL_ARG_NFDS)
				return -1;
			nfds_t nfds = values[param + 1];
			pushstr(&buf, &size, "{");
			for (nfds_t i = 0; i < nfds; ++i)
			{
				struct pollfd pfd;
				if (peekdata(&pfd, sizeof(pfd), value, userptr))
					return -1;
				char tmp[1024];
				snprintf(tmp, sizeof(tmp), "{fd=%d,"
				                           " events=%hd,"
				                           " revents=%hd}",
				         pfd.fd,
				         pfd.events,
				         pfd.revents);
				pushstr(&buf, &size, tmp);
			}
			pushstr(&buf, &size, "}");
			break;
		}
		case DBG_SYSCALL_ARG_GIDLIST:
			snprintf(buf, size, "%p", (void*)value); /* XXX */
			break;
		case DBG_SYSCALL_ARG_SEMBUF:
			snprintf(buf, size, "%p", (void*)value); /* XXX */
			break;
		case DBG_SYSCALL_ARG_TMS:
		{
			struct tms v;
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&v, sizeof(v), value, userptr))
				return -1;
			snprintf(buf, size, "{tms_utime=%zd,"
			                    " tms_stime=%zd,"
			                    " tms_cutime=%zd,"
			                    " tms_cstime=%zd}",
			                    v.tms_utime,
			                    v.tms_stime,
			                    v.tms_cutime,
			                    v.tms_cstime);
			break;
		}
		case DBG_SYSCALL_ARG_DIRENT:
			snprintf(buf, size, "%p", (void*)value); /* XXX */
			break;
		case DBG_SYSCALL_ARG_MOUNT_FLAGS:
			snprintf(buf, size, "%zx", value); /* XXX */
			break;
		case DBG_SYSCALL_ARG_KMLOAD_FLAGS:
			snprintf(buf, size, "%zx", value); /* XXX */
			break;
		case DBG_SYSCALL_ARG_KMUNLOAD_FLAGS:
			snprintf(buf, size, "%zx", value); /* XXX */
			break;
		case DBG_SYSCALL_ARG_SOCK_PROTO:
			snprintf(buf, size, "%d", (int)value); /* XXX */
			break;
		case DBG_SYSCALL_ARG_SOCK_OPT:
			snprintf(buf, size, "%d", (int)value); /* XXX */
			break;
		case DBG_SYSCALL_ARG_SHM_CMD:
			print_enum(buf, size, shm_cmd_values, value);
			break;
		case DBG_SYSCALL_ARG_SEM_CMD:
			print_enum(buf, size, sem_cmd_values, value);
			break;
		case DBG_SYSCALL_ARG_MSG_CMD:
			print_enum(buf, size, msg_cmd_values, value);
			break;
		case DBG_SYSCALL_ARG_OPEN_FLAGS:
		{
			int first = 1;
			switch (value & 0x3)
			{
				case O_RDONLY:
					pushstr(&buf, &size, "O_RDONLY");
					first = 0;
					break;
				case O_WRONLY:
					pushstr(&buf, &size, "O_WRONLY");
					first = 0;
					break;
				case O_RDWR:
					pushstr(&buf, &size, "O_RDWR");
					first = 0;
					break;
				default:
					pushstr(&buf, &size, "O_UNK");
					break;
			}
			print_bitmask(buf, size, open_flags_values, value & ~0x3, first);
			break;
		}
		case DBG_SYSCALL_ARG_WHENCE:
			print_enum(buf, size, whence_values, value);
			break;
		case DBG_SYSCALL_ARG_TIMES:
		{
			struct timespec ts[2];
			char t0[64];
			char t1[64];
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(ts, sizeof(ts), value, userptr))
				return -1;
			print_timespec(t0, sizeof(t0), &ts[0]);
			print_timespec(t1, sizeof(t1), &ts[1]);
			snprintf(buf, size, "[%s, %s]", t0, t1);
			break;
		}
		case DBG_SYSCALL_ARG_OFF:
		{
			off_t v;
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&v, sizeof(v), value, userptr))
				return -1;
			snprintf(buf, size, "{%" PRId64 "}", (int64_t)v);
			break;
		}
		case DBG_SYSCALL_ARG_IOCTL_REQ:
			print_enum(buf, size, ioctl_req_values, value);
			break;
		case DBG_SYSCALL_ARG_FCNTL_CMD:
			print_enum(buf, size, fcntl_cmd_values, value);
			break;
		case DBG_SYSCALL_ARG_FDSET:
		{
			fd_set fds;
			int first = 1;

			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&fds, sizeof(fds), value, userptr))
				return -1;
			if (buf && size)
				buf[0] = 0;
			for (int i = 0; i < FD_SETSIZE; ++i)
			{
				if (!FD_ISSET(i, &fds))
					continue;
				if (first)
					first = 0;
				else
					strlcat(buf, ", ", size);
				char tmp[32];
				snprintf(tmp, sizeof(tmp), "%d", i);
				strlcat(buf, tmp, size);
			}
			break;
		}
		case DBG_SYSCALL_ARG_TIMEVAL:
		{
			struct timeval tv;
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&tv, sizeof(tv), value, userptr))
				return -1;
			print_timeval(buf, size, &tv);
			break;
		}
		case DBG_SYSCALL_ARG_SOCK_FAMILY:
			print_enum(buf, size, sock_family_values, value);
			break;
		case DBG_SYSCALL_ARG_SOCK_TYPE:
			print_enum(buf, size, sock_type_values, value);
			break;
		case DBG_SYSCALL_ARG_MSG_FLAGS:
			print_enum(buf, size, msg_flags_values, value);
			break;
		case DBG_SYSCALL_ARG_SOCK_LEVEL:
			print_enum(buf, size, sock_level_values, value);
			break;
		case DBG_SYSCALL_ARG_PROT:
			print_bitmask(buf, size, prot_values, value, 1);
			break;
		case DBG_SYSCALL_ARG_MMAP_FLAGS:
			print_bitmask(buf, size, mmap_flags_values, value, 1);
			break;
		case DBG_SYSCALL_ARG_SHUTDOWN_HOW:
			print_enum(buf, size, shutdown_how_values, value);
			break;
		case DBG_SYSCALL_ARG_MSYNC:
			print_bitmask(buf, size, msync_values, value, 1);
			break;
		case DBG_SYSCALL_ARG_TIMESPEC:
		{
			struct timespec ts;
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&ts, sizeof(ts), value, userptr))
				return -1;
			print_timespec(buf, size, &ts);
			break;
		}
		case DBG_SYSCALL_ARG_IPC_FLAGS:
			print_bitmask(buf, size, ipc_flags_values, value, 1);
			break;
		case DBG_SYSCALL_ARG_SHM_FLAGS:
			print_bitmask(buf, size, shm_flags_values, value, 1);
			break;
		case DBG_SYSCALL_ARG_CLOCKID:
			print_enum(buf, size, clockid_values, value);
			break;
		case DBG_SYSCALL_ARG_FUTEX_OP:
			print_enum(buf, size, futex_op_values, value);
			break;
		case DBG_SYSCALL_ARG_CLOCK:
		{
			clock_t v;
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&v, sizeof(v), value, userptr))
				return -1;
			snprintf(buf, size, "{%" PRId64 "}", (int64_t)v);
			break;
		}
		case DBG_SYSCALL_ARG_MSGHDR:
		{
			struct msghdr msg;
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&msg, sizeof(msg), value, userptr))
				return -1;
			snprintf(buf, size, "{msg_name=%p, msg_namelen=%" PRIu32 ", msg_iov=%p,"
			         " msg_iovlen=%zu, msg_control=%p, msg_controllen=%zu,"
			         " msg_flags=%d}",
			         msg.msg_name, msg.msg_namelen, msg.msg_iov,
			         msg.msg_iovlen, msg.msg_control, msg.msg_controllen,
			         msg.msg_flags);
			break;
		}
		case DBG_SYSCALL_ARG_SOCKADDR:
		{
			sa_family_t family;
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&family, sizeof(family), value, userptr))
				return -1;
			switch (family)
			{
				case AF_INET:
				{
					struct sockaddr_in sin;
					if (peekdata(&sin, sizeof(sin), value, userptr))
						return -1;
					snprintf(buf, size, "{sin_family=AF_INET,"
					                    " sin_port=%" PRIu16 ","
					                    " sin_addr=%s}",
					         ntohs(sin.sin_port),
					         inet_ntoa(sin.sin_addr));
					break;
				}
				case AF_INET6:
				{
					struct sockaddr_in6 sin6;
					char tmp[64];
					if (peekdata(&sin6, sizeof(sin6), value, userptr))
						return -1;
					snprintf(buf, size, "{sin6_family=AF_INET6,"
					                    " sin6_port=%" PRIu16 ","
					                    " sin6_flowinfo=%" PRIu32 ","
					                    " sin6_addr=%s,"
					                    " sin6_scope_id=%" PRIu32 "}",
					         ntohs(sin6.sin6_port),
					         sin6.sin6_flowinfo,
					         inet_ntop(AF_INET6, &sin6.sin6_addr, tmp, sizeof(tmp)),
					         sin6.sin6_scope_id);
					break;
				}
				case AF_UNIX:
				{
					struct sockaddr_un sun;
					if (peekdata(&sun, sizeof(sun), value, userptr))
						return -1;
					snprintf(buf, size, "{sun_family=AF_UNIX, sun_path=%.*s}",
					         (int)sizeof(sun.sun_path), sun.sun_path);
					break;
				}
				default:
				{
					char name[32];
					print_enum(name, sizeof(name), sock_family_values, value);
					printf("{sa_family=%s}", name);
					break;
				}
			}
			break;
		}
		case DBG_SYSCALL_ARG_STR:
		case DBG_SYSCALL_ARG_PATH:
			if (buf && size)
				buf[0] = '\0';
			if (read_str(&buf, &size, value, peekdata, userptr))
				return -1;
			break;
		case DBG_SYSCALL_ARG_STRA:
			if (buf && size)
				buf[0] = '\0';
			if (read_stra(buf, size, value, peekdata, userptr))
				return -1;
			break;
		case DBG_SYSCALL_ARG_STAT:
		{
			struct stat st;
			char mode[64];
			char uid[64];
			char gid[64];
			char rdev[64];
			char atim[64];
			char mtim[64];
			char ctim[64];
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&st, sizeof(st), value, userptr))
				return -1;
			print_bitmask(mode, sizeof(mode), mode_values, st.st_mode, 1);
			print_uid(uid, sizeof(uid), st.st_uid);
			print_uid(gid, sizeof(gid), st.st_gid);
			print_rdev(rdev, sizeof(rdev), st.st_dev); 
			print_timespec(atim, sizeof(atim), &st.st_atim);
			print_timespec(mtim, sizeof(mtim), &st.st_mtim);
			print_timespec(ctim, sizeof(ctim), &st.st_ctim);
			snprintf(buf, size, "{st_dev=%" PRId32 ","
			                    " st_ino=%" PRId64 ","
			                    " st_mode=%s,"
			                    " st_nlink=%" PRIu32 ","
			                    " st_uid=%s,"
			                    " st_gid=%s,"
			                    " st_rdev=%s,"
			                    " st_size=%" PRId64 ","
			                    " st_blksize=%" PRId64 ","
			                    " st_blocks=%" PRId64 ","
			                    " st_atim=%s,"
			                    " st_mtim=%s,"
			                    " st_ctim=%s}",
			                    st.st_dev,
			                    st.st_ino,
			                    mode,
			                    st.st_nlink,
			                    uid,
			                    gid,
			                    rdev,
			                    st.st_size,
			                    st.st_blksize,
			                    st.st_blocks,
			                    atim,
			                    mtim,
			                    ctim);
			break;
		}
		case DBG_SYSCALL_ARG_TIME:
		{
			time_t v;
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&v, sizeof(v), value, userptr))
				return -1;
			snprintf(buf, size, "{%" PRId64 "}", (int64_t)v);
			break;
		}
		case DBG_SYSCALL_ARG_FDS:
		{
			int fds[2];
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&fds, sizeof(fds), value, userptr))
				return -1;
			snprintf(buf, size, "[%d, %d]", fds[0], fds[1]);
			break;
		}
		case DBG_SYSCALL_ARG_ULONGP:
		{
			unsigned long v;
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&v, sizeof(v), value, userptr))
				return -1;
			snprintf(buf, size, "{%lu}", v);
			break;
		}
		case DBG_SYSCALL_ARG_WSTATUS:
		{
			int wstatus;
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&wstatus, sizeof(wstatus), value, userptr))
				return -1;
			if (WIFEXITED(wstatus))
				snprintf(buf, size, "{WIFEXITED(s) && WEXITSTATUS(s) == %d}",
				         WEXITSTATUS(wstatus));
			else if (WIFSTOPPED(wstatus))
				snprintf(buf, size, "{WIFSTOPPED(s) && WSTOPSIG(s) == %d%s}",
				         WSTOPSIG(wstatus), (wstatus & 0x80) ? " | 0x80" : "");
			else if (WIFSIGNALED(wstatus))
				snprintf(buf, size, "{WIFSIGNALED(s) && WTERMSIG(s) == %d}",
				         WTERMSIG(wstatus));
			break;
		}
		case DBG_SYSCALL_ARG_INTP:
		{
			int v;
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&v, sizeof(v), value, userptr))
				return -1;
			snprintf(buf, size, "{%d}", v);
			break;
		}
		case DBG_SYSCALL_ARG_WAIT_OPTIONS:
			print_bitmask(buf, size, wait_options_values, value, 1);
			break;
		case DBG_SYSCALL_ARG_RUSAGE:
		{
			struct rusage v;
			char utime[64];
			char stime[64];
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&v, sizeof(v), value, userptr))
				return -1;
			print_timeval(utime, sizeof(utime), &v.ru_utime);
			print_timeval(stime, sizeof(stime), &v.ru_stime);
			snprintf(buf, size, "{ru_utime=%s,"
			                    " ru_stime=%s,"
			                    " ru_maxrss=%ld,"
			                    " ru_ixrss=%ld,"
			                    " ru_idrss=%ld,"
			                    " ru_isrss=%ld,"
			                    " ru_minflt=%ld,"
			                    " ru_majflt=%ld,"
			                    " ru_nswap=%ld,"
			                    " ru_inblock=%ld,"
			                    " ru_outblock=%ld,"
			                    " ru_msgsnd=%ld,"
			                    " ru_msgrcv=%ld,"
			                    " ru_nsignals=%ld,"
			                    " ru_nvcsw=%ld,"
			                    " ru_nivcsw=%ld}",
			                    utime,
			                    stime,
			                    v.ru_maxrss,
			                    v.ru_ixrss,
			                    v.ru_idrss,
			                    v.ru_isrss,
			                    v.ru_minflt,
			                    v.ru_majflt,
			                    v.ru_nswap,
			                    v.ru_inblock,
			                    v.ru_outblock,
			                    v.ru_msgsnd,
			                    v.ru_msgrcv,
			                    v.ru_nsignals,
			                    v.ru_nvcsw,
			                    v.ru_nivcsw);
			break;
		}
		case DBG_SYSCALL_ARG_UTSNAME:
		{
			struct utsname v;
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&v, sizeof(v), value, userptr))
				return -1;
			snprintf(buf, size, "{sysname=\"%s\","
			                    " nodename=\"%s\","
			                    " release=\"%s\","
			                    " version=\"%s\","
			                    " machine=\"%s\"}",
			                    v.sysname,
			                    v.nodename,
			                    v.release,
			                    v.version,
			                    v.machine);
			break;
		}
		case DBG_SYSCALL_ARG_STACK:
		{
			stack_t v;
			if (!value)
			{
				snprintf(buf, size, "NULL");
				break;
			}
			if (peekdata(&v, sizeof(v), value, userptr))
				return -1;
			snprintf(buf, size, "{ss_sp=%p, ss_flags=%x, ss_size=%zu}",
			         v.ss_sp, v.ss_flags, v.ss_size);
			break;
		}
		case DBG_SYSCALL_ARG_ADVISE:
			print_bitmask(buf, size, advise_values, value, 1);
			break;
		case DBG_SYSCALL_ARG_REBOOT_CMD:
			print_enum(buf, size, reboot_cmd_values, value);
			break;
	}
	return 0;
}

void
dbg_syscall_ret_print(char *buf,
                      size_t size,
                      const struct dbg_syscall *syscall,
                      uintptr_t value)
{
	switch (syscall->return_type)
	{
		case DBG_SYSCALL_RET_INT:
		case DBG_SYSCALL_RET_PID:
		case DBG_SYSCALL_RET_PGID:
		case DBG_SYSCALL_RET_SID:
		case DBG_SYSCALL_RET_FD:
		case DBG_SYSCALL_RET_MODE:
		case DBG_SYSCALL_RET_UID:
		case DBG_SYSCALL_RET_GID:
		case DBG_SYSCALL_RET_SSIZE:
		case DBG_SYSCALL_RET_SHMID:
		case DBG_SYSCALL_RET_SEMID:
		case DBG_SYSCALL_RET_MSGID:
			snprintf(buf, size, "%ld", (long)value);
			break;
		case DBG_SYSCALL_RET_PTR:
			snprintf(buf, size, "%p", (void*)value);
			break;
	}
}

static const struct dbg_syscall
syscalls[] =
{
	[SYS_exit]          = {"exit",          DBG_SYSCALL_RET_INT, 1,
	                     {{"code",          DBG_SYSCALL_ARG_INT,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_clone]         = {"clone",         DBG_SYSCALL_RET_INT, 1,
	                     {{"flags",         DBG_SYSCALL_ARG_INT,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_kill]          = {"kill",          DBG_SYSCALL_RET_INT, 2,
	                     {{"pid",           DBG_SYSCALL_ARG_PID,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"sig",           DBG_SYSCALL_ARG_SIGNAL,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_wait4]         = {"wait4",         DBG_SYSCALL_RET_PID, 3,
	                     {{"pid",           DBG_SYSCALL_ARG_PID,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"wstatus",       DBG_SYSCALL_ARG_WSTATUS,
	                                        DBG_SYSCALL_ARG_OUT},
	                      {"options",       DBG_SYSCALL_ARG_WAIT_OPTIONS,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"rusage",        DBG_SYSCALL_ARG_RUSAGE,
	                                        DBG_SYSCALL_ARG_OUT}}},

	[SYS_execveat]      = {"execveat",      DBG_SYSCALL_RET_INT, 5,
	                     {{"dirfd",         DBG_SYSCALL_ARG_DIRFD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"path",          DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"argv",          DBG_SYSCALL_ARG_STRA,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"envp",          DBG_SYSCALL_ARG_STRA,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_AT_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_getpid]        = {"getpid",        DBG_SYSCALL_RET_PID, 0},

	[SYS_setpgid]       = {"setpgid",       DBG_SYSCALL_RET_INT, 2,
	                     {{"pid",           DBG_SYSCALL_ARG_PID,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"pgid",          DBG_SYSCALL_ARG_PGID,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_getppid]       = {"getppid",       DBG_SYSCALL_RET_PID, 0},

	[SYS_getpgrp]       = {"getpgrp",       DBG_SYSCALL_RET_PGID, 0},

	[SYS_setsid]        = {"setsid",        DBG_SYSCALL_RET_INT, 0},

	[SYS_getpgid]       = {"getpgid",       DBG_SYSCALL_RET_PGID, 1,
	                     {{"pid",           DBG_SYSCALL_ARG_PID,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_getsid]        = {"getsid",        DBG_SYSCALL_RET_SID, 1,
	                     {{"pid",           DBG_SYSCALL_ARG_PID,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_sigaction]     = {"sigaction",     DBG_SYSCALL_RET_INT, 3,
	                     {{"signum",        DBG_SYSCALL_ARG_SIGNAL,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"action",        DBG_SYSCALL_ARG_SIGACTION,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"oldaction",     DBG_SYSCALL_ARG_SIGACTION,
	                                        DBG_SYSCALL_ARG_OUT}}},

	[SYS_sched_yield]   = {"sched_yield",   DBG_SYSCALL_RET_INT, 0},

	[SYS_ptrace]        = {"ptrace",        DBG_SYSCALL_RET_INT, 4,
	                     {{"request",       DBG_SYSCALL_ARG_PTRACE_REQUEST,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"pid",           DBG_SYSCALL_ARG_PID,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"addr",          DBG_SYSCALL_ARG_PTR,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"data",          DBG_SYSCALL_ARG_PTR,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_sigprocmask]   = {"sigprocmask",   DBG_SYSCALL_RET_INT, 3,
	                     {{"how",           DBG_SYSCALL_ARG_SIGHOW,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"set",           DBG_SYSCALL_ARG_SIGSET,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"oldset",        DBG_SYSCALL_ARG_SIGSET,
	                                        DBG_SYSCALL_ARG_OUT}}},

	[SYS_getpriority]   = {"getpriority",   DBG_SYSCALL_RET_INT, 2,
	                     {{"which",         DBG_SYSCALL_ARG_PRIO_WHICH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"who",           DBG_SYSCALL_ARG_PRIO_ID,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_setpriority]   = {"setpriority",   DBG_SYSCALL_RET_INT, 3,
	                     {{"which",         DBG_SYSCALL_ARG_PRIO_WHICH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"who",           DBG_SYSCALL_ARG_PRIO_ID,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"prio",          DBG_SYSCALL_ARG_INT,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_getrlimit]     = {"getrlimit",     DBG_SYSCALL_RET_INT, 2,
	                     {{"res",           DBG_SYSCALL_ARG_RLIMIT_RES,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"rlimit",        DBG_SYSCALL_ARG_RLIMIT,
	                                        DBG_SYSCALL_ARG_OUT}}},

	[SYS_setrlimit]     = {"setrlimit",     DBG_SYSCALL_RET_INT, 3,
	                     {{"res",           DBG_SYSCALL_ARG_RLIMIT_RES,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"rlimit",        DBG_SYSCALL_ARG_RLIMIT,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_getrusage]     = {"getrusage",     DBG_SYSCALL_RET_INT, 2,
	                     {{"who",           DBG_SYSCALL_ARG_RUSAGE_WHO,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"rusage",        DBG_SYSCALL_ARG_RUSAGE,
	                                        DBG_SYSCALL_ARG_OUT}}},

	[SYS_sigreturn]     = {"sigreturn",     DBG_SYSCALL_RET_INT, 0},

	[SYS_gettid]        = {"gettid",        DBG_SYSCALL_RET_PID, 0},

	[SYS_settls]        = {"settls",        DBG_SYSCALL_RET_INT, 1,
	                     {{"addr",          DBG_SYSCALL_ARG_PTR,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_gettls]        = {"gettls",        DBG_SYSCALL_RET_PTR, 0},

	[SYS_exit_group]    = {"exit_group",    DBG_SYSCALL_RET_INT, 1,
	                     {{"code",          DBG_SYSCALL_ARG_INT,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_times]         = {"times",         DBG_SYSCALL_RET_INT, 2,
	                     {{"tms",           DBG_SYSCALL_ARG_TMS,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"clk",           DBG_SYSCALL_ARG_CLOCK,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_sigaltstack]   = {"sigaltstack",   DBG_SYSCALL_RET_INT, 2,
	                     {{"ss",            DBG_SYSCALL_ARG_STACK,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"old_ss",        DBG_SYSCALL_ARG_STACK,
	                                        DBG_SYSCALL_ARG_OUT}}},

	[SYS_sigpending]    = {"sigpending",    DBG_SYSCALL_RET_INT, 1,
	                     {{"set",           DBG_SYSCALL_ARG_SIGSET,
	                                        DBG_SYSCALL_ARG_OUT}}},

	[SYS_openat]        = {"openat",        DBG_SYSCALL_RET_FD, 4,
	                     {{"dirfd",         DBG_SYSCALL_ARG_DIRFD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"path",          DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_OPEN_FLAGS,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"mode",          DBG_SYSCALL_ARG_MODE,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_close]         = {"close",         DBG_SYSCALL_RET_INT, 1,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_readv]         = {"readv",         DBG_SYSCALL_RET_SSIZE, 3,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"iov",           DBG_SYSCALL_ARG_IOV,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"iovcnt",        DBG_SYSCALL_ARG_IOVCNT,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_writev]        = {"writev",        DBG_SYSCALL_RET_SSIZE, 3,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"iov",           DBG_SYSCALL_ARG_IOV,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"iovcnt",        DBG_SYSCALL_ARG_IOVCNT,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_faccessat]     = {"faccessat",     DBG_SYSCALL_RET_INT, 3,
	                     {{"dirfd",         DBG_SYSCALL_ARG_DIRFD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"path",          DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"mode",          DBG_SYSCALL_ARG_MODE,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_AT_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_linkat]        = {"linkat",        DBG_SYSCALL_RET_INT, 5,
	                     {{"olddirfd",      DBG_SYSCALL_ARG_DIRFD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"oldpath",       DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"newdirfd",      DBG_SYSCALL_ARG_DIRFD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"newpath",       DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_AT_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_unlinkat]      = {"unlinkat",      DBG_SYSCALL_RET_INT, 3,
	                     {{"dirfd",         DBG_SYSCALL_ARG_DIRFD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"path",          DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_AT_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_chdir]         = {"chdir",         DBG_SYSCALL_RET_INT, 1,
	                     {{"path",          DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_mknodat]       = {"mknodat",       DBG_SYSCALL_RET_INT, 4,
	                     {{"dirfd",         DBG_SYSCALL_ARG_DIRFD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"path",          DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"mode",          DBG_SYSCALL_ARG_MODE,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"dev",           DBG_SYSCALL_ARG_RDEV,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_fchmodat]      = {"fchmodat",      DBG_SYSCALL_RET_INT, 4,
	                     {{"dirfd",         DBG_SYSCALL_ARG_DIRFD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"path",          DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"mode",          DBG_SYSCALL_ARG_MODE,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_AT_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_fchownat]      = {"fchownat",      DBG_SYSCALL_RET_INT, 5,
	                     {{"dirfd",         DBG_SYSCALL_ARG_DIRFD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"path",          DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"uid",           DBG_SYSCALL_ARG_UID,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"gid",           DBG_SYSCALL_ARG_GID,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_AT_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_fstatat]       = {"fstatat",       DBG_SYSCALL_RET_INT, 4,
	                     {{"dirfd",         DBG_SYSCALL_ARG_DIRFD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"path",          DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"statbuf",       DBG_SYSCALL_ARG_STAT,
	                                        DBG_SYSCALL_ARG_OUT},
	                      {"flags",         DBG_SYSCALL_ARG_AT_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_lseek]         = {"lseek",         DBG_SYSCALL_RET_INT, 3,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"offset",        DBG_SYSCALL_ARG_OFF,
	                                        DBG_SYSCALL_ARG_INOUT},
	                      {"whence",        DBG_SYSCALL_ARG_WHENCE,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_dup]           = {"dup",           DBG_SYSCALL_RET_FD, 1,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_pipe2]         = {"pipe2",         DBG_SYSCALL_RET_INT, 1,
	                     {{"fds",           DBG_SYSCALL_ARG_FDS,
	                                        DBG_SYSCALL_ARG_OUT}}},

	[SYS_ioctl]         = {"ioctl",         DBG_SYSCALL_RET_INT, 3,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"request",       DBG_SYSCALL_ARG_IOCTL_REQ,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"data",          DBG_SYSCALL_ARG_ULONG,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_dup3]          = {"dup3",          DBG_SYSCALL_RET_INT, 2,
	                     {{"oldfd",         DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"newfd",         DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_OPEN_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_symlinkat]     = {"symlinkat",     DBG_SYSCALL_RET_INT, 3,
	                     {{"target",        DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"newdirfd",      DBG_SYSCALL_ARG_DIRFD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"linkpath",      DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_readlinkat]    = {"readlinkat",    DBG_SYSCALL_RET_INT, 4,
	                     {{"dirfd",         DBG_SYSCALL_ARG_DIRFD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"path",          DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"buf",           DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_OUT},
	                      {"bufsiz",        DBG_SYSCALL_ARG_ULONG,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_umask]         = {"umask",         DBG_SYSCALL_RET_MODE, 1,
	                     {{"mask",          DBG_SYSCALL_ARG_MODE,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_fchdir]        = {"fchdir",        DBG_SYSCALL_RET_INT, 1,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_getdents]      = {"getdents",      DBG_SYSCALL_RET_INT, 3,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"dirp",          DBG_SYSCALL_ARG_DIRENT,
	                                        DBG_SYSCALL_ARG_OUT},
	                      {"count",         DBG_SYSCALL_ARG_ULONG}}},

	[SYS_utimensat]     = {"utimensat",     DBG_SYSCALL_RET_INT, 4,
	                     {{"dirfd",         DBG_SYSCALL_ARG_DIRFD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"path",          DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"times",         DBG_SYSCALL_ARG_TIMES,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_AT_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_renameat]      = {"renameat",      DBG_SYSCALL_RET_INT, 4,
	                     {{"olddirfd",      DBG_SYSCALL_ARG_DIRFD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"oldpath",       DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"newdirfd",      DBG_SYSCALL_ARG_DIRFD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"newpath",       DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_mount]         = {"mount",         DBG_SYSCALL_RET_INT, 5,
	                     {{"source",        DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"target",        DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"type",          DBG_SYSCALL_ARG_STR,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_MOUNT_FLAGS,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"data",          DBG_SYSCALL_ARG_PTR,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_fstatvfsat]    = {"fstatvfsat",    DBG_SYSCALL_RET_INT, 4,
	                     {{"dirfd",         DBG_SYSCALL_ARG_DIRFD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"path",          DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"buf",           DBG_SYSCALL_ARG_STATVFS,
	                                        DBG_SYSCALL_ARG_OUT},
	                      {"flags",         DBG_SYSCALL_ARG_AT_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_ftruncateat]   = {"ftruncateat",   DBG_SYSCALL_RET_INT, 4,
	                     {{"dirfd",         DBG_SYSCALL_ARG_DIRFD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"path",          DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"length",        DBG_SYSCALL_ARG_OFF,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_AT_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_fcntl]         = {"fcntl",         DBG_SYSCALL_RET_INT, 3,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"cmd",           DBG_SYSCALL_ARG_FCNTL_CMD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"data",          DBG_SYSCALL_ARG_ULONG,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_kmload]        = {"kmload",        DBG_SYSCALL_RET_INT, 3,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"len",           DBG_SYSCALL_ARG_ULONG,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_KMLOAD_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_kmunload]      = {"kmunload",      DBG_SYSCALL_RET_INT, 2,
	                     {{"path",          DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_KMUNLOAD_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_pselect]       = {"pselect",       DBG_SYSCALL_RET_INT, 6,
	                     {{"nfds",          DBG_SYSCALL_ARG_INT,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"rfds",          DBG_SYSCALL_ARG_FDSET,
	                                        DBG_SYSCALL_ARG_INOUT},
	                      {"wfds",          DBG_SYSCALL_ARG_FDSET,
	                                        DBG_SYSCALL_ARG_INOUT},
	                      {"efds",          DBG_SYSCALL_ARG_FDSET,
	                                        DBG_SYSCALL_ARG_INOUT},
	                      {"timeout",       DBG_SYSCALL_ARG_TIMESPEC,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"sigmask",       DBG_SYSCALL_ARG_SIGSET,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_ppoll]         = {"ppoll",         DBG_SYSCALL_RET_INT, 4,
	                     {{"fds",           DBG_SYSCALL_ARG_POLLFD,
	                                        DBG_SYSCALL_ARG_INOUT},
	                      {"nfds",          DBG_SYSCALL_ARG_NFDS,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"timeout",       DBG_SYSCALL_ARG_TIMESPEC,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"sigmask",       DBG_SYSCALL_ARG_SIGSET,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_fsync]         = {"fsync",         DBG_SYSCALL_RET_INT, 1,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_fdatasync]     = {"fdatasync",     DBG_SYSCALL_RET_INT, 1,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_chroot]        = {"chroot",        DBG_SYSCALL_RET_INT, 1,
	                     {{"path",          DBG_SYSCALL_ARG_PATH,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_getuid]        = {"getuid",        DBG_SYSCALL_RET_UID, 0},

	[SYS_getgid]        = {"getgid",        DBG_SYSCALL_RET_GID, 0},

	[SYS_geteuid]       = {"geteuid",       DBG_SYSCALL_RET_UID, 0},

	[SYS_getegid]       = {"getguid",       DBG_SYSCALL_RET_GID, 0},

	[SYS_setreuid]      = {"setreuid",      DBG_SYSCALL_RET_INT, 2,
	                     {{"ruid",          DBG_SYSCALL_ARG_UID,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"euid",          DBG_SYSCALL_ARG_UID,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_setregid]      = {"setregid",      DBG_SYSCALL_RET_INT, 2,
	                     {{"rgid",          DBG_SYSCALL_ARG_GID,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"egid",          DBG_SYSCALL_ARG_GID,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_getgroups]     = {"getgroups",     DBG_SYSCALL_RET_INT, 2,
	                     {{"size",          DBG_SYSCALL_ARG_INT},
	                      {"groups",        DBG_SYSCALL_ARG_GIDLIST,
	                                        DBG_SYSCALL_ARG_OUT}}},

	[SYS_setgroups]     = {"setgroups",     DBG_SYSCALL_RET_INT, 2,
	                     {{"size",          DBG_SYSCALL_ARG_INT,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"groups",        DBG_SYSCALL_ARG_GIDLIST,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_setuid]        = {"setuid",        DBG_SYSCALL_RET_INT, 1,
	                     {{"uid",           DBG_SYSCALL_ARG_UID,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_setgid]        = {"setgid",        DBG_SYSCALL_RET_INT, 1,
	                     {{"gid",           DBG_SYSCALL_ARG_GID,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_socket]        = {"socket",        DBG_SYSCALL_RET_FD, 3,
	                     {{"domain",        DBG_SYSCALL_ARG_SOCK_FAMILY,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"type",          DBG_SYSCALL_ARG_SOCK_TYPE,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"protocol",      DBG_SYSCALL_ARG_SOCK_PROTO,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_bind]          = {"bind",          DBG_SYSCALL_RET_INT, 3,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"addr",          DBG_SYSCALL_ARG_SOCKADDR,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"addrlen",       DBG_SYSCALL_ARG_ULONG,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_connect]       = {"connect",       DBG_SYSCALL_RET_INT, 3,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"addr",          DBG_SYSCALL_ARG_SOCKADDR,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"addrlen",       DBG_SYSCALL_ARG_ULONG,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_listen]        = {"listen",        DBG_SYSCALL_RET_INT, 2,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"backlog",       DBG_SYSCALL_ARG_INT,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_accept]        = {"accept",        DBG_SYSCALL_RET_FD, 3,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"sockaddr",      DBG_SYSCALL_ARG_SOCKADDR,
	                                        DBG_SYSCALL_ARG_OUT},
	                      {"addrlen",       DBG_SYSCALL_ARG_ULONGP,
	                                        DBG_SYSCALL_ARG_OUT}}},

	[SYS_recvmsg]       = {"recvmsg",       DBG_SYSCALL_RET_SSIZE, 3,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"msg",           DBG_SYSCALL_ARG_MSGHDR,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_MSG_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_sendmsg]       = {"sendmsg",       DBG_SYSCALL_RET_SSIZE, 3,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"msg",           DBG_SYSCALL_ARG_MSGHDR,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_MSG_FLAGS}}},

	[SYS_getsockopt]    = {"getsockopt",    DBG_SYSCALL_RET_INT, 5,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"level",         DBG_SYSCALL_ARG_SOCK_LEVEL,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"opt",           DBG_SYSCALL_ARG_SOCK_OPT,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"val",           DBG_SYSCALL_ARG_PTR,
	                                        DBG_SYSCALL_ARG_OUT},
	                      {"len",           DBG_SYSCALL_ARG_ULONGP,
	                                        DBG_SYSCALL_ARG_INOUT}}},

	[SYS_setsockopt]    = {"setsockopt",    DBG_SYSCALL_RET_INT, 5,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"level",         DBG_SYSCALL_ARG_SOCK_LEVEL,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"opt",           DBG_SYSCALL_ARG_SOCK_OPT,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"val",           DBG_SYSCALL_ARG_PTR,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"len",           DBG_SYSCALL_ARG_ULONG,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_getpeername]   = {"getpeername",   DBG_SYSCALL_RET_INT, 3,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"addr",          DBG_SYSCALL_ARG_SOCKADDR,
	                                        DBG_SYSCALL_ARG_OUT},
	                      {"len",           DBG_SYSCALL_ARG_ULONGP,
	                                        DBG_SYSCALL_ARG_OUT}}},

	[SYS_getsockname]   = {"getsockname",   DBG_SYSCALL_RET_INT, 3,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"addr",          DBG_SYSCALL_ARG_SOCKADDR,
	                                        DBG_SYSCALL_ARG_OUT},
	                      {"len",           DBG_SYSCALL_ARG_ULONGP,
	                                        DBG_SYSCALL_ARG_OUT}}},

	[SYS_shutdown]      = {"shutdown",      DBG_SYSCALL_RET_INT, 2,
	                     {{"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"how",           DBG_SYSCALL_ARG_SHUTDOWN_HOW,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_socketpair]    = {"socketpair",    DBG_SYSCALL_RET_FD, 4,
	                     {{"domain",        DBG_SYSCALL_ARG_SOCK_FAMILY,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"type",          DBG_SYSCALL_ARG_SOCK_TYPE,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"protocol",      DBG_SYSCALL_ARG_SOCK_PROTO,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"fds",           DBG_SYSCALL_ARG_FDS,
	                                        DBG_SYSCALL_ARG_OUT}}},

	[SYS_time]          = {"time",          DBG_SYSCALL_RET_INT, 1,
	                     {{"tloc",          DBG_SYSCALL_ARG_TIME,
	                                        DBG_SYSCALL_ARG_OUT}}},

	[SYS_mmap]          = {"mmap",          DBG_SYSCALL_RET_PTR, 6,
	                     {{"addr",          DBG_SYSCALL_ARG_PTR,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"len",           DBG_SYSCALL_ARG_ULONG,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"prot",          DBG_SYSCALL_ARG_PROT,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_MMAP_FLAGS,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"fd",            DBG_SYSCALL_ARG_FD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"off",           DBG_SYSCALL_ARG_OFF,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_munmap]        = {"munmap",        DBG_SYSCALL_RET_INT, 2,
	                     {{"addr",          DBG_SYSCALL_ARG_PTR,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"len",           DBG_SYSCALL_ARG_ULONG,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_mprotect]      = {"mprotect",      DBG_SYSCALL_RET_INT, 3,
	                     {{"addr",          DBG_SYSCALL_ARG_PTR,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"len",           DBG_SYSCALL_ARG_ULONG,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"prot",          DBG_SYSCALL_ARG_PROT,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_msync]         = {"msync",         DBG_SYSCALL_RET_INT, 3,
	                     {{"addr",          DBG_SYSCALL_ARG_PTR,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"len",           DBG_SYSCALL_ARG_ULONG,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_MSYNC,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_nanosleep]     = {"nanosleep",     DBG_SYSCALL_RET_INT, 2,
	                     {{"req",           DBG_SYSCALL_ARG_TIMESPEC,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"rem",           DBG_SYSCALL_ARG_TIMESPEC,
	                                        DBG_SYSCALL_ARG_OUT}}},

	[SYS_clock_settime] = {"clock_settime", DBG_SYSCALL_RET_INT, 2,
	                     {{"clk_id",        DBG_SYSCALL_ARG_CLOCKID,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"ts",            DBG_SYSCALL_ARG_TIMESPEC,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_clock_gettime] = {"clock_gettime", DBG_SYSCALL_RET_INT, 2,
	                     {{"clk_id",        DBG_SYSCALL_ARG_CLOCKID,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"ts",            DBG_SYSCALL_ARG_TIMESPEC,
	                                        DBG_SYSCALL_ARG_OUT}}},

	[SYS_clock_getres]  = {"clock_getres",  DBG_SYSCALL_RET_INT, 2,
	                     {{"clk_id",        DBG_SYSCALL_ARG_CLOCKID,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"ts",            DBG_SYSCALL_ARG_TIMESPEC,
	                                        DBG_SYSCALL_ARG_OUT}}},

	[SYS_getpagesize]   = {"getpagesize",   DBG_SYSCALL_RET_INT, 0},

	[SYS_uname]         = {"uname",         DBG_SYSCALL_RET_INT, 1,
	                     {{"buf",           DBG_SYSCALL_ARG_UTSNAME,
	                                        DBG_SYSCALL_ARG_OUT}}},

	[SYS_futex]         = {"futex",         DBG_SYSCALL_RET_INT, 4,
	                     {{"uaddr",         DBG_SYSCALL_ARG_INTP,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"op",            DBG_SYSCALL_ARG_FUTEX_OP,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"val",           DBG_SYSCALL_ARG_INT,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"timeout",       DBG_SYSCALL_ARG_TIMESPEC,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_sigsuspend]    = {"sigsuspend",    DBG_SYSCALL_RET_INT, 1,
	                     {{"set",           DBG_SYSCALL_ARG_SIGSET,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_madvise]       = {"madvise",       DBG_SYSCALL_RET_INT, 3,
	                     {{"addr",          DBG_SYSCALL_ARG_PTR,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"len",           DBG_SYSCALL_ARG_ULONG,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"advise",        DBG_SYSCALL_ARG_ADVISE,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_reboot]        = {"reboot",        DBG_SYSCALL_RET_INT, 1,
	                     {{"cmd",           DBG_SYSCALL_ARG_REBOOT_CMD,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_shmget]        = {"shmget",        DBG_SYSCALL_RET_SHMID, 3,
	                     {{"key",           DBG_SYSCALL_ARG_KEY,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"size",          DBG_SYSCALL_ARG_ULONG,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_IPC_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_shmat]         = {"shmat",         DBG_SYSCALL_RET_PTR, 3,
	                     {{"shmid",         DBG_SYSCALL_ARG_SHMID,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"size",          DBG_SYSCALL_ARG_ULONG,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_SHM_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_shmdt]         = {"shmdt",         DBG_SYSCALL_RET_INT, 1,
	                     {{"addr",          DBG_SYSCALL_ARG_PTR,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_shmctl]        = {"shmctl",        DBG_SYSCALL_RET_INT, 3,
	                     {{"shmid",         DBG_SYSCALL_ARG_SHMID,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"cmd",           DBG_SYSCALL_ARG_SHM_CMD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"buf",           DBG_SYSCALL_ARG_PTR,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_semget]        = {"semget",        DBG_SYSCALL_RET_SEMID, 3,
	                     {{"key",           DBG_SYSCALL_ARG_KEY,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"nsems",         DBG_SYSCALL_ARG_INT,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_IPC_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_semtimedop]    = {"semtimedop",    DBG_SYSCALL_RET_INT, 4,
	                     {{"semid",         DBG_SYSCALL_ARG_SEMID,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"sops",          DBG_SYSCALL_ARG_SEMBUF,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"nsops",         DBG_SYSCALL_ARG_ULONG,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"timeout",       DBG_SYSCALL_ARG_TIMESPEC,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_semctl]        = {"semctl",        DBG_SYSCALL_RET_INT, 4,
	                     {{"semid",         DBG_SYSCALL_ARG_SEMID,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"sennum",        DBG_SYSCALL_ARG_INT,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"cmd",           DBG_SYSCALL_ARG_SEM_CMD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"data",          DBG_SYSCALL_ARG_PTR,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_msgget]        = {"msgget",        DBG_SYSCALL_RET_MSGID, 2,
	                     {{"key",           DBG_SYSCALL_ARG_KEY,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_IPC_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_msgsnd]        = {"msgsnd",        DBG_SYSCALL_RET_INT, 4,
	                     {{"msgid",         DBG_SYSCALL_ARG_MSGID,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"msgp",          DBG_SYSCALL_ARG_PTR,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"msgsz",         DBG_SYSCALL_ARG_ULONG,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_IPC_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_msgrcv]        = {"msgrcv",        DBG_SYSCALL_RET_SSIZE, 5,
	                     {{"msgid",         DBG_SYSCALL_ARG_MSGID,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"msgp",          DBG_SYSCALL_ARG_PTR,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"msgsz",         DBG_SYSCALL_ARG_ULONG,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"msgtyp",        DBG_SYSCALL_ARG_LONG,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"flags",         DBG_SYSCALL_ARG_IPC_FLAGS,
	                                        DBG_SYSCALL_ARG_IN}}},

	[SYS_msgctl]        = {"msgctl",        DBG_SYSCALL_RET_INT, 3,
	                     {{"msgid",         DBG_SYSCALL_ARG_MSGID,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"cmd",           DBG_SYSCALL_ARG_MSG_CMD,
	                                        DBG_SYSCALL_ARG_IN},
	                      {"buf",           DBG_SYSCALL_ARG_PTR,
	                                        DBG_SYSCALL_ARG_IN}}},
};

const struct dbg_syscall *
dbg_syscall_get(int syscall)
{
	if (syscall < 0
	 || (unsigned)syscall >= sizeof(syscalls) / sizeof(*syscalls)
	 || !syscalls[syscall].name[0])
		return NULL;
	return &syscalls[syscall];
}

int
dbg_syscall_from_name(const char *name)
{
	for (size_t i = 0; i < sizeof(syscalls) / sizeof(*syscalls); ++i)
	{
		if (!strcasecmp(syscalls[i].name, name))
			return i;
	}
	return -1;
}
