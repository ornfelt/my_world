#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

static int parse_signal(const char *progname, const char *str, int *sig)
{
	char *endptr;
	errno = 0;
	*sig = strtol(str, &endptr, 0);
	if (errno || *endptr)
	{
		fprintf(stderr, "%s: invalid signal\n", progname);
		return 1;
	}
	return 0;
}

static void usage(const char *progname)
{
	printf("%s [-h] [-s signal] pid\n", progname);
	printf("-h       : show this help\n");
	printf("-s signal: send the given signal to the pid\n");
}

int main(int argc, char **argv)
{
	int sig = SIGTERM;
	pid_t pid;
	int c;

	while ((c = getopt(argc, argv, "s:h")) != -1)
	{
		switch (c)
		{
			case 's':
				if (parse_signal(argv[0], optarg, &sig))
					return EXIT_FAILURE;
				break;
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
	if (argc - optind > 1)
	{
		fprintf(stderr, "%s: extra operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	char *endptr;
	errno = 0;
	pid = strtol(argv[optind], &endptr, 0);
	if (errno || *endptr)
	{
		fprintf(stderr, "%s: invalid pid\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (kill(pid, sig) == -1)
	{
		fprintf(stderr, "%s: kill: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	return EXIT_FAILURE;
}
