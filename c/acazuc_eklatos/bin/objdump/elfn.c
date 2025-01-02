#include "objdump.h"

#include <libasm/riscv.h>
#include <libasm/x86.h>

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

static int get_addr_width(const ElfN_Shdr *shdr)
{
	if (shdr->sh_addr + shdr->sh_size >= 0x10000000)
		return 8;
	if (shdr->sh_addr + shdr->sh_size >= 0x1000000)
		return 7;
	if (shdr->sh_addr + shdr->sh_size >= 0x100000)
		return 6;
	if (shdr->sh_addr + shdr->sh_size >= 0x10000)
		return 5;
	if (shdr->sh_addr + shdr->sh_size >= 0x1000)
		return 4;
	if (shdr->sh_addr + shdr->sh_size >= 0x100)
		return 3;
	if (shdr->sh_addr + shdr->sh_size >= 0x10)
		return 2;
	return 1;
}

static void print_function_name(size_t offset, struct elfN_symtab *symtab)
{
	ElfN_Sym *sym = elfN_symtab_sym(symtab, offset);
	if (!sym || ELFN_ST_TYPE(sym->st_info) != STT_FUNC)
		return;
	const char *name = elfN_symtab_str(symtab, sym);
	if (!name)
		return;
	printf("\n");
	printf("%0*zx <%s>:\n",
	       (int)(sizeof(size_t) * 2), offset,
	       name);
}

static int disas_x86(struct env *env, const ElfN_Shdr *shdr,
                     const uint8_t *data, size_t size,
                     struct elfN_symtab *symtab)
{
	int addr_width = get_addr_width(shdr);
	for (size_t i = 0; i < size;)
	{
		char buf[128];
		size_t offset = shdr->sh_addr + i;
		if (symtab)
			print_function_name(offset, symtab);
		size_t bytes = asm_x86_disas(buf, sizeof(buf), &data[i], offset);
		printf(" %*zx: ", addr_width, offset);
		for (size_t j = 0; j < bytes; ++j)
			printf("%02x ", data[i + j]);
		for (size_t j = bytes; j < 8; ++j)
			printf("   ");
		printf("%s\n", buf);
		i += bytes;
	}
	return 0;
}

static int disas_riscv(struct env *env, const ElfN_Shdr *shdr,
                       const uint8_t *data, size_t size,
                       struct elfN_symtab *symtab)
{
	int addr_width = get_addr_width(shdr);
	for (size_t i = 0; i < size;)
	{
		char buf[128];
		size_t offset = shdr->sh_addr + i;
		if (symtab)
			print_function_name(offset, symtab);
		size_t bytes = asm_riscv_disas(buf, sizeof(buf), &data[i], offset);
		printf(" %*zx: ", addr_width, offset);
		switch (bytes)
		{
			case 2:
				printf("%04" PRIx16 "    \t", *(uint16_t*)&data[i]);
				break;
			case 4:
				printf("%08" PRIx32 "\t", *(uint32_t*)&data[i]);
				break;
		}
		printf("%s\n", buf);
		i += bytes;
	}
	return 0;
}

static struct elfN_symtab *get_symtab(struct elfN *elf)
{
	/* XXX list of symtab ? */
	for (ElfN_Word i = 0; i < elfN_get_shnum(elf); ++i)
	{
		const ElfN_Shdr *shdr = elfN_get_shdr(elf, i);
		if (shdr->sh_type != SHT_SYMTAB)
			continue;
		return elfN_symtab_read(elf, shdr);
	}
	return NULL;
}

static int disas_section(struct env *env, struct elfN *elf,
                         const ElfN_Shdr *shdr)
{
	struct elfN_symtab *symtab = NULL;
	uint8_t *data;
	ElfN_Word size;
	int ret = 1;

	if (elfN_read_section(elf, shdr, (void**)&data, &size))
	{
		fprintf(stderr, "%s: failed to read section: %s\n",
		        env->progname, strerror(errno));
		goto end;
	}
	if (!size)
	{
		ret = 0;
		goto end;
	}
	symtab = get_symtab(elf);
	printf("\nDisassembly of section %s:\n\n", elfN_get_shname(elf, shdr));
	switch (elfN_get_ehdr(elf)->e_machine)
	{
		case EM_386:
			ret = disas_x86(env, shdr, data, size, symtab);
			break;
		case EM_RISCV:
			ret = disas_riscv(env, shdr, data, size, symtab);
			break;
	}

end:
	elfN_symtab_free(symtab);
	free(data);
	return ret;
}

int print_elfN(struct env *env, struct elfN *elf)
{
	int ret = 1;

	if (env->opt & OPT_d)
	{
		for (size_t i = 0; i < elfN_get_shnum(elf); ++i)
		{
			const ElfN_Shdr *shdr = elfN_get_shdr(elf, i);
			if (shdr->sh_type == SHT_PROGBITS
			 && (shdr->sh_flags & SHF_EXECINSTR))
			{
				if (disas_section(env, elf, shdr))
					goto end;
			}
		}
	}
	ret = 0;

end:
	return ret;
}
