#ifndef OBJDUMP_H
#define OBJDUMP_H

#define OPT_d (1 << 0)

struct env
{
	const char *progname;
	int opt;
};

struct elf32;
struct elf64;

int print_elf32(struct env *env, struct elf32 *elf);
int print_elf64(struct env *env, struct elf64 *elf);

#endif
