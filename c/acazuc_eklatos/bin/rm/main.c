#include <sys/param.h>
#include <sys/stat.h>

#include <strings.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define OPT_R (1 << 0)
#define OPT_f (1 << 1)
#define OPT_i (1 << 2)
#define OPT_d (1 << 3)
#define OPT_v (1 << 4)

struct env
{
	const char *progname;
	int opt;
};

static int remove_file(struct env *env, const char *file);

static int
remove_recursive(struct env *env, const char *file)
{
	struct dirent *dirent;
	char *line = NULL;
	size_t n = 0;
	ssize_t ret;
	DIR *dir;

	if (env->opt & OPT_i)
	{
		printf("%s: remove into directory '%s'? ",
		       env->progname,
		       file);
		fflush(stdout);
		ret = getline(&line, &n, stdin);
		if (ret < 0)
		{
			fprintf(stderr, "%s: getline: %s\n",
			        env->progname,
			        strerror(errno));
			return 1;
		}
		if (strncasecmp(line, "y\n", ret) && strncasecmp(line, "yes\n", ret))
		{
			free(line);
			return 0;
		}
		free(line);
	}
	dir = opendir(file);
	if (!dir)
	{
		if (errno == ENOENT && (env->opt & OPT_f))
			return 0;
		fprintf(stderr, "%s: opendir: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	while ((dirent = readdir(dir)))
	{
		char path[MAXPATHLEN];

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
		if (remove_file(env, path))
		{
			closedir(dir);
			return 1;
		}
	}
	closedir(dir);
	return 0;
}

static int
remove_file(struct env *env, const char *file)
{
	struct stat st;

	if (lstat(file, &st) == -1)
	{
		if (errno == ENOENT && (env->opt & OPT_f))
			return 0;
		fprintf(stderr, "%s: stat: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	if (S_ISDIR(st.st_mode))
	{
		if (!(env->opt & OPT_d))
		{
			fprintf(stderr, "%s: '%s' is a directory\n",
			        env->progname,
			        file);
			return 1;
		}
		if (env->opt & OPT_R)
		{
			if (remove_recursive(env, file))
				return 1;
		}
	}
	if (env->opt & OPT_i)
	{
		const char *type;

		switch (st.st_mode & S_IFMT)
		{
			case S_IFBLK:
				type = "block device";
				break;
			case S_IFCHR:
				type = "character device";
				break;
			case S_IFDIR:
				type = "directory";
				break;
			case S_IFIFO:
				type = "fifo";
				break;
			case S_IFLNK:
				type = "symbolic link";
				break;
			case S_IFREG:
				type = "regular file";
				break;
			case S_IFSOCK:
				type = "socket";
				break;
			default:
				type = "file";
				break;
		}
		printf("%s: remove %s '%s'? ", env->progname, type, file);
		fflush(stdout);
		char *line = NULL;
		size_t n = 0;
		ssize_t ret = getline(&line, &n, stdin);
		if (ret < 0)
		{
			fprintf(stderr, "%s: getline: %s\n", env->progname, strerror(errno));
			return 1;
		}
		if (strncasecmp(line, "y\n", ret) && strncasecmp(line, "yes\n", ret))
		{
			free(line);
			return 0;
		}
		free(line);
	}
	if (env->opt & OPT_v)
		printf("unlink '%s'\n", file);
	int ret;
	if (S_ISDIR(st.st_mode))
		ret = rmdir(file);
	else
		ret = unlink(file);
	if (ret == -1 && (errno != ENOENT || !(env->opt & OPT_f)))
	{
		fprintf(stderr, "%s: unlink: %s\n", env->progname, strerror(errno));
		return 1;
	}
	return 0;
}

static void
usage(const char *progname)
{
	printf("%s [-r] [-R] [-f] [-i] [-d] [-v] FILES\n", progname);
	printf("-r: synonym of -R\n");
	printf("-R: remove files recursively, implies -d\n");
	printf("-f: ignore non existing files\n");
	printf("-i: prompt before deleting file\n");
	printf("-d: remove empty directory\n");
	printf("-v: verbose operations\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "rRfidv")) != -1)
	{
		switch (c)
		{
			case 'r':
			case 'R':
				env.opt |= OPT_R;
				env.opt |= OPT_d;
				break;
			case 'f':
				env.opt |= OPT_f;
				break;
			case 'i':
				env.opt |= OPT_i;
				break;
			case 'd':
				env.opt |= OPT_d;
				break;
			case 'v':
				env.opt |= OPT_v;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	for (int i = optind; i < argc; ++i)
	{
		if (remove_file(&env, argv[i]))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
