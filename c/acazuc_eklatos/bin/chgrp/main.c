#include <sys/param.h>
#include <sys/stat.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <grp.h>

#define OPT_R (1 << 0)
#define OPT_h (1 << 1)
#define OPT_L (1 << 2)
#define OPT_H (1 << 3)
#define OPT_P (1 << 4)

struct env
{
	const char *progname;
	int opt;
	gid_t gid;
};

static int
change_group(struct env *env, const char *file, int depth);

static int
recur(struct env *env, const char *file, int depth)
{
	char path[MAXPATHLEN];
	struct dirent *dirent;
	struct stat st;
	DIR *dir;

	if (lstat(file, &st) == -1)
	{
		fprintf(stderr, "%s: lstat(%s): %s\n",
		        env->progname,
		        file,
		        strerror(errno));
		return 1;
	}
	if (S_ISLNK(st.st_mode))
	{
		if (env->opt & OPT_P)
			return 1;
		if (env->opt & OPT_H && depth > 0)
			return 1;
	}
	else if (!S_ISDIR(st.st_mode))
	{
		return 1;
	}
	dir = opendir(file);
	if (!dir)
	{
		fprintf(stderr, "%s: opendir(%s): %s\n",
		        env->progname,
		        file,
		        strerror(errno));
		return 1;
	}
	while ((dirent = readdir(dir)))
	{
		if (!strcmp(dirent->d_name, ".") || !strcmp(dirent->d_name, ".."))
			continue;
		if (snprintf(path, sizeof(path), "%s/%s", file, dirent->d_name) >= (int)sizeof(path))
		{
			fprintf(stderr, "%s: path too long\n",
			        env->progname);
			closedir(dir);
			return 1;
		}
		if (change_group(env, path, depth + 1))
		{
			closedir(dir);
			return 1;
		}
	}
	closedir(dir);
	return 0;
}

static int
change_group(struct env *env, const char *file, int depth)
{
	if (env->opt & OPT_R)
	{
		if (recur(env, file, depth))
			return 1;
	}
	if (env->opt & OPT_h)
	{
		if (lchown(file, -1, env->gid) == -1)
		{
			fprintf(stderr, "%s: lchown(%s): %s\n",
			        env->progname,
			        file,
			        strerror(errno));
			return 1;
		}
	}
	else
	{
		if (chown(file, -1, env->gid) == -1)
		{
			fprintf(stderr, "%s: chown(%s): %s\n",
			        env->progname,
			        file,
			        strerror(errno));
			return 1;
		}
	}
	return 0;
}

static int
parse_group(const char *progname, const char *str, gid_t *gid)
{
	struct group *gr;

	gr = getgrnam(str);
	if (!gr)
	{
		fprintf(stderr, "%s: invalid group: '%s'\n", progname, str);
		return 1;
	}
	*gid = gr->gr_gid;
	return 0;
}

static void
usage(const char *progname)
{
	printf("%s [-h] [-R] [-H] [-L] [-P] group FILES\n", progname);
	printf("-h: don't dereference symbolic links\n");
	printf("-R: recursive mode\n");
	printf("-H: if an argument is a symbolic link to a directory, traverse it\n");
	printf("-L: traverse all the symbolic links to directories\n");
	printf("-P: don't traverse any symbolic link to directory\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	env.opt |= OPT_P;
	while ((c = getopt(argc, argv, "hRHLP")) != -1)
	{
		switch (c)
		{
			case 'h':
				env.opt |= OPT_h;
				break;
			case 'R':
				env.opt |= OPT_R;
				break;
			case 'H':
				env.opt |= OPT_H;
				env.opt &= ~(OPT_L | OPT_P);
				break;
			case 'L':
				env.opt |= OPT_L;
				env.opt &= ~(OPT_H | OPT_P);
				break;
			case 'P':
				env.opt |= OPT_P;
				env.opt &= ~(OPT_H | OPT_L);
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
	if (parse_group(argv[0], argv[optind], &env.gid))
		return EXIT_FAILURE;
	for (int i = optind + 1; i < argc; ++i)
	{
		if (change_group(&env, argv[i], 0))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
