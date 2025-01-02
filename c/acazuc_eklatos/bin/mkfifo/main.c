#include <sys/stat.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

struct env
{
	const char *progname;
	mode_t mode;
};

static void usage(const char *progname)
{
	printf("%s [-m mode] FILES\n", progname);
	printf("-m: set the mode of created fifo\n");
}

static int create_fifo(struct env *env, const char *path)
{
	int ret = mkfifo(path, env->mode);
	if (ret < 0)
	{
		fprintf(stderr, "%s: mkfifo: %s\n", env->progname, strerror(errno));
		return 1;
	}
	return 0;
}

static int parse_mode(const char *progname, const char *str, mode_t *mode)
{
	*mode = 0;
	for (size_t i = 0; str[i]; ++i)
	{
		if (str[i] < '0' || str[i] > '7')
		{
			fprintf(stderr, "%s: invalid mode\n", progname);
			return 1;
		}
		*mode = *mode * 8 + str[i] - '0';
		if (*mode > 07777)
		{
			fprintf(stderr, "%s: mode out of bounds\n", progname);
			return 1;
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	env.mode = 0777;
	while ((c = getopt(argc, argv, "m:")) != -1)
	{
		switch (c)
		{
			case 'm':
				if (parse_mode(argv[0], optarg, &env.mode))
					return EXIT_FAILURE;
				break;
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
	for (int i = optind; i < argc; ++i)
	{
		if (create_fifo(&env, argv[i]))
			return EXIT_FAILURE;
	}
	return 0;
}
