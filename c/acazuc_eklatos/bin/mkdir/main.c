#include <sys/stat.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#define OPT_v (1 << 0)
#define OPT_p (1 << 1)

struct env
{
	const char *progname;
	int opt;
	mode_t mode;
};

static void
usage(const char *progname)
{
	printf("%s [-v] [-h] [-m mode] [-p] DIRS\n", progname);
	printf("-v: display each created directory\n");
	printf("-h: display this help\n");
	printf("-m: set the mode of created dir\n");
	printf("-p: no error if directory already exists, create parent directories\n");
}

static int
create_parents(struct env *env, const char *path)
{
	const char *it = path;

	while (*it == '/')
		it++;
	it = strchr(it, '/');
	while (it)
	{
		char tmp[MAXPATHLEN];

		snprintf(tmp, sizeof(tmp), "%.*s", (int)(it - path), path);
		if (mkdir(tmp, env->mode) == -1)
		{
			if (errno != EEXIST)
			{
				fprintf(stderr, "%s: mkdir(%s): %s\n",
				        env->progname,
				        tmp,
				        strerror(errno));
				return 1;
			}
		}
		if (env->opt & OPT_v)
			printf("%s: created directory '%s'\n",
			       env->progname,
			       tmp);
		while (*it == '/')
			it++;
		it = strchr(it + 1, '/');
	}
	return 0;
}

static int
create_dir(struct env *env, const char *path)
{
	if (mkdir(path, env->mode) == -1)
	{
		if (env->opt & OPT_p)
		{
			if (errno == ENOENT)
			{
				if (create_parents(env, path))
					return 1;
				if (mkdir(path, env->mode) != -1)
					goto end;
			}
			if (errno == EEXIST)
				return 0;
		}
		fprintf(stderr, "%s: mkdir(%s): %s\n", env->progname,
		        path, strerror(errno));
		return 1;
	}
end:
	if (env->opt & OPT_v)
		printf("%s: created directory '%s'\n", env->progname, path);
	return 0;
}

static int
parse_mode(const char *progname, const char *str, mode_t *mode)
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

int
main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	env.mode = 0777;
	while ((c = getopt(argc, argv, "m:pv")) != -1)
	{
		switch (c)
		{
			case 'v':
				env.opt |= OPT_v;
				break;
			case 'm':
				if (parse_mode(argv[0], optarg, &env.mode))
					return EXIT_FAILURE;
				break;
			case 'p':
				env.opt |= OPT_p;
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
		if (create_dir(&env, argv[i]))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
