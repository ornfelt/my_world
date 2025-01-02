#ifndef SIGNAL_H
#define SIGNAL_H

#include <types.h>

#define SIGHUP    1
#define SIGINT    2
#define SIGQUIT   3
#define SIGILL    4
#define SIGTRAP   5
#define SIGABRT   6
#define SIGBUS    7
#define SIGFPE    8
#define SIGKILL   9
#define SIGUSR1   10
#define SIGSEGV   11
#define SIGUSR2   12
#define SIGPIPE   13
#define SIGALRM   14
#define SIGTERM   15
#define SIGCHLD   17
#define SIGCONT   18
#define SIGSTOP   19
#define SIGTSTP   20
#define SIGTTIN   21
#define SIGTTOU   22
#define SIGURG    23
#define SIGXCPU   24
#define SIGXFSZ   25
#define SIGVTALRM 26
#define SIGPROF   27
#define SIGWINCH  28
#define SIGPOLL   29
#define SIGLAST   29

#define NSIG 64

#define SIG_ERR -1
#define SIG_IGN 1
#define SIG_DFL 2

#define SIG_BLOCK   1
#define SIG_UNBLOCK 2
#define SIG_SETMASK 3

#define SA_NOCLDSTOP (1 << 0)
#define SA_NODEFER   (1 << 1)
#define SA_ONSTACK   (1 << 2)
#define SA_RESETHAND (1 << 3)
#define SA_RESTART   (1 << 4)
#define SA_RESTORER  (1 << 5)
#define SA_SIGINFO   (1 << 6)

#define SS_ONSTACK (1 << 0)
#define SS_DISABLE (1 << 1)

#define MINSIGSTKSZ 4096
#define SIGSTKSZ    4096

typedef struct
{
	uint8_t set[8];
} sigset_t;

typedef void (*sighandler_t)(int);

typedef union sigval
{
	int sival_int;
	void *sival_ptr;
} sigval_t;

typedef struct siginfo
{
	int si_signo;
	int si_errno;
	int si_code;
	int si_trapno;
	pid_t si_pid;
	uid_t si_uid;
	int si_status;
	clock_t si_utime;
	clock_t si_stime;
	sigval_t si_value;
	int si_int;
	void *si_ptr;
	int si_overrun;
	int si_timerid;
	void *si_addr;
} siginfo_t;

struct sigaction
{
	union
	{
		void (*sa_handler)(int);
		void (*sa_sigaction)(int, siginfo_t *, void *);
	};
	sigset_t sa_mask;
	int sa_flags;
	void (*sa_restorer)(void);
};

typedef struct stack
{
	void *ss_sp;
	int ss_flags;
	size_t ss_size;
} stack_t;

#endif
