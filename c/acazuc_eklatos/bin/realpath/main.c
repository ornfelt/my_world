#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

struct env
{
	const char *progname;
};

static void print_realpath(struct env *env, const char *file)
{
	char *ret = realpath(file, NULL);
	if (!ret)
	{
		fprintf(stderr, "%s: malloc: %s\n", env->progname, strerror(errno));
		return;
	}
	puts(ret);
	free(ret);
}

static void usage(const char *progname)
{
	printf("%s FILES\n", progname);
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "")) != -1)
	{
		switch (c)
		{
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
		print_realpath(&env, argv[i]);
	return EXIT_SUCCESS;
}
