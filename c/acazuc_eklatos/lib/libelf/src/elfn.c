#include <libelf.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <zlib.h>

struct elfN
{
	FILE *fp;
	ElfN_Ehdr ehdr;
	ElfN_Phdr *phdr;
	ElfN_Shdr *shdr;
	ElfN_Shdr *sh_dynamic;
	ElfN_Dyn *dyns;
	ElfN_Word dyns_count;
	ElfN_Shdr *sh_dynsym;
	ElfN_Sym *dynsyms;
	ElfN_Word dynsyms_count;
	ElfN_Shdr *sh_strndx;
	char *shstrtab;
	ElfN_Word shstrtab_size;
	ElfN_Shdr *sh_dynstr;
	char *dynstr;
	ElfN_Word dynstr_size;
};

struct elfN_symtab
{
	const ElfN_Shdr *symtab_shdr;
	uint8_t *symtab_data;
	const ElfN_Shdr *strtab_shdr;
	uint8_t *strtab_data;
};

static int elfN_readat(struct elfN *elf, void *ptr, ElfN_Word len,
                       ElfN_Off off)
{
	if (fseek(elf->fp, off, SEEK_SET) == -1)
		return 1;
	if (fread(ptr, 1, len, elf->fp) != len)
		return 1;
	return 0;
}

static int elfN_load_phdr(struct elfN *elf)
{
	elf->phdr = malloc(sizeof(*elf->phdr) * elf->ehdr.e_phnum);
	if (!elf->phdr)
		return 1;
	for (ElfN_Word i = 0; i < elf->ehdr.e_phnum; ++i)
	{
		if (elfN_readat(elf, &elf->phdr[i], sizeof(*elf->phdr),
		                elf->ehdr.e_phoff + elf->ehdr.e_phentsize * i))
		{
			free(elf->phdr);
			elf->phdr = NULL;
			return 1;
		}
	}
	return 0;
}

static int elfN_load_shdr(struct elfN *elf)
{
	elf->shdr = malloc(sizeof(*elf->shdr) * elf->ehdr.e_shnum);
	if (!elf->shdr)
		return 1;
	for (ElfN_Word i = 0; i < elf->ehdr.e_shnum; ++i)
	{
		if (elfN_readat(elf, &elf->shdr[i], sizeof(*elf->shdr),
		                elf->ehdr.e_shoff + elf->ehdr.e_shentsize * i))
		{
			free(elf->shdr);
			elf->shdr = NULL;
			return 1;
		}
	}
	return 0;
}

static int elfN_load_shstr(struct elfN *elf)
{
	elf->sh_strndx = (ElfN_Shdr*)elfN_get_shdr(elf, elf->ehdr.e_shstrndx);
	if (!elf->sh_strndx)
		return 1;
	return elfN_read_section(elf, elf->sh_strndx,
	                         (void**)&elf->shstrtab, &elf->shstrtab_size);
}

static int elfN_load_dynstr(struct elfN *elf)
{
	elf->sh_dynstr = (ElfN_Shdr*)elfN_get_shdr(elf, elf->sh_dynsym->sh_link);
	if (!elf->sh_dynstr)
		return 1;
	return elfN_read_section(elf, elf->sh_dynstr,
	                         (void**)&elf->dynstr,
	                         &elf->dynstr_size);
}

static int elfN_load_dynsym(struct elfN *elf)
{
	for (ElfN_Word i = 0; i < elf->ehdr.e_shnum; ++i)
	{
		const ElfN_Shdr *shdr = &elf->shdr[i];
		if (shdr->sh_type != SHT_DYNSYM)
			continue;
		ElfN_Word size;
		if (elfN_read_section(elf, shdr,
		                      (void**)&elf->dynsyms,
		                      &size))
			return 1;
		elf->sh_dynsym = (ElfN_Shdr*)shdr;
		elf->dynsyms_count = size / sizeof(ElfN_Sym);
		return 0;
	}
	return 1;
}

static int elfN_load_dyns(struct elfN *elf)
{
	for (ElfN_Word i = 0; i < elf->ehdr.e_shnum; ++i)
	{
		const ElfN_Shdr *shdr = &elf->shdr[i];
		if (shdr->sh_type != SHT_DYNAMIC)
			continue;
		ElfN_Word size;
		if (elfN_read_section(elf, shdr,
		                      (void**)&elf->dyns,
		                      &size))
			return 1;
		elf->sh_dynamic = (ElfN_Shdr*)shdr;
		elf->dyns_count = size / sizeof(ElfN_Dyn);
		return 0;
	}
	return 1;
}

static int elf_init(struct elfN *elf)
{
	if (fread(&elf->ehdr, 1, sizeof(elf->ehdr), elf->fp) != sizeof(elf->ehdr))
		return 1;
	if (elf->ehdr.e_ident[EI_MAG0] != ELFMAG0
	 || elf->ehdr.e_ident[EI_MAG1] != ELFMAG1
	 || elf->ehdr.e_ident[EI_MAG2] != ELFMAG2
	 || elf->ehdr.e_ident[EI_MAG3] != ELFMAG3
	 || elf->ehdr.e_ident[EI_CLASS] != ELFCLASS)
	{
		errno = ENOEXEC;
		return 1;
	}
	if (elfN_load_phdr(elf)
	 || elfN_load_shdr(elf)
	 || elfN_load_shstr(elf)
	 || elfN_load_dynsym(elf)
	 || elfN_load_dynstr(elf)
	 || elfN_load_dyns(elf))
		return 1;
	return 0;
}

struct elfN *elfN_open(const char *path)
{
	struct elfN *elf = calloc(1, sizeof(*elf));
	if (!elf)
		goto err;
	elf->fp = fopen(path, "rb");
	if (!elf->fp)
		goto err;
	if (elf_init(elf))
		goto err;
	return elf;

err:
	elfN_free(elf);
	return NULL;
}

struct elfN *elfN_open_fd(int fd)
{
	int nfd = dup(fd);
	if (nfd == -1)
		return NULL;
	struct elfN *elf = calloc(1, sizeof(*elf));
	if (!elf)
		goto err;
	elf->fp = fdopen(nfd, "rb");
	if (!elf->fp)
		goto err;
	nfd = -1;
	if (elf_init(elf))
		goto err;
	return elf;

err:
	if (nfd != -1)
		close(nfd);
	elfN_free(elf);
	return NULL;
}

void elfN_free(struct elfN *elf)
{
	if (!elf)
		return;
	free(elf->dyns);
	free(elf->shstrtab);
	free(elf->dynstr);
	free(elf->phdr);
	free(elf->shdr);
	if (elf->fp)
		fclose(elf->fp);
	free(elf);
}

static int decompress_zlib(void *out, size_t out_size,
                           const void *in, size_t in_size)
{
	z_stream zstream;
	memset(&zstream, 0, sizeof(zstream));
	if (inflateInit(&zstream) != Z_OK)
		return 1;
	zstream.avail_in = in_size;
	zstream.next_in = in;
	zstream.avail_out = out_size;
	zstream.next_out = out;
	int ret = inflate(&zstream, Z_FINISH);
	inflateEnd(&zstream);
	if (ret != Z_STREAM_END)
		return 1;
	if (zstream.avail_out)
		return 1;
	return 0;
}

int elfN_read_section(struct elfN *elf, const ElfN_Shdr *shdr,
                      void **datap, ElfN_Word *sizep)
{
	ElfN_Chdr chdr;
	ElfN_Word size;
	ElfN_Off offset;

	if (!shdr->sh_size)
	{
		*datap = NULL;
		*sizep = 0;
		return 0;
	}
	size = shdr->sh_size;
	offset = shdr->sh_offset;
	if (shdr->sh_flags & SHF_COMPRESSED)
	{
		if (elfN_readat(elf, &chdr, sizeof(chdr), shdr->sh_offset))
			return 1;
		size -= sizeof(chdr);
		offset += sizeof(chdr);
	}
	*datap = malloc(size);
	if (!*datap)
		return 1;
	if (elfN_readat(elf, *datap, size, offset))
	{
		free(*datap);
		*datap = NULL;
		return 1;
	}
	if (shdr->sh_flags & SHF_COMPRESSED)
	{
		switch (chdr.ch_type)
		{
			case ELFCOMPRESS_ZLIB:
			{
				void *decompressed_data = malloc(chdr.ch_size);
				if (!decompressed_data)
					return 1;
				int ret = decompress_zlib(decompressed_data,
				                          chdr.ch_size,
				                          *datap,
				                          size);
				free(*datap);
				*datap = NULL;
				if (ret)
				{
					free(decompressed_data);
					return 1;
				}
				*datap = decompressed_data;
				*sizep = chdr.ch_size;
				return 0;
			}
			/* XXX ELFCOMPRESS_ZSTD */
			default:
			{
				free(*datap);
				*datap = NULL;
				return 1;
			}
		}
	}
	if (sizep)
		*sizep = size;
	return 0;
}

ElfN_Ehdr *elfN_get_ehdr(struct elfN *elf)
{
	return &elf->ehdr;
}

ElfN_Half elfN_get_shnum(struct elfN *elf)
{
	return elf->ehdr.e_shnum;
}

ElfN_Off elfN_get_shoff(struct elfN *elf)
{
	return elf->ehdr.e_shoff;
}

ElfN_Shdr *elfN_get_shdr(struct elfN *elf, ElfN_Half idx)
{
	if (idx >= elf->ehdr.e_shnum)
		return NULL;
	return &elf->shdr[idx];
}

ElfN_Half elfN_get_phnum(struct elfN *elf)
{
	return elf->ehdr.e_phnum;
}

ElfN_Off elfN_get_phoff(struct elfN *elf)
{
	return elf->ehdr.e_phoff;
}

ElfN_Phdr *elfN_get_phdr(struct elfN *elf, ElfN_Half idx)
{
	if (idx >= elf->ehdr.e_phnum)
		return NULL;
	return &elf->phdr[idx];
}

ElfN_Word elfN_get_dynnum(struct elfN *elf)
{
	return elf->dyns_count;
}

ElfN_Dyn *elfN_get_dyn(struct elfN *elf, ElfN_Word idx)
{
	if (idx >= elf->dyns_count)
		return NULL;
	return &elf->dyns[idx];
}

ElfN_Dyn *elfN_get_dynid(struct elfN *elf, ElfN_Sword id)
{
	for (ElfN_Word i = 0; i < elf->dyns_count; ++i)
	{
		ElfN_Dyn *dyn = &elf->dyns[i];
		if (dyn->d_tag == id)
			return dyn;
	}
	return NULL;
}

ElfN_Word elfN_get_dynsymnum(struct elfN *elf)
{
	return elf->dynsyms_count;
}

ElfN_Sym *elfN_get_dynsym(struct elfN *elf, ElfN_Word idx)
{
	if (idx >= elf->dynsyms_count)
		return NULL;
	return &elf->dynsyms[idx];
}

const char *elfN_get_shname(struct elfN *elf, const ElfN_Shdr *shdr)
{
	if (shdr->sh_name >= elf->shstrtab_size)
		return NULL;
	return &elf->shstrtab[shdr->sh_name];
}

const char *elfN_get_dynstr_str(struct elfN *elf, ElfN_Word val)
{
	if (val >= elf->dynstr_size)
		return NULL;
	return &elf->dynstr[val];
}

struct elfN_symtab *elfN_symtab_read(struct elfN *elf, const ElfN_Shdr *shdr)
{
	struct elfN_symtab *symtab = calloc(1, sizeof(*symtab));
	if (!symtab)
		return NULL;
	symtab->symtab_shdr = shdr;
	if (elfN_read_section(elf, shdr, (void**)&symtab->symtab_data, NULL))
		goto err;
	symtab->strtab_shdr = elfN_get_shdr(elf, shdr->sh_link);
	if (symtab->strtab_shdr)
		elfN_read_section(elf, symtab->strtab_shdr,
		                  (void**)&symtab->strtab_data, NULL);
	return symtab;

err:
	elfN_symtab_free(symtab);
	return NULL;
}

ElfN_Sym *elfN_symtab_sym(struct elfN_symtab *symtab, ElfN_Word value)
{
	for (ElfN_Off i = 0; i < symtab->symtab_shdr->sh_size; i += symtab->symtab_shdr->sh_entsize)
	{
		ElfN_Sym *sym = (ElfN_Sym*)&symtab->symtab_data[i];
		if (sym->st_value == value)
			return sym;
	}
	return NULL;
}

const char *elfN_symtab_str(struct elfN_symtab *symtab, const ElfN_Sym *sym)
{
	if (!sym)
		return NULL;
	if (!symtab->strtab_data)
		return NULL;
	if (sym->st_name >= symtab->strtab_shdr->sh_size)
		return NULL;
	return (const char*)&symtab->strtab_data[sym->st_name];
}

void elfN_symtab_free(struct elfN_symtab *symtab)
{
	if (!symtab)
		return;
	free(symtab->symtab_data);
	free(symtab->strtab_data);
	free(symtab);
}
