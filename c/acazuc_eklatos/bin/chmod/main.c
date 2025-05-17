#include <sys/param.h>
#include <sys/stat.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>

#define OPT_R (1 << 0)
#define OPT_h (1 << 1)

struct env
{
	const char *progname;
	int opt;
	mode_t mode;
};

static int change_mode(struct env *env, const char *file);

static int
change_recursive(struct env *env, const char *file)
{
	char path[MAXPATHLEN];
	struct dirent *dirent;
	DIR *dir;

	dir = opendir(file);
	if (!dir)
	{
		if (errno != ENOTDIR)
		{
			fprintf(stderr, "%s: opendir(%s): %s\n",
			        env->progname,
			        file,
			        strerror(errno));
			return 1;
		}
		return 0;
	}
	while ((dirent = readdir(dir)))
	{
		if (!strcmp(dirent->d_name, ".")
		 || !strcmp(dirent->d_name, ".."))
			continue;
		if (snprintf(path, sizeof(path), "%s/%s", file, dirent->d_name) >= (int)sizeof(path))
		{
			fprintf(stderr, "%s: path too long\n",
			        env->progname);
			closedir(dir);
			return 1;
		}
		if (change_mode(env, path))
		{
			closedir(dir);
			return 1;
		}
	}
	closedir(dir);
	return 0;
}

static int
change_mode(struct env *env, const char *file)
{
	if (env->opt & OPT_R)
	{
		if (change_recursive(env, file))
			return 1;
	}
	if (env->opt & OPT_h)
	{
		if (lchmod(file, env->mode) == -1)
		{
			fprintf(stderr, "%s: lchmod(%s): %s\n",
			        env->progname,
			        file,
			        strerror(errno));
			return 1;
		}
	}
	else
	{
		if (chmod(file, env->mode) == -1)
		{
			fprintf(stderr, "%s: chmod(%s): %s\n",
			        env->progname,
			        file,
			        strerror(errno));
			return 1;
		}
	}
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

static void
usage(const char *progname)
{
	printf("%s [-h] [-R] mode FILES\n", progname);
	printf("-h: don't dereference symbolic links\n");
	printf("-R: recursive mode\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "hR")) != -1)
	{
		switch (c)
		{
			case 'h':
				env.opt |= OPT_h;
				break;
			case 'R':
				env.opt |= OPT_R;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (argc - optind < 2)
	{
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (parse_mode(argv[0], argv[optind], &env.mode))
		return EXIT_FAILURE;
	for (int i = optind + 1; i < argc; ++i)
	{
		if (change_mode(&env, argv[i]))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
