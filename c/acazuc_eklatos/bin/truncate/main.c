#include <sys/stat.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#define OPT_c (1 << 0)
#define OPT_o (1 << 1)
#define OPT_r (1 << 2)

struct env
{
	const char *progname;
	int opt;
	unsigned size;
};

static int
truncate_file(struct env *env, const char *file)
{
	if (env->opt & OPT_o)
	{
		struct stat st;

		if (stat(file, &st) == -1)
		{
			fprintf(stderr, "%s: stat(%s): %s\n",
			        env->progname,
			        file,
			        strerror(errno));
			return 1;
		}
		if (truncate(file, env->size * st.st_blksize) == -1)
		{
			fprintf(stderr, "%s: truncate(%s): %s\n",
			        env->progname,
			        file,
			        strerror(errno));
			return 1;
		}
		return 0;
	}
	if (truncate(file, env->size) == -1)
	{
		fprintf(stderr, "%s: truncate(%s): %s\n",
		        env->progname,
		        file,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static int
get_file_size(struct env *env, const char *file)
{
	struct stat st;

	if (stat(file, &st) == -1)
	{
		fprintf(stderr, "%s: stat(%s): %s\n",
		        env->progname,
		        file,
		        strerror(errno));
		return 1;
	}
	env->size = st.st_size;
	return 0;
}

static int parse_size(struct env *env, const char *str)
{
	char *endptr;
	errno = 0;

	env->size = strtol(str, &endptr, 10);
	if (errno)
	{
		fprintf(stderr, "%s: strtoul: %s",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	if (*endptr)
	{
		fprintf(stderr, "%s: invalid number\n", env->progname);
		return 1;
	}
	return 0;
}

static void
usage(const char *progname)
{
	printf("%s [-c] [-o] [-r file] [-s size] FILES\n", progname);
	printf("-c     : don't create files\n");
	printf("-o     : interpret size as a number of IO blocks\n");
	printf("-r file: use file as reference size\n");
	printf("-s size: size to be set\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "cor:s:")) != -1)
	{
		switch (c)
		{
			case 'c':
				env.opt |= OPT_c;
				break;
			case 'o':
				env.opt |= OPT_o;
				break;
			case 'r':
				if (get_file_size(&env, optarg))
					return EXIT_FAILURE;
				env.opt |= OPT_r;
				break;
			case 's':
				if (parse_size(&env, optarg))
					return EXIT_FAILURE;
				env.opt &= ~OPT_r;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (env.opt & OPT_r)
		env.opt &= ~OPT_o;
	if (optind == argc)
	{
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	for (int i = optind; i < argc; ++i)
	{
		if (truncate_file(&env, argv[i]))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
