#ifndef READELF_H
#define READELF_H

#define OPT_h (1 << 0)
#define OPT_l (1 << 1)
#define OPT_S (1 << 2)
#define OPT_g (1 << 3)
#define OPT_t (1 << 4)
#define OPT_s (1 << 5)
#define OPT_r (1 << 6)
#define OPT_d (1 << 7)
#define OPT_V (1 << 8)
#define OPT_A (1 << 9)
#define OPT_I (1 << 10)
#define OPT_w (1 << 11)

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
