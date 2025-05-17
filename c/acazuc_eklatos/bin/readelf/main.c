#include "readelf.h"

#include <stdlib.h>
#include <string.h>
#include <libelf.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

static int
readelf_file(struct env *env, const char *file)
{
	struct elf32 *elf32 = NULL;
	struct elf64 *elf64 = NULL;
	int ret;

	elf32 = elf32_open(file);
	if (elf32)
	{
		 ret = print_elf32(env, elf32);
		elf32_free(elf32);
		return ret;
	}
	elf64 = elf64_open(file);
	if (elf64)
	{
		ret = print_elf64(env, elf64);
		elf64_free(elf64);
		return ret;
	}
	fprintf(stderr, "%s: failed to open elf %s: %s\n",
	        env->progname,
	        file,
	        strerror(errno));
	return 1;
}

static void
usage(const char *progname)
{
	printf("%s [-H] [-a] [-h] [-l] [-S] [-g] [-t] [-e] [-s] [-r] [-d] [-V] [-A] [-I] [-w] FILES\n", progname);
	printf("-H: show this help\n");
	printf("-a: equivalent to -h -l -S -s -r -d -V -A -I\n");
	printf("-h: display ELF header\n");
	printf("-l: display program headers\n");
	printf("-S: display section headers\n");
	printf("-g: display section groups\n");
	printf("-t: display section details\n");
	printf("-e: equivalent to -h -l -S\n");
	printf("-s: display symbol table\n");
	printf("-r: display relocations\n");
	printf("-d: display dynamic section\n");
	printf("-V: display version section\n");
	printf("-A: display architecture specific informations\n");
	printf("-I: display histogram\n");
	printf("-w: display debug sections\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	int multi;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "ahlSgtesrdVAIwH")) != -1)
	{
		switch (c)
		{
			case 'a':
				env.opt |= OPT_h | OPT_l | OPT_S | OPT_s | OPT_r | OPT_d | OPT_V | OPT_A | OPT_I;
				break;
			case 'h':
				env.opt |= OPT_h;
				break;
			case 'l':
				env.opt |= OPT_l;
				break;
			case 'S':
				env.opt |= OPT_S;
				break;
			case 'g':
				env.opt |= OPT_g;
				break;
			case 't':
				env.opt |= OPT_t;
				break;
			case 'e':
				env.opt |= OPT_h | OPT_l | OPT_S;
				break;
			case 's':
				env.opt |= OPT_s;
				break;
			case 'r':
				env.opt |= OPT_r;
				break;
			case 'd':
				env.opt |= OPT_d;
				break;
			case 'V':
				env.opt |= OPT_V;
				break;
			case 'A':
				env.opt |= OPT_A;
				break;
			case 'I':
				env.opt |= OPT_I;
				break;
			case 'w':
				env.opt |= OPT_w;
				break;
			case 'H':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (!env.opt)
	{
		fprintf(stderr, "%s: nothing to display\n", argv[0]);
		return EXIT_SUCCESS;
	}
	if (optind == argc)
	{
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	multi = optind + 1 < argc;
	for (int i = optind; i < argc; ++i)
	{
		if (multi)
			printf("==> %s <==\n", argv[i]);
		if (readelf_file(&env, argv[i]))
			return EXIT_FAILURE;
		if (multi && optind + 1 != argc)
			printf("\n");
	}
	return EXIT_SUCCESS;
}
