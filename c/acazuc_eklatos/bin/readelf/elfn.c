#include <sys/endian.h>

#include <libdwarf.h>

struct buf
{
	uint8_t *data;
	size_t size;
	size_t pos;
};

static void
buf_init(struct buf *buf, void *data, size_t size)
{
	buf->data = data;
	buf->size = size;
	buf->pos = 0;
}

static int
buf_u8(struct buf *buf, uint8_t *v)
{
	if (buf->pos + 1 > buf->size)
		return 1;
	*v = buf->data[buf->pos];
	buf->pos += 1;
	return 0;
}

static int
buf_u16(struct buf *buf, uint16_t *v)
{
	if (buf->pos + 2 > buf->size)
		return 1;
	*v = le16dec(&buf->data[buf->pos]);
	buf->pos += 2;
	return 0;
}

static int
buf_u32(struct buf *buf, uint32_t *v)
{
	if (buf->pos + 4 > buf->size)
		return 1;
	*v = le32dec(&buf->data[buf->pos]);
	buf->pos += 4;
	return 0;
}

static int
buf_u64(struct buf *buf, uint64_t *v)
{
	if (buf->pos + 8 > buf->size)
		return 1;
	*v = le64dec(&buf->data[buf->pos]);
	buf->pos += 8;
	return 0;
}

static int
print_ehdr(ElfN_Ehdr *ehdr)
{
	printf("ELF Header:\n");
	printf("  Magic:  ");
	for (ElfN_Word i = 0 ; i < EI_NIDENT; ++i)
		printf(" %02x", ehdr->e_ident[i]);
	printf("\n");
	printf("  Class:                             %s\n", elf_ehdr_class_str(ehdr->e_ident[EI_CLASS]));
	printf("  Data:                              %s\n", elf_ehdr_data_str(ehdr->e_ident[EI_DATA]));
	printf("  Version:                           %" PRIu8 "\n", ehdr->e_ident[EI_VERSION]);
	printf("  OS/ABI:                            %s\n", elf_ehdr_abi_str(ehdr->e_ident[EI_OSABI]));
	printf("  ABI Version:                       %" PRIu8 "\n", ehdr->e_ident[EI_ABIVERSION]);
	printf("  Type:                              %s\n", elf_ehdr_type_str(ehdr->e_type));
	printf("  Machine:                           %s\n", elf_ehdr_machine_str(ehdr->e_machine));
	printf("  Version:                           0x%" PRIx32 "\n", ehdr->e_version);
	printf("  Entry point address:               0x%" PRIxN "\n", ehdr->e_entry);
	printf("  Start of program headers:          %" PRIuN " (bytes into file)\n", ehdr->e_phoff);
	printf("  Start of section headers:          %" PRIuN " (bytes into file)\n", ehdr->e_shoff);
	printf("  Flags:                             0x%" PRIx32 "\n", ehdr->e_flags);
	printf("  Size of this header:               %" PRIu16 " (bytes)\n", ehdr->e_ehsize);
	printf("  Size of program headers:           %" PRIu16 " (bytes)\n", ehdr->e_phentsize);
	printf("  Number of program headers:         %" PRIu16 "\n", ehdr->e_phnum);
	printf("  Size of section headers:           %" PRIu16 " (bytes)\n", ehdr->e_shentsize);
	printf("  Number of section headers:         %" PRIu16 "\n", ehdr->e_shnum);
	printf("  Section header string table index: %" PRIu16 "\n", ehdr->e_shstrndx);
	return 0;
}

static int
print_dyn(struct elfN *elf, const ElfN_Dyn *dyn)
{
	const char *name = elf_dyn_str(dyn->d_tag);
	printf(" 0x%08" PRIxN " (%s)", dyn->d_tag, name);
	printf("%*s", (int)(28 - strlen(name) - 2), "");
	switch (dyn->d_tag)
	{
		case DT_NULL:
		case DT_PLTGOT:
		case DT_HASH:
		case DT_STRTAB:
		case DT_SYMTAB:
		case DT_RELA:
		case DT_RELAENT:
		case DT_INIT:
		case DT_FINI:
		case DT_RPATH:
		case DT_SYMBOLIC:
		case DT_REL:
		case DT_DEBUG:
		case DT_TEXTREL:
		case DT_JMPREL:
		case DT_INIT_ARRAY:
		case DT_FINI_ARRAY:
		case DT_GNU_HASH:
		case DT_VERNEED:
		case DT_VERSYM:
		case DT_VERDEF:
			printf(" 0x%" PRIxN "\n", dyn->d_un.d_ptr);
			break;
		case DT_PLTRELSZ:
		case DT_RELASZ:
		case DT_STRSZ:
		case DT_SYMENT:
		case DT_RELSZ:
		case DT_RELENT:
		case DT_INIT_ARRAYSZ:
		case DT_FINI_ARRAYSZ:
			printf(" %" PRIuN " (bytes)\n", dyn->d_un.d_val);
			break;
		case DT_RELACOUNT:
		case DT_RELCOUNT:
		case DT_VERNEEDNUM:
		case DT_VERDEFNUM:
			printf(" %" PRIuN "\n", dyn->d_un.d_val);
			break;
		case DT_NEEDED:
			printf(" Shared library: [%s]\n",
			       elfN_get_dynstr_str(elf, dyn->d_un.d_val));
			break;
		case DT_SONAME:
			printf(" Library soname: [%s]\n",
			       elfN_get_dynstr_str(elf, dyn->d_un.d_val));
			break;
		case DT_PLTREL:
			if (dyn->d_un.d_val == DT_REL)
				printf(" REL\n");
			else if (dyn->d_un.d_val == DT_RELA)
				printf(" RELA\n");
			else
				printf(" 0x%" PRIxN "\n", dyn->d_un.d_ptr);
			break;
		case DT_FLAGS:
		{
			char buf[128];
			printf(" %s\n", elf_dyn_flags_str(buf, sizeof(buf),
			                                  dyn->d_un.d_val));
			break;
		}
		case DT_FLAGS_1:
		{
			char buf[128];
			printf(" Flags: %s\n", elf_dyn_flags1_str(buf, sizeof(buf),
			                                          dyn->d_un.d_val));
			break;
		}
		case DT_BIND_NOW:
			printf("\n");
			break;
	}
	return 0;
}

static int
print_dynamic_section(struct env *env,
                      struct elfN *elf,
                      const ElfN_Shdr *shdr)
{
	uint8_t *data;

	if (elfN_read_section(elf, shdr, (void**)&data, NULL))
	{
		fprintf(stderr, "%s: failed to read section: %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	printf("\n");
	printf("Dynamic section at offset 0x%" PRIxN " contains %" PRIdN " entries:\n",
	       shdr->sh_offset,
	       shdr->sh_size / shdr->sh_entsize);
	printf(" %-10s %-28s %s\n", " Tag", " Type", " Name/Value");
	for (ElfN_Off i = 0; i < shdr->sh_size; i += shdr->sh_entsize)
	{
		const ElfN_Dyn *dyn = (const ElfN_Dyn*)&data[i];
		print_dyn(elf, dyn);
		if (dyn->d_tag == DT_NULL)
			break;
	}
	return 0;
}

static int
print_dynamic(struct env *env, struct elfN *elf)
{
	for (ElfN_Word i = 0; i < elfN_get_shnum(elf); ++i)
	{
		const ElfN_Shdr *shdr = elfN_get_shdr(elf, i);
		if (shdr->sh_type != SHT_DYNAMIC)
			continue;
		if (print_dynamic_section(env, elf, shdr))
			return 1;
	}
	return 0;
}

static const char *
reloc_str(struct elfN *elf, uint32_t v)
{
	switch (elfN_get_ehdr(elf)->e_machine)
	{
		case EM_386:
			return elf_r_386_str(v);
		case EM_X86_64:
			return elf_r_x86_64_str(v);
		case EM_ARM:
			return elf_r_arm_str(v);
		case EM_AARCH64:
			return elf_r_aarch64_str(v);
		case EM_RISCV:
			return elf_r_riscv_str(v);
	}
	return NULL;
}

static void
print_rel_sym(struct elfN *elf, const ElfN_Rel *rel)
{
	ElfN_Word symidx = ELFN_R_SYM(rel->r_info);
	if (!symidx)
		return;
	const ElfN_Sym *sym = elfN_get_dynsym(elf, symidx);
	printf("%0*" PRIxN "   %-.15s",
	       (int)(sizeof(size_t) * 2), sym->st_value,
	       elfN_get_dynstr_str(elf, sym->st_name));
}

static int
print_rel_section(struct env *env, struct elfN *elf, const ElfN_Shdr *shdr)
{
	uint8_t *data;

	if (elfN_read_section(elf, shdr, (void**)&data, NULL))
	{
		fprintf(stderr, "%s: failed to read section: %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	printf("\n");
	printf("Relocation section '%s' at offset 0x%" PRIxN " contains %" PRIuN " entries:\n",
	       elfN_get_shname(elf, shdr),
	       shdr->sh_offset,
	       shdr->sh_size / shdr->sh_entsize);
	printf("%-*s %-*s %-16s %-11s %-15s\n",
	       (int)(sizeof(size_t) * 2), "Offset",
	       (int)(sizeof(size_t) * 2), "Info",
	       "Type", "Sym.Value", "Sym. Name");
	for (ElfN_Off i = 0; i < shdr->sh_size; i += shdr->sh_entsize)
	{
		const ElfN_Rel *rel = (const ElfN_Rel*)&data[i];
		printf("%0*" PRIxN " %0*" PRIxN " R_%s_%-11.11s ",
		       (int)(sizeof(size_t) * 2), rel->r_offset,
		       (int)(sizeof(size_t) * 2), rel->r_info,
		       elf_r_str(elfN_get_ehdr(elf)->e_machine),
		       reloc_str(elf, ELFN_R_TYPE(rel->r_info)));
		print_rel_sym(elf, rel);
		printf("\n");
	}
	free(data);
	return 0;
}

static void
print_rela_sym(struct elfN *elf, const ElfN_Rela *rela)
{
	ElfN_Word symidx = ELFN_R_SYM(rela->r_info);
	if (!symidx)
		return;
	const ElfN_Sym *sym = elfN_get_dynsym(elf, symidx);
	printf("%0*" PRIxN "   %-.15s + 0x%" PRIxN "",
	       (int)(sizeof(size_t) * 2), sym->st_value,
	       elfN_get_dynstr_str(elf, sym->st_name), rela->r_addend);
}

static int
print_rela_section(struct env *env, struct elfN *elf, const ElfN_Shdr *shdr)
{
	uint8_t *data;

	if (elfN_read_section(elf, shdr, (void**)&data, NULL))
	{
		fprintf(stderr, "%s: failed to read section: %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	printf("\n");
	printf("Relocation section '%s' at offset 0x%" PRIxN " contains %" PRIuN " entries:\n",
	       elfN_get_shname(elf, shdr),
	       shdr->sh_offset,
	       shdr->sh_size / shdr->sh_entsize);
	printf("%-*s %-*s %-20s %-10s %-15s\n",
	       (int)(sizeof(size_t) * 2), "Offset",
	       (int)(sizeof(size_t) * 2), "Info",
	       "Type", "Sym.Value", "Sym. Name");
	for (ElfN_Off i = 0; i < shdr->sh_size; i += shdr->sh_entsize)
	{
		const ElfN_Rela *rela = (const ElfN_Rela*)&data[i];
		printf("%0*" PRIxN " %0*" PRIxN " R_%s_%-11.11s ",
		       (int)(sizeof(size_t) * 2), rela->r_offset,
		       (int)(sizeof(size_t) * 2), rela->r_info,
		       elf_r_str(elfN_get_ehdr(elf)->e_machine),
		       reloc_str(elf, ELFN_R_TYPE(rela->r_info)));
		print_rela_sym(elf, rela);
		printf("\n");
	}
	free(data);
	return 0;
}

static int
print_relocations(struct env *env, struct elfN *elf)
{
	for (ElfN_Word i = 0; i < elfN_get_shnum(elf); ++i)
	{
		const ElfN_Shdr *shdr = elfN_get_shdr(elf, i);
		switch (shdr->sh_type)
		{
			case SHT_REL:
				if (print_rel_section(env, elf, shdr))
					return 1;
				break;
			case SHT_RELA:
				if (print_rela_section(env, elf, shdr))
					return 1;
				break;
		}
	}
	return 0;
}

static int
print_symbol_section(struct env *env,
                     struct elfN *elf,
                     const ElfN_Shdr *sym_shdr)
{
	const ElfN_Shdr *str_shdr = NULL;
	uint8_t *sym_data = NULL;
	uint8_t *str_data = NULL;
	int ret = 1;

	if (elfN_read_section(elf, sym_shdr, (void**)&sym_data, NULL))
	{
		fprintf(stderr, "%s: failed to read symbol section: %s\n",
		        env->progname, strerror(errno));
		goto end;
	}
	str_shdr = elfN_get_shdr(elf, sym_shdr->sh_link);
	if (!str_shdr)
	{
		fprintf(stderr, "%s: invalid symbol string link\n",
		        env->progname);
		goto end;
	}
	if (elfN_read_section(elf, str_shdr, (void**)&str_data, NULL))
	{
		fprintf(stderr, "%s: failed to read string section: %s\n",
		        env->progname, strerror(errno));
		goto end;
	}
	printf("\n");
	printf("Symbol table '%s' contains %" PRIuN " entries:\n",
	       elfN_get_shname(elf, sym_shdr),
	       sym_shdr->sh_size / sym_shdr->sh_entsize);
	printf(" %5s: %*s %5s %-7s %-6s %-8s %3s %s\n",
	       "Num",
	       (int)(sizeof(size_t) * 2),
	       "Value", "Size", "Type", "Bind", "Vis", "Ndx", "Name");
	for (ElfN_Off i = 0; i < sym_shdr->sh_size; i += sym_shdr->sh_entsize)
	{
		const ElfN_Sym *sym = (const ElfN_Sym*)&sym_data[i];
		printf(" %5" PRIuN ": %0*" PRIxN " %5" PRIuN " %-7s %-6s %-8s",
		       i / sym_shdr->sh_entsize,
		       (int)(sizeof(size_t) * 2), sym->st_value,
		       sym->st_size,
		       elf_stt_str(ELFN_ST_TYPE(sym->st_info)),
		       elf_stb_str(ELFN_ST_BIND(sym->st_info)),
		       elf_stv_str(ELFN_ST_VISIBILITY(sym->st_other)));
		switch (sym->st_shndx)
		{
			case SHN_UNDEF:
				printf(" UND");
				break;
			case SHN_ABS:
				printf(" ABS");
				break;
			default:
				printf(" %3" PRIu16, sym->st_shndx);
				break;
		}
		printf(" %s\n", sym->st_name < str_shdr->sh_size ? (char*)&str_data[sym->st_name] : "");
	}
	ret = 0;

end:
	free(str_data);
	free(sym_data);
	return ret;
}

static int
print_symbols(struct env *env, struct elfN *elf)
{
	for (ElfN_Word i = 0; i < elfN_get_shnum(elf); ++i)
	{
		const ElfN_Shdr *shdr = elfN_get_shdr(elf, i);
		if (shdr->sh_type != SHT_SYMTAB
		 && shdr->sh_type != SHT_DYNSYM)
			continue;
		if (print_symbol_section(env, elf, shdr))
			return 1;
	}
	return 0;
}

static int
print_hash(struct env *env, struct elfN *elf, const ElfN_Shdr *shdr)
{
	if (shdr->sh_size < 4 * 2)
	{
		fprintf(stderr, "%s: invalid hash section length\n",
		        env->progname);
		return 1;
	}
	uint32_t *hashptr;
	if (elfN_read_section(elf, shdr, (void**)&hashptr, NULL))
	{
		fprintf(stderr, "%s: failed to read section: %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	uint32_t buckets_count = hashptr[0];
	uint32_t chain_count = hashptr[1];
	uint32_t *buckets = &hashptr[2];
	uint32_t *chain = &hashptr[2 + buckets_count];
	if (shdr->sh_size < 4 * (2 + buckets_count + chain_count))
	{
		fprintf(stderr, "%s: invalid hash section length\n",
		        env->progname);
		free(hashptr);
		return 1;
	}
	uint32_t *counts = calloc(sizeof(*counts) * buckets_count, 1);
	if (!counts)
	{
		fprintf(stderr, "%s: malloc: %s\n", env->progname,
		        strerror(errno));
		free(hashptr);
		return 1;
	}
	printf("\n");
	printf("Histogram for bucket list length (total of %" PRIu32 " buckets):\n",
	       buckets_count);
	printf(" %-6s %-8s %-10s %s\n", "Length", "Number", "% of total", "Coverage");
	uint32_t sum = 0;
	for (uint32_t i = 0; i < buckets_count; ++i)
	{
		uint32_t count = 0;
		for (size_t j = buckets[i]; j && j < chain_count; j = chain[j])
		{
			sum++;
			count++;
		}
		counts[count]++;
	}
	uint32_t coverage = 0;
	for (uint32_t i = 0; i < buckets_count; ++i)
	{
		if (!counts[i])
			continue;
		coverage += counts[i] * i;
		printf(" %6" PRIu32 " %-8" PRIu32 " (%3" PRIu32 ".%" PRIu32 "%%)     %3" PRIu32 ".%" PRIu32 "%%\n",
		       i, counts[i], (counts[i] * 100 / buckets_count),
		       (counts[i] * 1000 / buckets_count) % 10,
		       (coverage * 100 / sum), (coverage * 1000 / sum) % 10);
	}
	free(hashptr);
	free(counts);
	return 0;
}

static int
print_gnu_hash(struct env *env, struct elfN *elf, const ElfN_Shdr *shdr)
{
	if (shdr->sh_size < 4 * 4 + sizeof(ElfN_Addr))
	{
		fprintf(stderr, "%s: invalid hash section length\n",
		        env->progname);
		return 1;
	}
	uint32_t *hashptr;
	if (elfN_read_section(elf, shdr, (void**)&hashptr, NULL))
	{
		fprintf(stderr, "%s: failed to read section: %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	uint32_t buckets_count = hashptr[0];
	uint32_t symoffset = hashptr[1];
	uint32_t bloom_size = hashptr[2];
	ElfN_Addr *bloom = (ElfN_Addr*)&hashptr[4];
	uint32_t *buckets = (uint32_t*)&bloom[bloom_size];
	uint32_t *chain = &buckets[buckets_count];
	if (shdr->sh_size < 4 * (5 + bloom_size + buckets_count))
	{
		fprintf(stderr, "%s: invalid hash section length\n",
		        env->progname);
		free(hashptr);
		return 1;
	}
	uint32_t counts[32];
	memset(counts, 0, sizeof(counts));
	printf("\n");
	printf("Histogram for `%s' bucket list length (total of %" PRIu32 " buckets):\n",
	       elfN_get_shname(elf, shdr), buckets_count);
	printf(" %-6s %-8s %-10s %s\n", "Length", "Number", "% of total", "Coverage");
	uint32_t sum = 0;
	for (uint32_t i = 0; i < buckets_count; ++i)
	{
		uint32_t count = 0;
		uint32_t v = buckets[i];
		if (v >= symoffset)
		{
			v -= symoffset;
			while (1)
			{
				sum++;
				count++;
				uint32_t h = chain[v];
				if (h & 1)
					break;
				v++;
			}
		}
		counts[count]++;
	}
	uint32_t n = 0;
	uint32_t coverage = 0;
	for (uint32_t i = 0; n < buckets_count && coverage < sum; ++i)
	{
		coverage += counts[i] * i;
		printf(" %6" PRIu32 " %-8" PRIu32 " (%3" PRIu32 ".%" PRIu32 "%%)     %3" PRIu32 ".%" PRIu32 "%%\n",
		       i, counts[i], (counts[i] * 100 / buckets_count),
		       (counts[i] * 1000 / buckets_count) % 10,
		       (coverage * 100 / sum), (coverage * 1000 / sum) % 10);
		if (counts[i])
			n++;
	}
	free(hashptr);
	return 0;
}

static int
print_histogram(struct env *env, struct elfN *elf)
{
	for (size_t i = 0; i < elfN_get_shnum(elf); ++i)
	{
		const ElfN_Shdr *shdr = elfN_get_shdr(elf, i);
		switch (shdr->sh_type)
		{
			case SHT_HASH:
				if (print_hash(env, elf, shdr))
					return 1;
				break;
			case SHT_GNU_HASH:
				if (print_gnu_hash(env, elf, shdr))
					return 1;
				break;
		}
	}
	return 0;
}

static int
print_verdef(struct env *env, struct elfN *elf, const ElfN_Shdr *shdr)
{
	const ElfN_Verdef *verdef;
	ElfN_Off i = 0;
	ElfN_Off ndef = 0;
	uint8_t *data;

	if (elfN_read_section(elf, shdr, (void**)&data, NULL))
	{
		fprintf(stderr, "%s: failed to read section: %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	while (1)
	{
		verdef = (const ElfN_Verdef*)&data[i];
		ndef++;
		if (!verdef->vd_next)
			break;
		i += verdef->vd_next;
	}
	printf("\n");
	printf("Version definition section '%s' contains %" PRIuN " entries:\n",
	       elfN_get_shname(elf, shdr), ndef);
	i = 0;
	while (1)
	{
		verdef = (const ElfN_Verdef*)&data[i];
		const ElfN_Verdaux *verdaux = (const ElfN_Verdaux*)&data[i + verdef->vd_aux];
		char flags_buf[32];
		elf_ver_flags_str(flags_buf, sizeof(flags_buf), verdef->vd_flags);
		if (!flags_buf[0])
			strlcpy(flags_buf, "none", sizeof(flags_buf));
		printf("  %#06" PRIxN ": Rev: %" PRIu16 "  Flags: %s  Index: %" PRIu16 "  Cnt: %" PRIu16 "  Name: %s\n",
		       i,
		       verdef->vd_version,
		       flags_buf,
		       verdef->vd_ndx,
		       verdef->vd_cnt,
		       elfN_get_dynstr_str(elf, verdaux->vda_name));
		if (!verdef->vd_next)
			break;
		i += verdef->vd_next;
	}
	free(data);
	return 0;
}

static int
print_verneed(struct env *env, struct elfN *elf, const ElfN_Shdr *shdr)
{
	const ElfN_Verneed *verneed;
	ElfN_Off i = 0;
	ElfN_Off nneed = 0;
	uint8_t *data;

	if (elfN_read_section(elf, shdr, (void**)&data, NULL))
	{
		fprintf(stderr, "%s: failed to read section: %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	while (1)
	{
		verneed = (const ElfN_Verneed*)&data[i];
		nneed++;
		if (!verneed->vn_next)
			break;
		i += verneed->vn_next;
	}
	printf("\n");
	printf("Version needs section '%s' contains %" PRIuN " entries:\n",
	       elfN_get_shname(elf, shdr), nneed);
	const ElfN_Shdr *link_shdr = elfN_get_shdr(elf, shdr->sh_link);
	printf(" Addr: 0x%0*" PRIxN "  Offset: 0x%06" PRIxN"  Link: %" PRIu32 " (%s)\n",
	       (int)(sizeof(size_t) * 2), shdr->sh_addr,
	       shdr->sh_offset,
	       shdr->sh_link,
	       link_shdr ? elfN_get_shname(elf, link_shdr) : "");
	i = 0;
	while (1)
	{
		verneed = (const ElfN_Verneed*)&data[i];
		printf("  %#06" PRIxN ": Version: %" PRIu16 "  File: %s  Cnt: %" PRIu16 "\n",
		       i,
		       verneed->vn_version,
		       elfN_get_dynstr_str(elf, verneed->vn_file),
		       verneed->vn_cnt);
		ElfN_Off j = i + verneed->vn_aux;
		while (1)
		{
			const ElfN_Vernaux *vernaux = (const ElfN_Vernaux*)&data[j];
			char flags_buf[32];
			elf_ver_flags_str(flags_buf, sizeof(flags_buf), vernaux->vna_flags);
			if (!flags_buf[0])
				strlcpy(flags_buf, "none", sizeof(flags_buf));
			printf("  %#06" PRIxN ":   Name: %s  Flags: %s  Version: %" PRIu16 "\n",
			       j,
			       elfN_get_dynstr_str(elf, vernaux->vna_name),
			       flags_buf,
			       vernaux->vna_other);
			if (!vernaux->vna_next)
				break;
			j += vernaux->vna_next;
		}
		if (!verneed->vn_next)
			break;
		i += verneed->vn_next;
	}
	free(data);
	return 0;
}

static const char *
get_verneed_name(struct elfN *elf, uint8_t *data, uint16_t version)
{
	ElfN_Off i = 0;
	while (1)
	{
		const ElfN_Verneed *verneed = (const ElfN_Verneed*)&data[i];
		ElfN_Off j = i + verneed->vn_aux;
		while (1)
		{
			const ElfN_Vernaux *vernaux = (const ElfN_Vernaux*)&data[j];
			if (vernaux->vna_other == version)
				return elfN_get_dynstr_str(elf, vernaux->vna_name);
			if (!vernaux->vna_next)
				break;
			j += vernaux->vna_next;
		}
		if (!verneed->vn_next)
			break;
		i += verneed->vn_next;
	}
	return NULL;
}

static const char *
get_verdef_name(struct elfN *elf, uint8_t *data, uint16_t version)
{
	ElfN_Off i = 0;
	while (1)
	{
		const ElfN_Verdef *verdef = (const ElfN_Verdef*)&data[i];
		const ElfN_Verdaux *verdaux = (const ElfN_Verdaux*)&data[i + verdef->vd_aux];
		if (verdef->vd_ndx == version)
			return elfN_get_dynstr_str(elf, verdaux->vda_name);
		if (!verdef->vd_next)
			break;
		i += verdef->vd_next;
	}
	return NULL;
}

static int
print_versym(struct env *env,
             struct elfN *elf,
             const ElfN_Shdr *shdr,
             const ElfN_Shdr *verneed,
             const ElfN_Shdr *verdef)
{
	uint8_t *verneed_data = NULL;
	uint8_t *verdef_data = NULL;
	uint8_t *data;
	int ret = 1;

	if (verneed && elfN_read_section(elf, verneed, (void**)&verneed_data, NULL))
	{
		fprintf(stderr, "%s: failed to read section: %s\n",
		        env->progname, strerror(errno));
		goto end;
	}
	if (verdef && elfN_read_section(elf, verdef, (void**)&verdef_data, NULL))
	{
		fprintf(stderr, "%s: failed to read section: %s\n",
		        env->progname, strerror(errno));
		goto end;
	}
	if (elfN_read_section(elf, shdr, (void**)&data, NULL))
	{
		fprintf(stderr, "%s: failed to read section: %s\n",
		        env->progname, strerror(errno));
		goto end;
	}
	printf("\n");
	printf("Version symbols section '%s' contains %" PRIuN " entries:\n",
	       elfN_get_shname(elf, shdr),
	       shdr->sh_size / shdr->sh_entsize);
	const ElfN_Shdr *link_shdr = elfN_get_shdr(elf, shdr->sh_link);
	printf(" Addr: 0x%0*" PRIxN "  Offset: 0x%06" PRIxN "  Link: %" PRIu32 " (%s)\n",
	       (int)(sizeof(size_t) * 2), shdr->sh_addr,
	       shdr->sh_offset,
	       shdr->sh_link,
	       link_shdr ? elfN_get_shname(elf, link_shdr) : "");
	for (ElfN_Off i = 0, n = 0; i < shdr->sh_size; i += shdr->sh_entsize, ++n)
	{
		if (!(i % 8))
			printf("  %03" PRIxN ":", i / 2);
		uint16_t ver_ndx = *(uint16_t*)&data[i];
		const char *name;
		switch (ver_ndx)
		{
			case VER_NDX_LOCAL:
				name = "*local*";
				break;
			case VER_NDX_GLOBAL:
				name = "*global*";
				break;
			default:
			{
				ElfN_Sym *sym = elfN_get_dynsym(elf, n);
				if (sym && sym->st_shndx != SHN_UNDEF)
					name = get_verdef_name(elf, verdef_data,
					                       ver_ndx);
				else
					name = get_verneed_name(elf, verneed_data,
					                        ver_ndx);
				break;
			}
		}
		char tmp[128];
		snprintf(tmp, sizeof(tmp), "%3" PRIu16 " (%s)",
		         ver_ndx, name ? name : "");
		printf("%-20s", tmp);
		if (i % 8 == 6)
			printf("\n");
	}
	if (shdr->sh_size % 8)
		printf("\n");
	ret = 0;

end:
	free(verneed_data);
	free(verdef_data);
	free(data);
	return ret;
}

static int
print_version(struct env *env, struct elfN *elf)
{
	const ElfN_Shdr *verneed = NULL;
	const ElfN_Shdr *verdef = NULL;

	for (ElfN_Word i = 0; i < elfN_get_shnum(elf); ++i)
	{
		const ElfN_Shdr *shdr = elfN_get_shdr(elf, i);
		switch (shdr->sh_type)
		{
			case SHT_VERNEED:
				verneed = shdr;
				break;
			case SHT_VERDEF:
				verdef = shdr;
				break;
		}
	}
	for (ElfN_Word i = 0; i < elfN_get_shnum(elf); ++i)
	{
		const ElfN_Shdr *shdr = elfN_get_shdr(elf, i);
		switch (shdr->sh_type)
		{
			case SHT_VERDEF:
				if (print_verdef(env, elf, shdr))
					return 1;
				break;
			case SHT_VERNEED:
				if (print_verneed(env, elf, shdr))
					return 1;
				break;
			case SHT_VERSYM:
				if (print_versym(env, elf, shdr, verneed, verdef))
					return 1;
				break;
		}
	}
	return 0;
}

static int
dwarf_hdr(struct env *env,
          struct buf *buf,
          uint64_t *unit_length,
          uint16_t *version,
          int *dwarf64)
{
	uint32_t u32;

	if (buf_u32(buf, &u32))
	{
		fprintf(stderr, "%s: DWARF unit_length EOF\n",
		        env->progname);
		return 1;
	}
	if (u32 == 0xFFFFFFFF)
	{
		printf("dwarf64\n");
		if (buf_u64(buf, unit_length))
		{
			fprintf(stderr, "%s: DWARF unit_length EOF\n",
			        env->progname);
			return 1;
		}
		*dwarf64 = 1;
	}
	else
	{
		printf("dwarf32\n");
		if (u32 >= 0xFFFFFFF0)
		{
			fprintf(stderr, "%s: invalid DWARF unit_length: 0x%" PRIx32 "\n",
			        env->progname, u32);
			return 1;
		}
		printf("length: %u\n", u32);
		*unit_length = u32;
		*dwarf64 = 0;
	}
	if (buf_u16(buf, version))
	{
		fprintf(stderr, "%s: DWARF version EOF\n",
		        env->progname);
		return 1;
	}
	return 0;
}

static int
print_debug_rnglists(struct env *env,
                     struct elfN *elf,
                     const ElfN_Shdr *shdr)
{
	uint64_t unit_length;
	uint16_t version;
	uint8_t address_size;
	uint8_t segment_selector_size;
	uint32_t offset_entry_count;
	int dwarf64;
	struct buf buf;
	uint8_t *data;
	ElfN_Word size;
	int ret = 1;

	if (elfN_read_section(elf, shdr, (void**)&data, &size))
	{
		fprintf(stderr, "%s: failed to read section: %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	printf("Contents of the %s section:\n", elfN_get_shname(elf, shdr));
	buf_init(&buf, data, size);
	if (dwarf_hdr(env, &buf, &unit_length, &version, &dwarf64))
		goto end;
	if (version != 5)
	{
		fprintf(stderr, "%s: invalid .debug_rnglists version: %" PRIu16 "\n",
		        env->progname, version);
		goto end;
	}
	if (buf_u8(&buf, &address_size))
	{
		fprintf(stderr, "%s: .debug_rnglists address_size EOF\n",
		        env->progname);
		goto end;
	}
	if (buf_u8(&buf, &segment_selector_size))
	{
		fprintf(stderr, "%s: .debug_rnglists segment_selector_size EOF\n",
		        env->progname);
		goto end;
	}
	if (buf_u32(&buf, &offset_entry_count))
	{
		fprintf(stderr, "%s: .debug_rnglists offset_entry_count EOF\n",
		        env->progname);
		goto end;
	}
	printf("length: %" PRIu64 ", count: %" PRIu32 "\n", unit_length, offset_entry_count);
	while (1)
	{
		uint8_t u8;
		if (buf_u8(&buf, &u8))
			break;
		printf("u8: 0x%02x\n", u8);
	}
	ret = 0;

end:
	free(data);
	return ret;
}

static int
print_debug(struct env *env, struct elfN *elf)
{
	for (ElfN_Word i = 0; i < elfN_get_shnum(elf); ++i)
	{
		const ElfN_Shdr *shdr = elfN_get_shdr(elf, i);
		const char *name = elfN_get_shname(elf, shdr);
		if (!name)
			continue;
		if (!strcmp(name, ".debug_rnglists"))
		{
			if (print_debug_rnglists(env, elf, shdr))
				return 1;
			continue;
		}
	}
	return 0;
}

int
print_elfN(struct env *env, struct elfN *elf)
{
	int ret = 1;

	if (env->opt & OPT_h)
	{
		if (print_ehdr(elfN_get_ehdr(elf)))
			goto end;
	}
	if (env->opt & OPT_S)
	{
		if (print_program_sections(env, elf))
			goto end;
	}
	if (env->opt & OPT_l)
	{
		if (print_program_headers(env, elf))
			goto end;
	}
	if (env->opt & OPT_d)
	{
		if (print_dynamic(env, elf))
			goto end;
	}
	if (env->opt & OPT_r)
	{
		if (print_relocations(env, elf))
			goto end;
	}
	if (env->opt & OPT_s)
	{
		if (print_symbols(env, elf))
			goto end;
	}
	if (env->opt & OPT_I)
	{
		if (print_histogram(env, elf))
			goto end;
	}
	if (env->opt & OPT_V)
	{
		if (print_version(env, elf))
			goto end;
	}
	if (env->opt & OPT_w)
	{
		if (print_debug(env, elf))
			goto end;
	}
	ret = 0;

end:
	return ret;
}
