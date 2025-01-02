#include <sys/resource.h>
#include <sys/wait.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

static void print_time(const char *name, struct timeval *tv)
{
	char buf[64];
	long long min = tv->tv_sec / 60;
	long long sec = tv->tv_sec % 60;
	long long ms = tv->tv_usec / 1000;
	snprintf(buf, sizeof(buf), "%lldm%lld,%03llds", min, sec, ms);
	printf("%s\t%s\n", name, buf);
}

static void usage(const char *progname)
{
	printf("%s [-h] PROGRAM\n", progname);
	printf("-h: show this help\n");
}

int main(int argc, char **argv)
{
	struct rusage rusage;
	struct timespec begin;
	struct timespec end;
	struct timeval duration;
	pid_t pid;
	int wstatus;
	int ret;
	int c;

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
	if (optind == argc)
	{
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	pid = vfork();
	if (pid == -1)
	{
		fprintf(stderr, "%s: vfork: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	if (!pid)
	{
		clock_gettime(CLOCK_MONOTONIC, &begin);
		execvp(argv[optind], &argv[optind]);
		fprintf(stderr, "%s: execve: %s\n", argv[0], strerror(errno));
		_exit(EXIT_FAILURE);
	}
	while (1)
	{
		if (wait4(pid, &wstatus, 0, &rusage) == -1)
		{
			if (errno == EINTR)
				continue;
			fprintf(stderr, "%s: waitpid: %s\n", argv[0], strerror(errno));
			return EXIT_FAILURE;
		}
		clock_gettime(CLOCK_MONOTONIC, &end);
		break;
	}
	if (WIFEXITED(wstatus))
		ret = WEXITSTATUS(wstatus);
	else
		ret = EXIT_FAILURE;
	duration.tv_sec = end.tv_sec - begin.tv_sec;
	if (end.tv_nsec >= begin.tv_nsec)
	{
		duration.tv_usec = (end.tv_nsec - begin.tv_nsec) / 1000;
	}
	else
	{
		duration.tv_usec = 1000000 - (begin.tv_nsec - end.tv_nsec) / 1000;
		duration.tv_sec--;
	}
	print_time("real", &duration);
	print_time("user", &rusage.ru_utime);
	print_time("sys", &rusage.ru_stime);
	return ret;
}
