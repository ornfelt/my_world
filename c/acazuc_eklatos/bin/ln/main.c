#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#define OPT_s (1 << 0)
#define OPT_f (1 << 1)

struct env
{
	const char *progname;
	int opt;
};

static void
usage(const char *progname)
{
	printf("%s [-s] [-f] target [file]\n", progname);
	printf("-s: make symbolic link instead of hard link\n");
	printf("-f: unlink destination path if file already exists\n");
}

static int
link_sym(struct env *env, const char *src, const char *dst)
{
	if (!symlink(src, dst))
		return 0;
	if (errno == EEXIST && (env->opt & OPT_f))
	{
		if (unlink(dst) == -1)
		{
			fprintf(stderr, "%s: unlink(%s): %s\n",
			        env->progname,
			        dst,
			        strerror(errno));
			return 1;
		}
		if (symlink(src, dst) == -1)
		{
			fprintf(stderr, "%s: symlink(%s, %s): %s\n",
			        env->progname,
			        src,
			        dst,
			        strerror(errno));
			return 1;
		}
	}
	fprintf(stderr, "%s: symlink: %s\n",
	        env->progname,
	        strerror(errno));
	return 1;
}

static int
link_hard(struct env *env, const char *src, const char *dst)
{
	if (!link(src, dst))
		return 0;
	if (errno == EEXIST && (env->opt & OPT_f))
	{
		if (unlink(dst) == -1)
		{
			fprintf(stderr, "%s: unlink(%s): %s\n",
			        env->progname,
			        dst,
			        strerror(errno));
			return 1;
		}
		if (link(src, dst) == -1)
		{
			fprintf(stderr, "%s: link(%s, %s): %s\n",
			        env->progname,
			        src,
			        dst,
			        strerror(errno));
			return 1;
		}
	}
	fprintf(stderr, "%s: link(%s): %s\n",
	        env->progname,
	        dst,
	        strerror(errno));
	return 1;
}

int
main(int argc, char **argv)
{
	struct env env;
	char *dst;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
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
		if (link_sym(&env, argv[optind], dst))
			return EXIT_FAILURE;
	}
	else
	{
		if (link_hard(&env, argv[optind], dst))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
