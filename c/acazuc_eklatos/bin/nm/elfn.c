#include "nm.h"

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

struct sym
{
	char *name;
	ElfN_Sym sym;
};

static int sym_cmp_name(const void *a, const void *b)
{
	const struct sym *sym_a = a;
	const struct sym *sym_b = b;
	const char *name_a = sym_a->name;
	const char *name_b = sym_b->name;
	while (*name_a && !isalnum(*name_a))
		name_a++;
	while (*name_b && !isalnum(*name_b))
		name_b++;
	return strcasecmp(name_a, name_b);
}

static const ElfN_Shdr *get_sym_section(struct elfN *elf,
                                        const ElfN_Addr value)
{
	if (value == STN_UNDEF)
		return NULL;
	for (ElfN_Off i = 0; i < elfN_get_shnum(elf); ++i)
	{
		const ElfN_Shdr *shdr = elfN_get_shdr(elf, i);
		if (value >= shdr->sh_addr
		 && value < shdr->sh_addr + shdr->sh_size)
			return shdr;
	}
	return NULL;
}

static int print_symtab(struct env *env, struct elfN *elf,
                        const ElfN_Shdr *symtab_shdr,
                        struct sym **syms, ElfN_Off *syms_count,
                        ElfN_Off *syms_size)
{
	const ElfN_Shdr *strtab_shdr = NULL;
	uint8_t *symtab_data = NULL;
	uint8_t *strtab_data = NULL;
	int ret = 1;

	if (elfN_read_section(elf, symtab_shdr, (void**)&symtab_data, NULL))
	{
		fprintf(stderr, "%s: failed to read symtab section\n",
		        env->progname);
		goto end;
	}
	strtab_shdr = elfN_get_shdr(elf, symtab_shdr->sh_link);
	if (!strtab_shdr)
	{
		fprintf(stderr, "%s: invalid symtab strtab link\n",
		        env->progname);
		goto end;
	}
	if (elfN_read_section(elf, strtab_shdr, (void**)&strtab_data, NULL))
	{
		fprintf(stderr, "%s: failed to read strtab section\n",
		        env->progname);
		goto end;
	}
	for (ElfN_Off i = sizeof(ElfN_Sym); i < symtab_shdr->sh_size; i += symtab_shdr->sh_entsize)
	{
		const ElfN_Sym *sym = (const ElfN_Sym*)&symtab_data[i];
		if (ELFN_ST_TYPE(sym->st_info) == STT_FILE)
			continue;
		if (*syms_count + 1 > *syms_size)
		{
			ElfN_Off new_size = *syms_size * 2;
			if (new_size < 32)
				new_size = 32;
			struct sym *new_syms = realloc(*syms, sizeof(*new_syms) * new_size);
			if (!new_syms)
			{
				fprintf(stderr, "%s: malloc: %s\n",
				        env->progname, strerror(errno));
				goto end;
			}
			*syms = new_syms;
			*syms_size = new_size;
		}
		(*syms)[*syms_count].name = sym->st_name < strtab_shdr->sh_size ? strdup((char*)&strtab_data[sym->st_name]) : NULL;
		(*syms)[*syms_count].sym = *sym;
		(*syms_count)++;
	}
	ret = 0;

end:
	free(strtab_data);
	free(symtab_data);
	return ret;
}

int print_elfN(struct env *env, struct elfN *elf)
{
	struct sym *syms = NULL;
	int ret = 1;
	ElfN_Off syms_count = 0;
	ElfN_Off syms_size = 0;

	for (ElfN_Off i = 0; i < elfN_get_shnum(elf); ++i)
	{
		const ElfN_Shdr *shdr = elfN_get_shdr(elf, i);
		if (shdr->sh_type != SHT_SYMTAB)
			continue;
		if (print_symtab(env, elf, shdr, &syms, &syms_count, &syms_size))
			goto end;
	}
	qsort(syms, syms_count, sizeof(*syms), sym_cmp_name);
	for (ElfN_Off i = 0; i < syms_count; ++i)
	{
		struct sym *sym = &syms[i];
		if (sym->sym.st_shndx == SHN_ABS || sym->sym.st_value != STN_UNDEF)
			printf("%0*" PRIxN " ", (int)(sizeof(size_t) * 2), sym->sym.st_value);
		else
			printf("%*s ", (int)(sizeof(size_t) * 2), "");
		const ElfN_Shdr *shdr = get_sym_section(elf, sym->sym.st_value);
		char c;
		if (sym->sym.st_shndx == SHN_ABS)
			c = 'a';
		else if (!shdr)
			c = 'd';
		else if (shdr->sh_type == SHT_NOBITS)
			c = 'b';
		else if (shdr->sh_flags & SHF_EXECINSTR)
			c = 't';
		else if (!(shdr->sh_flags & SHF_WRITE))
			c = 'r';
		else
			c = 'd';
		if (ELFN_ST_BIND(sym->sym.st_info) == STB_GLOBAL)
			c = toupper(c);
		printf("%c %s\n", c, sym->name);
	}
	ret = 0;

end:
	for (ElfN_Off i = 0; i < syms_count; ++i)
		free(syms[i].name);
	free(syms);
	return ret;
}
