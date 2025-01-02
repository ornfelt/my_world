#include <inttypes.h>
#include <stdlib.h>
#include <getopt.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

struct env
{
	const char *progname;
};

static void usage(const char *progname)
{
	printf("%s [-h]\n", progname);
	printf("-h: show this help\n");
}

static int get_module_stats(struct env *env, int dirfd, const char *name,
                            uint64_t *sizep, uint64_t *addrp, char **deps)
{
	char *line = NULL;
	size_t size = 0;
	FILE *fp = NULL;
	int ret = 1;
	int fd = -1;

	fd = openat(dirfd, name, O_RDONLY);
	if (fd == -1)
	{
		fprintf(stderr, "%s: open(%s): %s\n", env->progname,
		        name, strerror(errno));
		goto end;
	}
	fp = fdopen(fd, "r");
	if (!fp)
	{
		fprintf(stderr, "%s: fdopen: %s\n", env->progname,
		        strerror(errno));
		goto end;
	}
	*sizep = 0;
	*addrp = 0;
	*deps = NULL;
	fd = -1;
	while (getline(&line, &size, fp) > 0)
	{
		if (!strncmp(line, "map: ", 5))
		{
			char *endptr;
			errno = 0;
			size_t start;
			size_t end;
			start = strtoull(&line[5], &endptr, 0);
			if (errno || *endptr != '-')
			{
				fprintf(stderr, "%s: invalid module line\n",
				        env->progname);
				goto end;
			}
			end = strtoull(&endptr[1], &endptr, 0);
			if (errno || *endptr != '\n')
			{
				fprintf(stderr, "%s: invalid module line\n",
				        env->progname);
				goto end;
			}
			*addrp = start;
			*sizep = end - start;
		}
		else if (!strncmp(line, "dependencies: ", 14))
		{
			*deps = strdup(&line[14]);
			if (!*deps)
			{
				fprintf(stderr, "%s: dependencies allocation failed",
				        env->progname);
				goto end;
			}
			size_t len = strlen(*deps);
			if (len > 0 && (*deps)[len - 1] == '\n')
				(*deps)[len - 1] = '\0';
		}
	}
	ret = 0;

end:
	if (fp)
		fclose(fp);
	free(line);
	if (fd != -1)
		close(fd);
	return ret;
}

static int is_dir(DIR *dir, struct dirent *dirent)
{
	switch (dirent->d_type)
	{
		case DT_DIR:
			return 1;
		case DT_LNK:
		case DT_UNKNOWN:
		{
			struct stat st;
			if (fstatat(dirfd(dir), dirent->d_name, &st, 0) == -1)
				return -1;
			return S_ISDIR(st.st_mode) != 0;
		}
		default:
			return 0;
	}
}

static int list_modules(struct env *env, const char *path)
{
	struct dirent *dirent;
	DIR *dir;
	int ret = 1;

	dir = opendir(path);
	if (!dir)
	{
		fprintf(stderr, "%s: opendir(%s): %s\n", env->progname,
		        path, strerror(errno));
		goto end;
	}
	while ((dirent = readdir(dir)))
	{
		if (!strcmp(dirent->d_name, ".") || !strcmp(dirent->d_name, ".."))
			continue;
		if (is_dir(dir, dirent))
		{
			char dir_path[MAXPATHLEN];
			snprintf(dir_path, sizeof(dir_path), "%s/%s", path,
			         dirent->d_name);
			list_modules(env, dir_path);
			continue;
		}
		uint64_t size;
		uint64_t addr;
		char *deps;
		if (get_module_stats(env, dirfd(dir), dirent->d_name, &size,
		                     &addr, &deps))
			goto end;
		printf("%-20s %#8" PRIx64 " %#18" PRIx64 " %s\n",
		       dirent->d_name, size, addr, deps ? deps : "");
		free(deps);
	}
	ret = 0;

end:
	if (dir)
		closedir(dir);
	return ret;
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "h")) != -1)
	{
		switch (c)
		{
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	printf("%-20s %8s %18s %s\n", "name", "size", "address", "dependencies");
	if (list_modules(&env, "/sys/kmod"))
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
