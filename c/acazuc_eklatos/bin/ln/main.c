#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#define OPT_s (1 << 0)
#define OPT_f (1 << 1)

struct env
{
	int opt;
};

static void usage(const char *progname)
{
	printf("%s [-s] [-f] target [file]\n", progname);
	printf("-s: make symbolic link instead of hard link\n");
	printf("-f: unlink destination path if file already exists\n");
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	while ((c = getopt(argc, argv, "sf")) != -1)
	{
		switch (c)
		{
			case 's':
				env.opt |= OPT_s;
				break;
			case 'f':
				env.opt |= OPT_f;
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
	if (argc - optind > 2)
	{
		fprintf(stderr, "%s: too much operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	char *dst;
	if (argc - optind == 2)
	{
		dst = argv[optind + 1];
	}
	else
	{
		/* XXX */
		if (strchr(argv[optind], '/'))
		{
			fprintf(stderr, "%s: path without destination isn't supported\n", argv[0]);
			return EXIT_FAILURE;
		}
		dst = argv[optind];
	}
	if (env.opt & OPT_s)
	{
		int ret = symlink(argv[optind], dst);
		if (ret < 0)
		{
			if (errno == EEXIST && (env.opt & OPT_f))
			{
				ret = unlink(dst);
				if (ret < 0)
				{
					fprintf(stderr, "%s: unlink: %s\n", argv[0], strerror(errno));
					return EXIT_FAILURE;
				}
				ret = symlink(argv[optind], dst);
				if (ret < 0)
				{
					fprintf(stderr, "%s: symlink: %s\n", argv[0], strerror(errno));
					return EXIT_FAILURE;
				}
			}
			fprintf(stderr, "%s: symlink: %s\n", argv[0], strerror(errno));
			return EXIT_FAILURE;
		}
	}
	else
	{
		int ret = link(argv[optind], dst);
		if (ret < 0)
		{
			if (errno == EEXIST && (env.opt & OPT_f))
			{
				ret = unlink(dst);
				if (ret < 0)
				{
					fprintf(stderr, "%s: unlink: %s\n", argv[0], strerror(errno));
					return EXIT_FAILURE;
				}
				ret = link(argv[optind], dst);
				if (ret < 0)
				{
					fprintf(stderr, "%s: link: %s\n", argv[0], strerror(errno));
					return EXIT_FAILURE;
				}
			}
			fprintf(stderr, "%s: link: %s\n", argv[0], strerror(errno));
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}
