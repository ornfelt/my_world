#include <sys/resource.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#define OPT_n (1 << 0)

struct env
{
	int opt;
	long increment;
};

static void
usage(const char *progname)
{
	printf("%s [-h] [-n increment] [COMMAND [ARGS]]\n", progname);
	printf("-h          : display this help\n");
	printf("-n increment: adjust the priority by given increment\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	int prio;
	int c;

	memset(&env, 0, sizeof(env));
	env.increment = 10;
	while ((c = getopt(argc, argv, "hn:")) != -1)
	{
		switch (c)
		{
			case 'n':
			{
				env.opt |= OPT_n;
				errno = 0;
				char *endptr;
				env.increment = strtol(optarg, &endptr, 10);
				if (errno || *endptr)
				{
					fprintf(stderr, "%s: invalid increment\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				break;
			}
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	prio = getpriority(PRIO_PROCESS, getpid());
	if (prio == -1)
	{
		fprintf(stderr, "%s: getpriority: %s\n",
		        argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	if (optind == argc)
	{
		if (env.opt & OPT_n)
		{
			fprintf(stderr, "%s: a command must be given when adjustment is given\n",
			        argv[0]);
			return EXIT_FAILURE;
		}
		printf("%d\n", prio);
		return EXIT_SUCCESS;
	}
	if (setpriority(PRIO_PROCESS, getpid(), prio + env.increment) == -1)
	{
		fprintf(stderr, "%s: setpriority: %s\n", argv[0],
		        strerror(errno));
		return EXIT_FAILURE;
	}
	execvp(argv[optind], &argv[optind]);
	fprintf(stderr, "%s: execvp: %s\n", argv[0], strerror(errno));
	return EXIT_FAILURE;
}
