#include "nm.h"

#include <unistd.h>
#include <libelf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int print_elf32(struct env *env, struct elf32 *elf);
int print_elf64(struct env *env, struct elf64 *elf);

static int nm_file(struct env *env, const char *file)
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
	printf("-h: display this help\n");
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
	int multi = optind + 1 < argc;
	for (int i = optind; i < argc; ++i)
	{
		if (multi)
			printf("%s:\n", argv[i]);
		if (nm_file(&env, argv[i]))
			return EXIT_FAILURE;
		if (multi && optind + 1 != argc)
			printf("\n");
	}
	return EXIT_SUCCESS;
}
