#include <sys/stat.h>

#if __SIZE_WIDTH__ == 32
#include <libelf32.h>
#else
#include <libelf64.h>
#endif

#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

struct env
{
	const char *progname;
	const char *base;
};

static int load_elf(struct env *env, const char *path);

static int load_dependencies(struct env *env, const char *path, int fd)
{
	struct elfN *elf = elfN_open_fd(fd);
	if (!elf)
	{
		fprintf(stderr, "%s: elf_open_fd(%s): %s\n", env->progname,
		        path, strerror(errno));
		return 1;
	}
	for (ElfN_Word i = 0; i < elfN_get_dynnum(elf); ++i)
	{
		ElfN_Dyn *dyn = elfN_get_dyn(elf, i);
		if (dyn->d_tag != DT_NEEDED)
			continue;
		const char *needed = elfN_get_dynstr_str(elf, dyn->d_un.d_val);
		if (!needed || !*needed)
			continue;
		char needed_path[MAXPATHLEN];
		if (env->base)
		{
			snprintf(needed_path, sizeof(needed_path), "%s/%s",
			         env->base, needed);
		}
		else
		{
			char needed_dir[MAXPATHLEN];
			strlcpy(needed_dir, path, sizeof(needed_dir));
			char *needed_dirname = dirname(needed_dir);
			snprintf(needed_path, sizeof(needed_path), "%s/%s",
			         needed_dirname, needed);
		}
		if (load_elf(env, needed_path))
			return 1;
	}
	elfN_free(elf);
	return 0;
}

static int load_elf(struct env *env, const char *path)
{
	int fd = open(path, O_RDONLY);
	if (fd == -1)
	{
		fprintf(stderr, "%s: open(%s): %s\n", env->progname,
		        path, strerror(errno));
		return 1;
	}
	if (load_dependencies(env, path, fd))
	{
		close(fd);
		return 1;
	}
	if (kmload(fd, NULL, 0) == -1)
	{
		close(fd);
		if (errno == EEXIST)
			return 0;
		fprintf(stderr, "%s: kmload: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	close(fd);
	return 0;
}

static void usage(const char *progname)
{
	printf("%s [-h] [-b base] FILES\n", progname);
	printf("-h: show this help\n");
	printf("-b base: set the base directory of the modules tree\n");
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "hb:")) != -1)
	{
		switch (c)
		{
			case 'b':
				env.base = optarg;
				break;
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
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
		if (load_elf(&env, argv[i]))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
