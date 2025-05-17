#include <readline/readline.h>
#include <readline/history.h>

#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>

#if defined(__x86_64__)
#include <libasm/x86.h>
#endif

#include <arpa/inet.h>

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <libdbg.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

struct env
{
	const char *progname;
	int argc;
	char **argv;
	pid_t child;
	int child_signum;
};

struct cmd
{
	const char *name;
	int (*cmd)(struct env *env, int argc, const char **argv);
};

static int
run_cmd(struct env *env, const struct cmd *cmds, int argc, const char **argv)
{
	const struct cmd *cmd = NULL;

	if (!argc)
		return -2;
	for (size_t i = 0; cmds[i].name; ++i)
	{
		if (strncmp(cmds[i].name, argv[0], strlen(argv[0])))
			continue;
		if (cmd)
		{
			fprintf(stderr, "ambiguous command\n");
			return -3;
		}
		cmd = &cmds[i];
	}
	if (!cmd)
		return -1;
	return cmd->cmd(env, argc, argv);
}

static int
launch_child(struct env *env)
{
	env->child = fork();
	if (env->child == -1)
	{
		fprintf(stderr, "%s: fork: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	if (!env->child)
	{
		if (setpgid(0, 0) == -1)
		{
			fprintf(stderr, "%s: setpgid: %s\n", env->progname,
			        strerror(errno));
			exit(EXIT_FAILURE);
		}
		tcsetpgrp(0, getpgrp());
		if (ptrace(PTRACE_TRACEME, 0, 0, 0) == -1)
		{
			fprintf(stderr, "%s: ptrace(PTRACE_TRACEME): %s\n",
			        env->progname, strerror(errno));
			exit(EXIT_FAILURE);
		}
		execvp(env->argv[0], env->argv);
		fprintf(stderr, "%s: execve: %s\n", env->progname,
		        strerror(errno));
		exit(EXIT_FAILURE);
	}
	while (1)
	{
		int wstatus;
		if (waitpid(env->child, &wstatus, 0) == -1)
		{
			if (errno == EINTR)
				continue;
			kill(SIGKILL, env->child);
			env->child = -1;
			fprintf(stderr, "%s: waitpid: %s\n", env->progname,
			        strerror(errno));
			return 1;
		}
		if (ptrace(PTRACE_SETOPTIONS, env->child, 0, PTRACE_O_EXITKILL))
		{
			kill(SIGKILL, env->child);
			env->child = -1;
			fprintf(stderr, "%s: ptrace(PTRACE_SETOPTIONS, PTRACE_SETOPTIONS): %s\n",
			        env->progname, strerror(errno));
			return 1;
		}
		if (ptrace(PTRACE_CONT, env->child, 0, 0))
		{
			kill(SIGKILL, env->child);
			env->child = -1;
			fprintf(stderr, "%s: ptrace(PTRACE_CONT): %s\n",
			        env->progname, strerror(errno));
			return 1;
		}
		return 0;
	}
}

static int
child_wait(struct env *env)
{
	while (1)
	{
		int wstatus;
		pid_t pid;

		pid = waitpid(env->child, &wstatus, 0);
		if (pid == -1)
		{
			if (errno == EINTR)
				continue;
		}
		if (WIFEXITED(wstatus))
		{
			printf("[process %" PRId32 " terminated with code %d]\n",
			       pid, WEXITSTATUS(wstatus));
			env->child = -1;
			return 0;
		}
		if (WIFSIGNALED(wstatus))
		{
			const struct dbg_signal *sig;

			sig = dbg_signal_get(WTERMSIG(wstatus));
			if (sig)
				printf("[process %" PRId32 " killed by signal %s]\n",
				       pid, sig->name);
			else
				printf("[process %" PRId32 " killed by signal %d]\n",
				       pid, WTERMSIG(wstatus));
			env->child = -1;
			return 0;
		}
		if (!WIFSTOPPED(wstatus))
		{
			fprintf(stderr, "unhandled wstatus %x\n", wstatus);
			return 1;
		}
		env->child_signum = WSTOPSIG(wstatus);
		if (env->child_signum == SIGTRAP)
		{
			/* XXX take action on caller (e.g: "breakpoint reach") */
			env->child_signum = 0;
			return 0;
		}
		const struct dbg_signal *sig = dbg_signal_get(env->child_signum);
		if (sig)
			printf("[process %" PRId32 " received signal %s]\n",
			       pid, sig->name);
		else
			printf("[process %" PRId32 " received signal %d]\n",
			       pid, env->child_signum);
		if (env->child_signum == SIGINT)
			env->child_signum = 0;
		return 0;
	}
}

static int
cmd_run(struct env *env, int argc, const char **argv)
{
	(void)argc;
	(void)argv;
	if (env->child != -1)
	{
		printf("child is already running\n");
		return 0;
	}
	if (launch_child(env))
		return 1;
	return child_wait(env);
}

static int
cmd_info_registers(struct env *env, int argc, const char **argv)
{
	struct user_regs_struct regs;

	(void)argc;
	(void)argv;
	if (ptrace(PTRACE_GETREGS, env->child, 0, &regs))
	{
		fprintf(stderr, "%s: ptrace(PTRACE_GETREGS): %s\n",
		       env->progname, strerror(errno));
		return 1;
	}
#if defined(__i386__)

#define PRINT_REG(r) printf("%-5s 0x%08" PRIx32 "   %" PRId32 "\n", \
                            #r, regs.r, (int32_t)regs.r);
	PRINT_REG(eax);
	PRINT_REG(ebx);
	PRINT_REG(ecx);
	PRINT_REG(edx);
	PRINT_REG(esi);
	PRINT_REG(edi);
	PRINT_REG(ebp);
	PRINT_REG(esp);
	PRINT_REG(eip);
	PRINT_REG(ef);
	PRINT_REG(cs);
	PRINT_REG(ds);
	PRINT_REG(es);
	PRINT_REG(fs);
	PRINT_REG(gs);
#undef PRINT_REG

#elif defined(__x86_64__)

#define PRINT_REG(r) printf("%-5s 0x%016" PRIx64 "   %" PRId64 "\n", \
                            #r, regs.r, (int64_t)regs.r);
	PRINT_REG(rax);
	PRINT_REG(rbx);
	PRINT_REG(rcx);
	PRINT_REG(rdx);
	PRINT_REG(rdi);
	PRINT_REG(rsi);
	PRINT_REG(rbp);
	PRINT_REG(rsp);
	PRINT_REG(r8);
	PRINT_REG(r9);
	PRINT_REG(r10);
	PRINT_REG(r11);
	PRINT_REG(r12);
	PRINT_REG(r13);
	PRINT_REG(r14);
	PRINT_REG(r15);
	PRINT_REG(rip);
	PRINT_REG(rf);
	PRINT_REG(cs);
	PRINT_REG(ds);
	PRINT_REG(es);
	PRINT_REG(fs);
	PRINT_REG(gs);
#undef PRINT_REG

#elif defined(__arm__)

	for (int i = 0; i < 16; ++i)
		printf("r%-4d 0x%08" PRIx32 "   %" PRId32 "\n",
		      i, regs.r[i], (int32_t)regs.r[i]);

#elif defined(__aarch64__)

	for (int i = 0; i < 32; ++i)
		printf("r%-4d 0x%016" PRIx64 "   %" PRId64 "\n",
		      i, regs.x[i], (int64_t)regs.x[i]);

#elif defined(__riscv)

#if __riscv_xlen == 64
#define PRINT_REG(r) printf("%-5s 0x%016" PRIx64 "   %" PRId64 "\n", \
                            #r, regs.r, (int64_t)regs.r);
#else
#define PRINT_REG(r) printf("%-5s 0x%08" PRIx32 "   %" PRId32 "\n", \
                            #r, regs.r, (int32_t)regs.r);
#endif
PRINT_REG(pc);
PRINT_REG(ra);
PRINT_REG(sp);
PRINT_REG(gp);
PRINT_REG(tp);
PRINT_REG(t0);
PRINT_REG(t1);
PRINT_REG(t2);
PRINT_REG(fp);
PRINT_REG(s1);
PRINT_REG(a0);
PRINT_REG(a1);
PRINT_REG(a2);
PRINT_REG(a3);
PRINT_REG(a4);
PRINT_REG(a5);
PRINT_REG(a6);
PRINT_REG(a7);
PRINT_REG(s2);
PRINT_REG(s3);
PRINT_REG(s4);
PRINT_REG(s5);
PRINT_REG(s6);
PRINT_REG(s7);
PRINT_REG(s8);
PRINT_REG(s9);
PRINT_REG(s10);
PRINT_REG(s11);
PRINT_REG(t3);
PRINT_REG(t4);
PRINT_REG(t5);
PRINT_REG(t6);
#undef PRINT_REG

#elif defined(__mips__)

#if defined(__mips64)
#define PRINT_REG(r) printf("%-5s 0x%016" PRIx64 "   %" PRId64 "\n", \
                            #r, regs.r, (int64_t)regs.r);
#else
#define PRINT_REG(r) printf("%-5s 0x%08" PRIx32 "   %" PRId32 "\n", \
                            #r, regs.r, (int32_t)regs.r);
#endif
PRINT_REG(pc);
PRINT_REG(at);
PRINT_REG(v0);
PRINT_REG(v1);
PRINT_REG(a0);
PRINT_REG(a1);
PRINT_REG(a2);
PRINT_REG(a3);
#if defined(__mips64)
PRINT_REG(a4);
PRINT_REG(a5);
PRINT_REG(a6);
PRINT_REG(a7);
#else
PRINT_REG(t0);
PRINT_REG(t1);
PRINT_REG(t2);
PRINT_REG(t3);
#endif
PRINT_REG(t4);
PRINT_REG(t5);
PRINT_REG(t6);
PRINT_REG(t7);
PRINT_REG(s0);
PRINT_REG(s1);
PRINT_REG(s2);
PRINT_REG(s3);
PRINT_REG(s4);
PRINT_REG(s5);
PRINT_REG(s6);
PRINT_REG(s7);
PRINT_REG(t8);
PRINT_REG(t9);
PRINT_REG(k0);
PRINT_REG(k1);
PRINT_REG(gp);
PRINT_REG(sp);
PRINT_REG(fp);
PRINT_REG(ra);
#undef PRINT_REG

#else
# error "unknown arch"
#endif
	return 0;
}

static const struct cmd
info_cmds[] =
{
	{"registers", cmd_info_registers},
	{NULL       , NULL},
};

static int
cmd_info(struct env *env, int argc, const char **argv)
{
	int ret;

	ret = run_cmd(env, info_cmds, argc - 1, argv + 1);
	switch (ret)
	{
		case -2:
			fprintf(stderr, "info require subcommand");
			ret = 1;
			break;
		case -1:
			printf("unknown info subcommand command: \"%s\"\n", argv[0]);
			ret = 1;
			break;
		default:
			break;
	}
	return ret;
}

static int
cmd_continue(struct env *env, int argc, const char **argv)
{
	(void)argc;
	(void)argv;
	if (env->child == -1)
	{
		printf("no child to continue\n");
		return 0;
	}
	if (ptrace(PTRACE_CONT, env->child, 0, env->child_signum))
	{
		fprintf(stderr, "%s: ptrace(PTRACE_CONT): %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	env->child_signum = 0;
	return child_wait(env);
}

static int
cmd_stepi(struct env *env, int argc, const char **argv)
{
	(void)argc;
	(void)argv;
	if (env->child == -1)
	{
		printf("no child to continue\n");
		return 0;
	}
	if (ptrace(PTRACE_SINGLESTEP, env->child, 0, env->child_signum))
	{
		fprintf(stderr, "%s: ptrace(PTRACE_SINGLESTEP): %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	env->child_signum = 0;
	return child_wait(env);
}

static int
cmd_help(struct env *env, int argc, const char **argv)
{
	(void)env;
	(void)argc;
	(void)argv;
	printf("help: show this help\n");
	printf("run: run the program\n");
	printf("quit: quit jdb\n");
	printf("disas: disassemble the current instruction\n");
	return 0;
}

static int
cmd_quit(struct env *env, int argc, const char **argv)
{
	(void)argc;
	(void)argv;
	if (env->child != -1)
		kill(SIGKILL, env->child);
	exit(EXIT_SUCCESS);
}

static int
cmd_disas(struct env *env, int argc, const char **argv)
{
#if defined(__x86_64__) /* XXX */
	char buf[4096];
	uint8_t data[16];
	struct user_regs_struct regs;
	uintptr_t addr;
	uintptr_t pad;

	(void)argc;
	(void)argv;
	if (ptrace(PTRACE_GETREGS, env->child, 0, &regs) == -1)
	{
		fprintf(stderr, "%s: ptrace(PTRACE_GETREGS): %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	addr = regs.rip;
	pad = addr % sizeof(addr);
	addr -= pad;
	for (size_t i = 0; i < sizeof(data);)
	{
		errno = 0;
		uintptr_t tmp = ptrace(PTRACE_PEEKDATA, env->child, addr, NULL);
		if (errno)
		{
			fprintf(stderr, "%s: ptrace(PTRACE_PEEKDATA): %s\n",
			        env->progname, strerror(errno));
			return 1;
		}
		for (size_t j = pad; j < sizeof(tmp); ++j)
			data[i++] = tmp >> (8 * j);
		if (pad)
			pad = 0;
		addr += sizeof(tmp);
	}
	asm_x86_disas(buf, sizeof(buf), data, 0);
	printf("%s\n", buf);
#endif
	return 0;
}

static int
cmd_backtrace(struct env *env, int argc, const char **argv)
{
	struct user_regs_struct regs;
	uintptr_t off;
	uintptr_t fp;
	uintptr_t pc;
	size_t i;

	(void)argc;
	(void)argv;
	if (env->child == -1)
	{
		printf("no child\n");
		return 0;
	}
	if (ptrace(PTRACE_GETREGS, env->child, 0, &regs) == -1)
	{
		fprintf(stderr, "%s: ptrace(PTRACE_GETREGS): %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
#if defined(__i386__)
	fp = regs.ebp;
	pc = regs.eip;
	off = 0;
#elif defined(__x86_64__)
	fp = regs.rbp;
	pc = regs.rip;
	off = 0;
#elif defined(__arm__)
	fp = regs.r[11];
	pc = regs.r[15];
	off = (uintptr_t)-4;
#elif defined(__aarch64__)
	fp = regs.x[29];
	pc = regs.pc;
	off = 0;
#elif defined(__riscv)
	fp = regs.fp;
	pc = regs.pc;
	off = (uintptr_t)-16;
#else
# error "unknown arch"
#endif
	i = 0;
	while (pc)
	{
		printf("[%3zu] FP=%.*zx PC=%.*zx\n",
		       i,
		       (int)(sizeof(uintptr_t) * 2), fp,
		       (int)(sizeof(uintptr_t) * 2), pc);
		errno = 0;
		pc = ptrace(PTRACE_PEEKDATA, env->child, fp + off + sizeof(uintptr_t), NULL);
		if (errno)
		{
			fprintf(stderr, "%s: ptrace(PTRACE_PEEKDATA): %s\n",
			        env->progname, strerror(errno));
			return 1;
		}
		fp = ptrace(PTRACE_PEEKDATA, env->child, fp + off, NULL);
		if (errno)
		{
			fprintf(stderr, "%s: ptrace(PTRACE_PEEKDATA): %s\n",
			        env->progname, strerror(errno));
			return 1;
		}
		i++;
	}
	return 0;
}

static int
cmd_signal(struct env *env, int argc, const char **argv)
{
	char *endptr;
	long signum;

	if (argc < 1)
	{
		fprintf(stderr, "signal <signal>\n");
		return 0;
	}
	if (env->child == -1)
	{
		printf("no child to signal\n");
		return 0;
	}
	errno = 0;
	signum = strtol(argv[1], &endptr, 10);
	if (errno || *endptr)
	{
		signum = dbg_signal_from_name(argv[1]);
		if (signum == -1)
		{
			fprintf(stderr, "invalid signal\n");
			return 0;
		}
	}
	if (ptrace(PTRACE_CONT, env->child, 0, signum))
	{
		fprintf(stderr, "%s: ptrace(PTRACE_CONT): %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	env->child_signum = 0;
	return child_wait(env);
}

static const struct cmd
cmds[] =
{
	{"run"      , cmd_run},
	{"info"     , cmd_info},
	{"continue" , cmd_continue},
	{"stepi"    , cmd_stepi},
	{"help"     , cmd_help},
	{"quit"     , cmd_quit},
	{"disas"    , cmd_disas},
	{"bt"       , cmd_backtrace},
	{"backtrace", cmd_backtrace},
	{"signal"   , cmd_signal},
	{NULL       , NULL},
};

static int
exec_line(struct env *env, const char *line)
{
	char **argv = NULL;
	int argc = 0;
	int ret = 1;

	while (1)
	{
		while (*line && isspace(*line))
			line++;
		if (!*line)
			break;
		const char *arg = line;
		while (*line && !isspace(*line))
			line++;
		char **tmp = realloc(argv, sizeof(*argv) * (argc + 2));
		if (!tmp)
		{
			fprintf(stderr, "%s: realloc: %s\n",
			        env->progname, strerror(errno));
			goto end;
		}
		argv = tmp;
		argv[argc] = strndup(arg, line - arg);
		if (!argv[argc])
		{
			fprintf(stderr, "%s: strndup: %s\n",
			        env->progname, strerror(errno));
			goto end;
		}
		argc++;
		argv[argc] = NULL;
	}
	ret = run_cmd(env, cmds, argc, (const char**)argv);
	switch (ret)
	{
		case -2:
			ret = 1;
			break;
		case -1:
			printf("unknown command: \"%s\"\n", argv[0]);
			ret = 1;
			break;
		default:
			break;
	}

end:
	if (argv)
	{
		for (size_t i = 0; argv[i]; ++i)
			free(argv[i]);
		free(argv);
	}
	return ret;
}

static int
hist_up(void)
{
	HIST_ENTRY *entry = previous_history();
	if (!entry)
		return 0;
	rl_delete_text(0, rl_end);
	rl_insert_text(entry->line);
	return 0;
}

static int
hist_down(void)
{
	HIST_ENTRY *entry = next_history();
	if (!entry)
	{
		rl_delete_text(0, rl_end);
		return 0;
	}
	rl_delete_text(0, rl_end);
	rl_insert_text(entry->line);
	return 0;
}

static int
run_interactive(struct env *env)
{
	using_history();
	stifle_history(100);
	rl_generic_bind(ISFUNC, "\033[A", hist_up, rl_get_keymap());
	rl_generic_bind(ISFUNC, "\033[B", hist_down, rl_get_keymap());
	while (1)
	{
		char *line = readline("jdb) ");
		if (!line)
		{
			fprintf(stderr, "%s: readline: %s\n", env->progname,
			        strerror(errno));
			return 1;
		}
		if (line && *line && *line != ' ')
			add_history(line);
		while (next_history())
			;
		if (exec_line(env, line))
		{
			free(line);
			return 1;
		}
		free(line);
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

	if (!isatty(0))
	{
		fprintf(stderr, "%s must be run in a terminal\n", argv[0]);
		return EXIT_FAILURE;
	}
	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	env.child = -1;
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
	env.argc = argc - optind;
	env.argv = &argv[optind];
	if (run_interactive(&env))
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
