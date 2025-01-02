#include "ldd.h"

#include <libelf.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>

bool get_path(char *buf, size_t size, const char *name)
{
	char *library_path = getenv("LD_LIBRARY_PATH");
	if (!library_path)
		library_path = "/lib";
	char *it = library_path;
	char *sp;
	while ((sp = strchrnul(it, ':')))
	{
		if (sp != it + 1)
		{
			if (snprintf(buf, size, "%.*s/%s", (int)(sp - it), it, name) >= (int)size)
				return false;
			int fd = open(buf, O_RDONLY);
			if (fd != -1)
			{
				close(fd);
				return true;
			}
		}
		if (!*sp)
			break;
		it = sp + 1;
	}
	return false;
}

static int ldd_file(struct env *env, const char *file)
{
	struct elf32 *elf32 = NULL;
	struct elf64 *elf64 = NULL;

	elf32 = elf32_open(file);
	if (elf32)
	{
		int ret = print_elf32(env, elf32);
		elf32_free(elf32);
		return ret;
	}
	elf64 = elf64_open(file);
	if (elf64)
	{
		int ret = print_elf64(env, elf64);
		elf64_free(elf64);
		return ret;
	}
	fprintf(stderr, "%s: failed to open elf: %s\n",
	        env->progname, strerror(errno));
	return 1;
}

static void usage(const char *progname)
{
	printf("%s [-h] FILES\n", progname);
	printf("-h: show this help\n");
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
	if (optind == argc)
	{
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	int multi = optind + 1 < argc;
	for (int i = optind; i < argc; ++i)
	{
		if (multi)
			printf("==> %s <==\n", argv[i]);
		if (ldd_file(&env, argv[i]))
			return EXIT_FAILURE;
		if (multi && optind + 1 != argc)
			printf("\n");
	}
	return EXIT_SUCCESS;
}
