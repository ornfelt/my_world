#include <sys/param.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#define OPT_n (1 << 0)

struct env
{
	int opt;
};

static void
usage(const char *progname)
{
	printf("%s [-n] FILES\n", progname);
	printf("-n: don't output newline after each file\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	while ((c = getopt(argc, argv, "n")) != -1)
	{
		switch (c)
		{
			case 'n':
				env.opt |= OPT_n;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (optind >= argc)
	{
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (optind + 1 < argc && (env.opt & OPT_n))
	{
		fprintf(stderr, "multiple files, ignoring -n\n");
		env.opt &= ~OPT_n;
	}
	for (int i = optind; i < argc; ++i)
	{
		char path[MAXPATHLEN];
		int ret;

		ret = readlink(argv[i], path, sizeof(path));
		if (ret < 0)
		{
			fprintf(stderr, "%s: readlink: %s\n", argv[0], strerror(errno));
			return EXIT_FAILURE;
		}
		if ((size_t)ret >= sizeof(path))
		{
			fprintf(stderr, "%s: path too long\n", argv[0]);
			return EXIT_FAILURE;
		}
		path[ret] = '\0';
		/* stdio is catastrophic */
		if (env.opt & OPT_n)
			fputs(path, stdout);
		else
			puts(path);
	}
	return EXIT_SUCCESS;
}
