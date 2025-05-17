#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>

#include <inttypes.h>
#include <libdbg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <stdio.h>

struct env
{
	const char *progname;
	int colored;
	pid_t child;
	int killed;
};

static struct env *g_env;

#define PTRACE_ASSERT(req, pid, addr, data) \
do \
{ \
	if (ptrace(req, pid, addr, data)) \
	{ \
		fprintf(stderr, "%s: ptrace(" #req "): %s\n", \
		        env->progname, strerror(errno)); \
		kill(pid, SIGKILL); \
		exit(EXIT_FAILURE); \
	} \
} while (0)

static void
sig_handler(struct env *env, int signum)
{
	const struct dbg_signal *signal_def;
	siginfo_t siginfo;

	if (env->killed)
		return;
	signal_def = dbg_signal_get(signum);
	PTRACE_ASSERT(PTRACE_GETSIGINFO, env->child, NULL, &siginfo);
	if (env->colored)
		printf("\033[1;37m");
	printf("--- %s { si_signo=%s, si_code=%d, si_pid=%" PRId32
	       ", si_uid=%" PRId32 ", si_value=%d, si_addr=%p} ---\n",
	       signal_def ? signal_def->name : "",
	       signal_def ? signal_def->name : "",
	       siginfo.si_code, siginfo.si_pid, siginfo.si_uid,
	       siginfo.si_value.sival_int, siginfo.si_addr);
}

static void
setemptyset(void)
{
	sigset_t empty;

	sigemptyset(&empty);
	sigprocmask(SIG_SETMASK, &empty, NULL);
}

static int
syscall_wait(struct env *env, int *exit_status, int *exit_return)
{
	sigset_t blocker;
	int wstatus;
	int signum = 0;

	sigemptyset(&blocker);
	sigaddset(&blocker, SIGHUP);
	sigaddset(&blocker, SIGQUIT);
	sigaddset(&blocker, SIGPIPE);
	sigaddset(&blocker, SIGTERM);
	while (1)
	{
		PTRACE_ASSERT(PTRACE_SYSCALL, env->child, 0, signum);
		setemptyset();
		waitpid(env->child, &wstatus, 0);
		sigprocmask(SIG_BLOCK, &blocker, NULL);
		signum = 0;
		if (WIFEXITED(wstatus) || WIFSIGNALED(wstatus))
		{
			*exit_return = wstatus;
			*exit_status = WEXITSTATUS(wstatus);
			return 1;
		}
		if (WIFSTOPPED(wstatus))
		{
			if (WSTOPSIG(wstatus) & 0x80)
				return 0;
			signum = WSTOPSIG(wstatus);
			sig_handler(env, signum);
		}
	}
}

static void
sigint_handler(int signum)
{
	struct env *env = g_env;

	(void)signum;
	if (env->colored)
		printf("\033[0m");
	printf("Process %" PRId32 " detached\n", env->child);
	fflush(stdout);
	PTRACE_ASSERT(PTRACE_DETACH, env->child, 0, SIGINT);
	exit(EXIT_SUCCESS);
}

static int
peekdata(void *data, size_t size, uintptr_t addr, void *userptr)
{
	struct env *env = userptr;
	ssize_t i;
	uintptr_t tmp;

	for (i = -(addr % sizeof(tmp)); i < (ssize_t)size; i += sizeof(tmp))
	{
		errno = 0;
		tmp = ptrace(PTRACE_PEEKDATA, env->child, addr + i, NULL);
		if (errno)
			return 1;
		uint8_t *ptr = (uint8_t*)&tmp;
		size_t len = sizeof(tmp);
		if (i < 0)
		{
			ptr -= i;
			len += i;
		}
		if (len > size)
			len = size;
		memcpy(data, ptr, len);
		data = (uint8_t*)data + len;
	}
	return 0;
}

static void
print_arg(struct env *env,
          const struct dbg_syscall *syscall,
          const uintptr_t *values,
          size_t param)
{
	char buf[4096];

	if (env->colored)
		printf("\033[1;34m");
	if (dbg_syscall_arg_print(buf,
	                          sizeof(buf),
	                          syscall,
	                          values,
	                          param,
	                          peekdata,
	                          env))
		printf("/* INVALID DATA */");
	else
		printf("%s", buf);
	if (env->colored)
		printf("\033[0m");
}

static void
print_args(struct env *env,
           const struct user_regs_struct *regs,
           const struct dbg_syscall *syscall)
{
	uintptr_t args[6];

#if defined(__i386__)
	args[0] = regs->ebx;
	args[1] = regs->ecx;
	args[2] = regs->edx;
	args[3] = regs->esi;
	args[4] = regs->edi;
	args[5] = regs->ebp;
#elif defined (__x86_64__)
	args[0] = regs->rdi;
	args[1] = regs->rsi;
	args[2] = regs->rdx;
	args[3] = regs->r10;
	args[4] = regs->r8;
	args[5] = regs->r9;
#elif defined(__arm__)
	args[0] = regs->r[0];
	args[1] = regs->r[1];
	args[2] = regs->r[2];
	args[3] = regs->r[3];
	args[4] = regs->r[4];
	args[5] = regs->r[5];
#elif defined(__aarch64__)
	args[0] = regs->x[0];
	args[1] = regs->x[1];
	args[2] = regs->x[2];
	args[3] = regs->x[3];
	args[4] = regs->x[4];
	args[5] = regs->x[5];
#elif defined(__riscv)
	args[0] = regs->a0;
	args[1] = regs->a1;
	args[2] = regs->a2;
	args[3] = regs->a3;
	args[4] = regs->a4;
	args[5] = regs->a5;
#elif defined(__mips__)
	args[0] = regs->a0;
	args[1] = regs->a1;
	args[2] = regs->a2;
	args[3] = regs->a3;
#if defined(__mips64)
	args[4] = regs->a4;
	args[5] = regs->a5;
#else
	args[4] = regs->t0;
	args[5] = regs->t1;
#endif
#else
# error "unknown arch"
#endif

	for (size_t i = 0; i < syscall->params_nb; ++i)
	{
		if (i)
			printf(", ");
		print_arg(env, syscall, args, i);
	}
}

static void
print_syscall_call(struct env *env,
                   const struct dbg_syscall *syscall_def,
                   const struct user_regs_struct *regs)
{
	size_t args_nb;

	if (env->colored)
		printf("\033[1;35m");
	printf("%s", syscall_def ? syscall_def->name : "unknown");
	if (env->colored)
		printf("\033[0m");
	printf("(");
	args_nb = syscall_def ? syscall_def->params_nb : 0;
	if (args_nb == 0)
		printf("void");
	else
		print_args(env, regs, syscall_def);
	fflush(stdout);
}

static void
print_syscall_ret(struct env *env,
                  const struct dbg_syscall *syscall_def,
                  const struct user_regs_struct *regs)
{
	uintptr_t ret;

	if (env->colored)
		printf("\033[1;37m");
	printf(") = ");
#if defined(__i386__)
	ret = regs->eax;
#elif defined(__x86_64__)
	ret = regs->rax;
#elif defined(__arm__)
	ret = regs->r[0];
#elif defined(__aarch64__)
	ret = regs->x[0];
#elif defined(__riscv)
	ret = regs->a0;
#elif defined(__mips__)
	ret = regs->v0;
#else
# error "unknown arch"
#endif
	if ((intptr_t)ret < 0 && (intptr_t)ret >= -4095)
	{
		int errno_id = -(intptr_t)ret;
		const struct dbg_errno *errno_def = dbg_errno_get(errno_id);
		if (env->colored)
			printf("\033[1;31m");
		const char *desc = strerror(errno_id);
		printf("%d %s (%s)", -1,
		       errno_def ? errno_def->name : "unknown",
		       desc ? desc : "");
	}
	else
	{
		if (env->colored)
			printf("\033[1;32m");
		if (syscall_def)
		{
			char buf[64];
			dbg_syscall_ret_print(buf, sizeof(buf), syscall_def, ret);
			printf("%s", buf);
		}
		else
		{
			printf("%d", (int)ret);
		}
	}
	if (env->colored)
		printf("\033[0m");
	printf("\n");
	fflush(stdout);
}

static void
parent_launch(struct env *env)
{
	struct user_regs_struct regs;
	int calling;
	int exit_return;
	int exit_status;

	calling = 0;
	exit_status = 0;
	signal(SIGINT, sigint_handler);
	waitpid(env->child, NULL, 0);
	PTRACE_ASSERT(PTRACE_SETOPTIONS, env->child, 0, PTRACE_O_TRACESYSGOOD);
	while (1)
	{
		if (syscall_wait(env, &exit_status, &exit_return))
			break;
		if (!env->killed)
		{
			PTRACE_ASSERT(PTRACE_GETREGS, env->child, 0, &regs);
			if (!env->killed)
			{
				calling = 1;
				size_t syscall_id;
#if defined(__i386__)
				syscall_id = regs.eax;
#elif defined(__x86_64__)
				syscall_id = regs.rax;
#elif defined(__arm__)
				syscall_id = regs.r[7];
#elif defined(__aarch64__)
				syscall_id = regs.x[8];
#elif defined(__riscv)
				syscall_id = regs.a7;
#elif defined(__mips__)
				syscall_id = regs.v0;
#else
# error "unknown arch"
#endif
				const struct dbg_syscall *syscall_def = dbg_syscall_get(syscall_id);
				print_syscall_call(env, syscall_def, &regs);
				if (syscall_wait(env, &exit_status, &exit_return))
					break;
				if (!env->killed)
				{
					calling = 0;
					PTRACE_ASSERT(PTRACE_GETREGS, env->child, 0, &regs);
					print_syscall_ret(env, syscall_def, &regs);
				}
			}
		}
	}
	setemptyset();
	if (env->killed)
		return;
	if (calling)
	{
		if (env->colored)
			printf("\033[1;37m");
		printf(") = ?\n");
	}
	if (WIFEXITED(exit_return))
	{
		printf("+++ exited with %d +++\n", exit_status);
		if (env->colored)
			printf("\033[0m");
		exit(exit_status);
	}
	if (WIFSIGNALED(exit_return))
	{
		const struct dbg_signal *signal_def = dbg_signal_get(WTERMSIG(exit_return));
		if (WCOREDUMP(exit_return))
			printf("+++ Killed by %s +++ (core dumped)\n",
			       signal_def ? signal_def->name : "unknown");
		else
			printf("+++ Killed by %s +++\n",
			       signal_def ? signal_def->name : "unknown");
		if (env->colored)
			printf("\033[0m");
		fflush(stdout);
		kill(getpid(), WTERMSIG(exit_return));
		return;
	}
	if (env->colored)
		printf("\033[0m");
	fflush(stdout);
}

static int
child_launch(struct env *env, char **argv)
{
	env->child = fork();
	if (env->child == -1)
	{
		fprintf(stderr, "%s: fork: %s\n", env->progname, strerror(errno));
		return 1;
	}
	if (env->child == 0)
	{
		PTRACE_ASSERT(PTRACE_TRACEME, 0, 0, 0);
		if (execvp(argv[0], &argv[0]) == -1)
		{
			fprintf(stderr, "%s: execvp: %s\n", env->progname, strerror(errno));
			exit(EXIT_FAILURE);
		}
		return 1;
	}
	return 0;
}

static void
usage(const char *progname)
{
	printf("%s [-h] PROGRAM ARGS\n", progname);
	printf("-h: show this help\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	g_env = &env;
	while ((c = getopt(argc, argv, "h")) != -1)
	{
		switch (c)
		{
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (argc - optind < 1)
	{
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (child_launch(&env, &argv[optind]))
		return EXIT_FAILURE;
	parent_launch(&env);
	return EXIT_SUCCESS;
}
