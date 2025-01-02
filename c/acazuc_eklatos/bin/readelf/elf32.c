#include "readelf.h"

#include <libelf32.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define print_elfN print_elf32

#define PRIdN PRId32
#define PRIuN PRIu32
#define PRIxN PRIx32

static int print_program_headers(struct env *env, struct elfN *elf)
{
	if (!(env->opt & OPT_h))
	{
		printf("\n");
		printf("Elf file type is %s\n", elf_ehdr_type_str(elfN_get_ehdr(elf)->e_type));
		printf("Entry point %#" PRIx32 "\n", elfN_get_ehdr(elf)->e_entry);
		printf("There are %" PRIu16 " program headers, starting at offset %" PRIu32 "\n",
		       elfN_get_phnum(elf), elfN_get_phoff(elf));
	}
	printf("\n");
	printf("Program Headers:\n");
	printf("  %-14s %-8s %-10s %-10s %-7s %-7s %-3s %s\n", "Type", "Offset", "VirtAddr", "PhysAddr", "FileSiz", "MemSiz", "Flg", "Align");
	for (ElfN_Word i = 0; i < elfN_get_phnum(elf); ++i)
	{
		const ElfN_Phdr *phdr = elfN_get_phdr(elf, i);
		printf("  %-14s", elf_phdr_type_str(phdr->p_type));
		printf(" 0x%06" PRIx32, phdr->p_offset);
		printf(" 0x%08" PRIx32, phdr->p_vaddr);
		printf(" 0x%08" PRIx32, phdr->p_paddr);
		printf(" 0x%05" PRIx32, phdr->p_filesz);
		printf(" 0x%05" PRIx32, phdr->p_memsz);
		printf(" %s", elf_phdr_flags_str(phdr->p_flags));
		printf(" 0x%" PRIx32, phdr->p_align);
		printf("\n");
	}
	printf("\n");
	printf(" Section to Segment mapping:\n");
	printf("  Segment Sections...\n");
	for (ElfN_Word i = 0; i < elfN_get_phnum(elf); ++i)
	{
		const ElfN_Phdr *phdr = elfN_get_phdr(elf, i);
		char sections[512] = "";
		for (ElfN_Word j = 0; j < elfN_get_shnum(elf); ++j)
		{
			const ElfN_Shdr *shdr = elfN_get_shdr(elf, j);
			if (shdr->sh_type != SHT_NULL
			 && shdr->sh_offset < phdr->p_offset + phdr->p_filesz
			 && shdr->sh_offset + shdr->sh_size >= phdr->p_offset)
			{
				strlcat(sections, elfN_get_shname(elf, shdr), sizeof(sections));
				strlcat(sections, " ", sizeof(sections));
			}
		}
		printf("   %02" PRIdN "     %s\n", i, sections);
	}
	return 0;
}

static int print_program_sections(struct env *env, struct elfN *elf)
{
	char buf[64];
	if (!(env->opt & OPT_h))
		printf("There are %" PRIu16 " section headers, starting at offset %" PRIu32 "\n",
		       elfN_get_shnum(elf), elfN_get_shoff(elf));
	printf("\n");
	printf("Section Headers:\n");
	printf("  [Nr] %-17s %-15s %-8s %-6s %-6s %2s %3s %2s %3s %2s\n", "Name", "Type", "Addr", "Off", "Size", "ES", "Flg", "Lk", "Inf", "Al");
	for (ElfN_Word i = 0; i < elfN_get_shnum(elf); ++i)
	{
		const ElfN_Shdr *shdr = elfN_get_shdr(elf, i);
		printf("  [%2" PRIdN "]", i);
		printf(" %-17.17s", elfN_get_shname(elf, shdr));
		printf(" %-15.15s", elf_shdr_type_str(shdr->sh_type));
		printf(" %08" PRIx32, shdr->sh_addr);
		printf(" %06" PRIx32, shdr->sh_offset);
		printf(" %06" PRIx32, shdr->sh_size);
		printf(" %02" PRIx32, shdr->sh_entsize);
		printf(" %3s", elf_shdr_flags_str(buf, sizeof(buf), shdr->sh_flags));
		printf(" %2" PRId32, shdr->sh_link);
		printf(" %3" PRId32, shdr->sh_info);
		printf(" %2" PRId32, shdr->sh_addralign);
		printf("\n");
	}
	printf("Key to Flags:\n");
	printf("  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),\n");
	printf("  L (link order), O (extra OS processing required), G (group), T (TLS),\n");
	printf("  C (compressed), x (unknown), o (OS specific), E (exclude),\n");
	printf("  p (processor specific)\n");
	return 0;
}

#include "elfn.c"
