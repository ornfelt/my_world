#ifndef LDD_H
#define LDD_H

#include <stdbool.h>
#include <stddef.h>

struct env
{
	const char *progname;
};

struct elf32;
struct elf64;

int print_elf32(struct env *env, struct elf32 *elf);
int print_elf64(struct env *env, struct elf64 *elf);

bool get_path(char *buf, size_t size, const char *name);

#endif
