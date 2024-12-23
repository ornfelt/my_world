#include "cmd/cmd.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct cmd
{
	const char *name;
	int (*fn)(int argc, char **argv);
};

static const struct cmd cmds[] =
{
#define CMD(name) {#name, cmd_##name}
	CMD(asn1parse),
	CMD(base64),
	CMD(bignum),
	CMD(dgst),
	CMD(dhparam),
	CMD(dsa),
	CMD(dsaparam),
	CMD(enc),
	CMD(gendsa),
	CMD(genrsa),
	CMD(help),
	CMD(rsa),
	CMD(rsautl),
	CMD(s_client),
	CMD(speed),
	CMD(x509),
#undef CMD
};

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		print_usage();
		return EXIT_FAILURE;
	}
	for (size_t i = 0; i < sizeof(cmds) / sizeof(*cmds); ++i)
	{
		if (!strcmp(argv[1], cmds[i].name))
			return cmds[i].fn(argc - 1, argv + 1);
	}
	fprintf(stderr, "jkssl: invalid command: %s\n", argv[1]);
	print_usage();
	return EXIT_FAILURE;
}

void print_usage(void)
{
	printf("jkssl command [command options] [commands args]\n");
	printf("commands:\n");
	for (size_t i = 0; i < sizeof(cmds) / sizeof(*cmds); ++i)
		printf("  %s\n", cmds[i].name);
}
