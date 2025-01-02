#include <strings.h>
#include <libdbg.h>
#include <stdlib.h>
#include <signal.h>

static const struct dbg_signal signals[] =
{
#define SIGNAL_DEF(sig) [sig] = {#sig}
	SIGNAL_DEF(SIGHUP),
	SIGNAL_DEF(SIGINT),
	SIGNAL_DEF(SIGQUIT),
	SIGNAL_DEF(SIGILL),
	SIGNAL_DEF(SIGTRAP),
	SIGNAL_DEF(SIGABRT),
	SIGNAL_DEF(SIGBUS),
	SIGNAL_DEF(SIGFPE),
	SIGNAL_DEF(SIGKILL),
	SIGNAL_DEF(SIGUSR1),
	SIGNAL_DEF(SIGSEGV),
	SIGNAL_DEF(SIGUSR2),
	SIGNAL_DEF(SIGPIPE),
	SIGNAL_DEF(SIGALRM),
	SIGNAL_DEF(SIGTERM),
	SIGNAL_DEF(SIGCHLD),
	SIGNAL_DEF(SIGCONT),
	SIGNAL_DEF(SIGSTOP),
	SIGNAL_DEF(SIGTSTP),
	SIGNAL_DEF(SIGTTIN),
	SIGNAL_DEF(SIGTTOU),
	SIGNAL_DEF(SIGURG),
	SIGNAL_DEF(SIGXCPU),
	SIGNAL_DEF(SIGXFSZ),
	SIGNAL_DEF(SIGVTALRM),
	SIGNAL_DEF(SIGPROF),
	SIGNAL_DEF(SIGWINCH),
	SIGNAL_DEF(SIGPOLL),
#undef SIGNAL_DEF
};

const struct dbg_signal *dbg_signal_get(int signum)
{
	if (signum < 0
	 || (unsigned)signum >= sizeof(signals) / sizeof(*signals)
	 || !signals[signum].name[0])
		return NULL;
	return &signals[signum];
}

int dbg_signal_from_name(const char *name)
{
	for (size_t i = 0; i < sizeof(signals) / sizeof(*signals); ++i)
	{
		if (!strcasecmp(signals[i].name, name))
			return i;
	}
	return -1;
}
