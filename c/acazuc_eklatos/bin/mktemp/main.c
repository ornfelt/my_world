#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define OPT_d (1 << 0)

struct env
{
	const char *progname;
	int opt;
};

static void
usage(const char *progname)
{
	printf("%s [-h] [-d]\n", progname);
}

int
main(int argc, char **argv)
{
	struct env env;
	unsigned rnd;
	int urandom;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "hd")) != -1)
	{
		switch (c)
		{
			case 'd':
				env.opt |= OPT_d;
				break;
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	urandom = open("/dev/urandom", O_RDONLY);
	if (urandom == -1)
	{
		fprintf(stderr, "%s: fopen(/dev/urandom): %s\n", argv[0],
		        strerror(errno));
		return EXIT_FAILURE;
	}
	if (read(urandom, &rnd, sizeof(rnd)) != sizeof(rnd))
	{
		fprintf(stderr, "%s: read: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	srand(rnd);
	if (env.opt & OPT_d)
	{
		char template[] = "/tmp/tmp.XXXXXX";
		if (!mkdtemp(template))
		{
			fprintf(stderr, "%s: mkdtemp: %s\n", argv[0], strerror(errno));
			return EXIT_FAILURE;
		}
		printf("%s\n", template);
	}
	else
	{
		char template[] = "/tmp/tmp.XXXXXX";
		if (!mktemp(template))
		{
			fprintf(stderr, "%s: mkstemp: %s\n", argv[0], strerror(errno));
			return EXIT_FAILURE;
		}
		printf("%s\n", template);
	}
	return EXIT_SUCCESS;
}
