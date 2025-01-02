#include <string.h>
#include <signal.h>
#include <stdio.h>

const char * const sys_siglist[] =
{
	[SIGHUP]    = "Hanghup",
	[SIGINT]    = "Interrupt",
	[SIGQUIT]   = "Quit",
	[SIGILL]    = "Illegal instruction",
	[SIGTRAP]   = "Trace trap",
	[SIGABRT]   = "Aborted",
	[SIGBUS]    = "Bus error",
	[SIGFPE]    = "Floating point exception",
	[SIGKILL]   = "Killed",
	[SIGUSR1]   = "User defined signal 1",
	[SIGSEGV]   = "Segmentation fault",
	[SIGUSR2]   = "User defined signal 2",
	[SIGPIPE]   = "Broken pipe",
	[SIGALRM]   = "Alarm clock",
	[SIGTERM]   = "Terminated",
	[SIGCHLD]   = "Child existed",
	[SIGCONT]   = "Continued",
	[SIGSTOP]   = "Stopped (signal)",
	[SIGTSTP]   = "Stopped",
	[SIGTTIN]   = "Stopped (tty input)",
	[SIGTTOU]   = "Stopped (tty output)",
	[SIGURG]    = "Urgent I/O condition",
	[SIGXCPU]   = "Cpu time limit exceeded",
	[SIGXFSZ]   = "File size limit exceeded",
	[SIGVTALRM] = "Virtual timer expired",
	[SIGPROF]   = "Profiling timer alarm",
	[SIGWINCH]  = "Window size changed",
	[SIGPOLL]   = "Pollable event",
};

char *strsignal(int signum)
{
	if (signum <= 0 || (size_t)signum > sizeof(sys_siglist) / sizeof(*sys_siglist))
	{
		static char unknown_str[64];
		snprintf(unknown_str, sizeof(unknown_str), "Unknown signal %d", signum);
		return unknown_str;
	}
	return (char*)sys_siglist[signum];
}
