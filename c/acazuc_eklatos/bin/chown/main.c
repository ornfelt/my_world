#include <sys/param.h>
#include <sys/stat.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <pwd.h>
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
	uid_t uid;
	gid_t gid;
};

static int change_owner(struct env *env, const char *file, int depth);

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
			fprintf(stderr, "%s: path too long\n", env->progname);
			closedir(dir);
			return 1;
		}
		if (change_owner(env, path, depth + 1))
		{
			closedir(dir);
			return 1;
		}
	}
	closedir(dir);
	return 0;
}

static int
change_owner(struct env *env, const char *file, int depth)
{
	if (env->opt & OPT_R)
	{
		if (recur(env, file, depth))
			return 1;
	}
	if (env->opt & OPT_h)
	{
		if (lchown(file, env->uid, env->gid) == -1)
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
		if (chown(file, env->uid, env->gid) == -1)
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

static int parse_owner(const char *progname, const char *str, uid_t *uid, gid_t *gid)
{
	char owner[33] = "";
	char group[33] = "";
	const char *sc;

	*uid = -1;
	*gid = -1;
	sc = strchr(str, ':');
	if (sc)
	{
		if (strchr(sc + 1, ':'))
		{
			fprintf(stderr, "%s: invalid owner\n", progname);
			return 1;
		}
		if ((size_t)(sc - str) >= sizeof(owner))
		{
			fprintf(stderr, "%s: owner too long\n", progname);
			return 1;
		}
		strlcpy(owner, str, sc - str + 1);
		if (strlcpy(group, sc + 1, sizeof(group)) >= sizeof(group))
		{
			fprintf(stderr, "%s: group too long\n", progname);
			return 1;
		}
	}
	else
	{
		if (strlcpy(owner, str, sizeof(owner)) >= sizeof(owner))
		{
			fprintf(stderr, "%s: user too long\n", progname);
			return 1;
		}
	}
	if (owner[0])
	{
		struct passwd *pw;

		pw = getpwnam(owner);
		if (!pw)
		{
			fprintf(stderr, "%s: invalid user: '%s'\n",
			        progname,
			        owner);
			return 1;
		}
		*uid = pw->pw_uid;
	}
	if (group[0])
	{
		struct group *gr;

		gr = getgrnam(group);
		if (!gr)
		{
			fprintf(stderr, "%s: invalid group: '%s'\n",
			        progname,
			        group);
			return 1;
		}
		*gid = gr->gr_gid;
	}
	return 0;
}

static void
usage(const char *progname)
{
	printf("%s [-h] [-R] [-H] [-L] [-P] [user][:group] FILES\n", progname);
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
	if (parse_owner(argv[0], argv[optind], &env.uid, &env.gid))
		return EXIT_FAILURE;
	for (int i = optind + 1; i < argc; ++i)
	{
		if (change_owner(&env, argv[i], 0))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
