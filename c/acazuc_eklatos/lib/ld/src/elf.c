#include "ld.h"

#include <sys/mman.h>
#include <sys/auxv.h>

#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>

struct tls_head tls_list = TAILQ_HEAD_INITIALIZER(tls_list);
struct elf_head elf_list = TAILQ_HEAD_INITIALIZER(elf_list);

static void elf_fini(struct elf *elf);
static int find_dep_sym(struct elf *elf, const char *name, uint8_t type,
                        struct elf **symelf, uintptr_t *addr);
static int find_native_sym(const char *name, uintptr_t *addr);
static int test_sym(struct elf *elf, const Elf_Sym *sym, const char *name,
                    uint8_t type, uintptr_t *addr);
static int handle_rel(struct elf *elf, const Elf_Rel *rel);
static int handle_rela(struct elf *elf, const Elf_Rela *rela);

static struct elf *
elf_alloc(void)
{
	struct elf *elf = calloc(1, sizeof(*elf));
	if (!elf)
	{
		LD_ERR("allocation failed");
		return NULL;
	}
	TAILQ_INSERT_TAIL(&elf_list, elf, chain);
	TAILQ_INIT(&elf->neededs);
	TAILQ_INIT(&elf->parents);
	elf->refcount++;
	return elf;
}

static struct elf *
find_elf(const char *path)
{
	struct elf *elf;
	TAILQ_FOREACH(elf, &elf_list, chain)
	{
		if (!strcmp(elf->path, path))
		{
			elf->refcount++;
			return elf;
		}
	}
	return NULL;
}

void
elf_free(struct elf *elf)
{
	if (--elf->refcount)
		return;
	if (elf->loaded)
		elf_fini(elf);
	struct elf_link *link, *next;
	TAILQ_FOREACH_SAFE(link, &elf->neededs, elf_chain, next)
	{
		TAILQ_REMOVE(&link->elf->neededs, link, elf_chain);
		TAILQ_REMOVE(&link->dep->parents, link, dep_chain);
		elf_free(link->dep);
		free(link);
	}
	if (elf->phdr && !elf->from_auxv)
	{
		size_t pad = (size_t)elf->phdr % g_page_size;
		size_t size = pad + elf->phnum * sizeof(Elf_Phdr);
		size += g_page_size - 1;
		size -= size % g_page_size;
		munmap((void*)((size_t)elf->phdr - pad), size);
	}
	if (elf->vaddr)
		munmap((void*)elf->vaddr, elf->vsize);
	if (elf->tls_module && elf->tls_module >= TAILQ_FIRST(&tls_list)->initial_mods_count)
		cleanup_dynamic_tls(elf);
	TAILQ_REMOVE(&elf_list, elf, chain);
	free(elf);
}

static int
test_sym(struct elf *elf,
         const Elf_Sym *sym,
         const char *name,
         uint8_t type,
         uintptr_t *addr)
{
	if (sym->st_shndx == SHN_UNDEF)
		return 1;
	if (ELF_ST_BIND(sym->st_info) != STB_GLOBAL)
		return 1;
	if (ELF_ST_TYPE(sym->st_info) != type)
		return 1;
	const char *sym_name = (const char*)(elf->vaddr
	                                   + elf->dt_strtab->d_un.d_ptr
	                                   + sym->st_name);
	if (strcmp(sym_name, name))
		return 1;
	*addr = elf->vaddr + sym->st_value;
	return 0;
}

static int
get_rel_sym(struct elf *elf,
            size_t symidx,
            struct elf **symelf,
            uintptr_t *addr)
{
	const Elf_Sym *sym = (const Elf_Sym*)(elf->vaddr
	                                    + elf->dt_symtab->d_un.d_ptr
	                                    + symidx * elf->dt_syment->d_un.d_val);
	if (sym->st_shndx != SHN_UNDEF)
	{
		if (symelf)
			*symelf = elf;
		*addr = elf->vaddr + sym->st_value;
		return 0;
	}
	const char *name = (const char*)(elf->vaddr
	                               + elf->dt_strtab->d_un.d_ptr
	                               + sym->st_name);
	if (!find_dep_sym(elf, name, ELF_ST_TYPE(sym->st_info), symelf, addr))
		return 0;
	if (!find_native_sym(name, addr))
		return 0;
	if (ELF_ST_BIND(sym->st_info) == STB_WEAK)
	{
		*addr = 0;
		return 0;
	}
	LD_ERR("symbol not found: %s", name);
	return 1;
}

static uint32_t
elf_hash(const char *name)
{
	uint32_t h = 0;
	uint32_t g;
	while (*name)
	{
		h = (h << 4) + *(name++);
		g = h & 0xF0000000;
		if (g)
			h ^= g >> 24;
		h &= ~g;
	}
	return h;
}

static uint32_t
gnu_hash(const char *name)
{
	uint32_t h = 5381;
	while (*name)
		h = (h << 5) + h + *(uint8_t*)(name++);
	return h;
}

static int
find_dep_sym(struct elf *elf,
             const char *name,
             uint8_t type,
             struct elf **symelf,
             uintptr_t *addr)
{
	struct elf_link *link;
	TAILQ_FOREACH(link, &elf->neededs, elf_chain)
	{
		if (!find_elf_sym(link->dep, name, type, symelf, addr))
			return 0;
	}
	return 1;
}

static int
find_elf_sym_hash(struct elf *elf,
                  const char *name,
                  uint8_t type,
                  uintptr_t *addr)
{
	uint32_t *hashptr = (uint32_t*)(elf->vaddr
	                              + elf->dt_hash->d_un.d_ptr);
	uint32_t buckets_count = hashptr[0];
	uint32_t chain_count = hashptr[1];
	uint32_t *buckets = &hashptr[2];
	uint32_t *chain = &hashptr[2 + buckets_count];
	uint32_t hash = elf_hash(name);
	for (size_t i = buckets[hash % buckets_count];
	     i && i < chain_count;
	     i = chain[i])
	{
		const Elf_Sym *sym = (const Elf_Sym*)(elf->vaddr
		                                    + elf->dt_symtab->d_un.d_ptr
		                                    + elf->dt_syment->d_un.d_val * i);
		if (!test_sym(elf, sym, name, type, addr))
			return 0;
	}
	return 1;
}

static int
find_elf_sym_gnuh(struct elf *elf,
                  const char *name,
                  uint8_t type,
                  uintptr_t *addr)
{
	uint32_t *hashptr = (uint32_t*)(elf->vaddr
	                              + elf->dt_gnu_hash->d_un.d_ptr);
	uint32_t buckets_count = hashptr[0];
	uint32_t symoffset = hashptr[1];
	uint32_t bloom_size = hashptr[2];
	uint32_t bloom_shift = hashptr[3];
	size_t *bloom = (size_t*)&hashptr[4];
	uint32_t *buckets = (uint32_t*)&bloom[bloom_size];
	uint32_t *chain = &buckets[buckets_count];
	uint32_t hash = gnu_hash(name);
	uint64_t bloom_test = bloom[(hash / (sizeof(size_t) * 8)) % bloom_size];
	if (!(bloom_test & (1UL << (hash % (sizeof(size_t) * 8))))
	 && !(bloom_test & (1UL << ((hash >> bloom_shift) % (sizeof(size_t) * 8)))))
		return 1;
	uint32_t i = buckets[hash % buckets_count];
	if (i < symoffset)
		return 1;
	while (1)
	{
		const Elf_Sym *sym = (const Elf_Sym*)(elf->vaddr
		                                    + elf->dt_symtab->d_un.d_ptr
		                                    + elf->dt_syment->d_un.d_val * i);
		uint32_t h = chain[i - symoffset];
		if ((hash & ~1) == (h & ~1))
		{
			if (!test_sym(elf, sym, name, type, addr))
				return 0;
		}
		if (h & 1)
			break;
		i++;
	}
	return 1;
}

int
find_elf_sym(struct elf *elf,
             const char *name,
             uint8_t type,
             struct elf **symelf,
             uintptr_t *addr)
{
	if (elf->dt_gnu_hash)
	{
		if (!find_elf_sym_gnuh(elf, name, type, addr))
		{
			if (symelf)
				*symelf = elf;
			return 0;
		}
	}
	else if (elf->dt_hash)
	{
		if (!find_elf_sym_hash(elf, name, type, addr))
		{
			if (symelf)
				*symelf = elf;
			return 0;
		}
	}
	return find_dep_sym(elf, name, type, symelf, addr);
}

static int
find_native_sym(const char *name, uintptr_t *addr)
{
#define TEST_SYMBOL(sym) \
do \
{ \
	if (!strcmp(name, #sym)) \
	{ \
		*addr = (uintptr_t)sym; \
		return 0; \
	} \
} while (0)

	TEST_SYMBOL(_dl_open);
	TEST_SYMBOL(_dl_close);
	TEST_SYMBOL(_dl_error);
	TEST_SYMBOL(_dl_sym);
	TEST_SYMBOL(_dl_tls_alloc);
	TEST_SYMBOL(_dl_tls_free);
	TEST_SYMBOL(_dl_tls_set);
	TEST_SYMBOL(_dl_iterate_phdr);
	return 1;

#undef TEST_SYMBOL
}

static struct elf *
load_needed(struct elf *elf, const Elf_Dyn *dyn)
{
	const char *name = (const char*)(elf->vaddr + elf->dt_strtab->d_un.d_ptr
	                               + dyn->d_un.d_val);
	if (!strcmp(name, "ld.so.1"))
		return (void*)1;
	if (strchr(name, '/'))
	{
		LD_ERR("invalid DT_NEEDED file");
		return NULL;
	}
	return elf_from_path(name);
}

static void
get_phdr_vmap(const Elf_Phdr *phdr, size_t *addr, size_t *size)
{
	*addr = phdr->p_vaddr;
	*addr -= *addr % g_page_size;
	*size = phdr->p_memsz;
	*size += phdr->p_vaddr - *addr;
	*size += g_page_size - 1;
	*size -= *size % g_page_size;
}

static int
handle_pt_load(struct elf *elf, const Elf_Phdr *phdr, int fd)
{
	if (phdr->p_filesz > phdr->p_memsz)
	{
		LD_ERR("PT_LOAD p_filesz > p_memsz");
		return 1;
	}
	size_t addr;
	size_t size;
	get_phdr_vmap(phdr, &addr, &size);
	size_t offset = phdr->p_offset;
	offset -= offset % g_page_size;
	size_t prefix = phdr->p_vaddr - addr;
	size_t fsize = prefix + phdr->p_filesz;
	size_t fmemsz = fsize + g_page_size - 1;
	fmemsz -= fmemsz % g_page_size;
	if (fmemsz > size)
	{
		LD_ERR("PT_LOAD file size > memory size");
		return 1;
	}
	void *dst = (void*)(elf->vaddr + addr);
	int prot = 0;
	if (phdr->p_flags & PF_X)
		prot |= PROT_EXEC;
	if (phdr->p_flags & PF_R)
		prot |= PROT_READ;
	if (phdr->p_flags & PF_W)
		prot |= PROT_WRITE;
	if (mmap(dst, fmemsz, prot, MAP_PRIVATE | MAP_FIXED, fd,
	         offset) == MAP_FAILED)
	{
		LD_ERR("mmap: %s", strerror(errno));
		return 1;
	}
	if (prot & PROT_WRITE)
	{
		if (prefix)
			memset(dst, 0, prefix);
		if (fmemsz != fsize)
			memset((uint8_t*)dst + fsize, 0, fmemsz - fsize);
	}
	if (size != fmemsz)
	{
		if (mmap((uint8_t*)dst + fmemsz, size - fmemsz, prot,
		         MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
		         -1, 0) == MAP_FAILED)
		{
			LD_ERR("mmap: %s", strerror(errno));
			return 1;
		}
	}
	return 0;
}

static int
elf_read(struct elf *elf, int fd)
{
	Elf_Ehdr ehdr;
	if (read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr))
	{
		LD_ERR("read: %s", strerror(errno));
		return 1;
	}
	if (ehdr.e_ident[EI_MAG0] != ELFMAG0
	 || ehdr.e_ident[EI_MAG1] != ELFMAG1
	 || ehdr.e_ident[EI_MAG2] != ELFMAG2
	 || ehdr.e_ident[EI_MAG3] != ELFMAG3)
	{
		LD_ERR("invalid header magic");
		return 1;
	}
	if (ehdr.e_ident[EI_CLASS] != ELFCLASS)
	{
		LD_ERR("invalid header class");
		return 1;
	}
	if (ehdr.e_ident[EI_DATA] != ELFDATA2LSB)
	{
		LD_ERR("invalid header data");
		return 1;
	}
	if (ehdr.e_ident[EI_VERSION] != EV_CURRENT)
	{
		LD_ERR("invalid header version ident");
		return 1;
	}
	if (ehdr.e_type != ET_DYN)
	{
		LD_ERR("not a dynamic binary");
		return 1;
	}
	if (ehdr.e_machine != ELFEM)
	{
		LD_ERR("invalid header machine");
		return 1;
	}
	if (ehdr.e_version != EV_CURRENT)
	{
		LD_ERR("invalid header version");
		return 1;
	}
	if (ehdr.e_shentsize != sizeof(Elf_Shdr))
	{
		LD_ERR("invalid section entry size");
		return 1;
	}
	if (ehdr.e_phentsize != sizeof(Elf_Phdr))
	{
		LD_ERR("invalid program entry size");
		return 1;
	}
	elf->phnum = ehdr.e_phnum;
	size_t pad = ehdr.e_phoff % g_page_size;
	elf->phdr = mmap(NULL, pad + ehdr.e_phnum * ehdr.e_phentsize,
	                 PROT_READ, MAP_PRIVATE, fd,
	                 ehdr.e_phoff - pad);
	if (elf->phdr == MAP_FAILED)
	{
		LD_ERR("mmap: %s", strerror(errno));
		return 1;
	}
	elf->phdr = (Elf_Phdr*)((uint8_t*)elf->phdr + pad);
	return 0;
}

static int
elf_parse(struct elf *elf)
{
	/* XXX max the number of PT_LOAD */
	for (size_t i = 0; i < elf->phnum; ++i)
	{
		const Elf_Phdr *phdr = &elf->phdr[i];
		if (!phdr->p_align)
		{
			LD_ERR("invalid p_align");
			return 1;
		}
		switch (phdr->p_type)
		{
			case PT_PHDR:
				if (elf->pt_phdr)
				{
					LD_ERR("multiple PT_PHDR");
					return 1;
				}
				elf->pt_phdr = phdr;
				break;
			case PT_TLS:
				if (elf->pt_tls)
				{
					LD_ERR("multiple PT_TLS");
					return 1;
				}
				elf->pt_tls = phdr;
				break;
			case PT_DYNAMIC:
				if (elf->pt_dynamic)
				{
					LD_ERR("multiple PT_DYNAMIC");
					return 1;
				}
				elf->pt_dynamic = phdr;
				break;
			case PT_GNU_STACK:
				if (elf->pt_gnu_stack)
				{
					LD_ERR("multiple PT_GNU_STACK");
					return 1;
				}
				elf->pt_gnu_stack = phdr;
				break;
			case PT_GNU_RELRO:
				if (elf->pt_gnu_relro)
				{
					LD_ERR("multiple PT_GNU_RELRO");
					return 1;
				}
				elf->pt_gnu_relro = phdr;
				break;
			default:
				break;
		}
	}
	if (elf->pt_phdr)
	{
		if (elf != TAILQ_FIRST(&elf_list))
		{
			LD_ERR("unexpected PT_PHDR");
			return 1;
		}
	}
	else if (elf == TAILQ_FIRST(&elf_list))
	{
		LD_ERR("no PT_PHDR");
		return 1;
	}
	if (!elf->pt_dynamic)
	{
		LD_ERR("no PT_DYNAMIC");
		return 1;
	}
	if (!elf->pt_gnu_stack)
	{
		LD_ERR("no PT_GNU_STACK");
		return 1;
	}
	if (!elf->pt_gnu_relro)
	{
		LD_ERR("no PT_GNU_RELRO");
		return 1;
	}
	if (elf->pt_gnu_stack->p_flags != (PF_R | PF_W))
	{
		LD_ERR("invalid stack protection");
		return 1;
	}
	elf->vaddr_min = SIZE_MAX;
	elf->vaddr_max = 0;
	for (size_t i = 0; i < elf->phnum; ++i)
	{
		const Elf_Phdr *phdr = &elf->phdr[i];
		if (phdr->p_type != PT_LOAD)
			continue;
		size_t addr;
		size_t size;
		get_phdr_vmap(phdr, &addr, &size);
		size_t addr_align = addr % phdr->p_align;
		addr -= addr_align;
		size += addr_align;
		size += phdr->p_align - 1;
		size -= size % phdr->p_align;
		if (addr < elf->vaddr_min)
			elf->vaddr_min = addr;
		if (addr + size > elf->vaddr_max)
			elf->vaddr_max = addr + size;
	}
	if (elf->vaddr_min >= elf->vaddr_max)
	{
		LD_ERR("invalid PT_LOAD mapping");
		return 1;
	}
	elf->vsize = elf->vaddr_max - elf->vaddr_min;
	return 0;
}

static int
elf_map(struct elf *elf, int fd)
{
	elf->vaddr = (size_t)mmap(NULL, elf->vsize,
	                          PROT_READ, MAP_PRIVATE,
	                          fd, 0);
	if (elf->vaddr == (size_t)-1)
	{
		LD_ERR("mmap: %s", strerror(errno));
		return 1;
	}
	for (size_t i = 0; i < elf->phnum; ++i)
	{
		const Elf_Phdr *phdr = &elf->phdr[i];
		if (phdr->p_type != PT_LOAD)
			continue;
		if (handle_pt_load(elf, phdr, fd))
			return 1;
	}
	return 0;
}

static int
handle_relocation(struct elf *elf,
                  void *addr,
                  size_t type,
                  size_t symidx,
                  size_t offset,
                  size_t addend)
{
	(void)offset;
	switch (type)
	{
#ifdef R_NONE
		case R_NONE:
			break;
#endif
#ifdef R_RELATIVE32
		case R_RELATIVE32:
			*(uint32_t*)addr = elf->vaddr + addend;
			break;
#endif
#ifdef R_RELATIVE64
		case R_RELATIVE64:
			*(uint64_t*)addr = elf->vaddr + addend;
			break;
#endif
#ifdef R_JMP_SLOT32
		case R_JMP_SLOT32:
		{
			uintptr_t sym;
			if (get_rel_sym(elf, symidx, NULL, &sym))
				return 1;
			*(uint32_t*)addr = sym;
			break;
		}
#endif
#ifdef R_JMP_SLOT64
		case R_JMP_SLOT64:
		{
			uintptr_t sym;
			if (get_rel_sym(elf, symidx, NULL, &sym))
				return 1;
			*(uint64_t*)addr = sym;
			break;
		}
#endif
#ifdef R_GLOB_DAT32
		case R_GLOB_DAT32:
		{
			uintptr_t sym;
			if (get_rel_sym(elf, symidx, NULL, &sym))
				return 1;
			*(uint32_t*)addr = sym;
			break;
		}
#endif
#ifdef R_GLOB_DAT64
		case R_GLOB_DAT64:
		{
			uintptr_t sym;
			if (get_rel_sym(elf, symidx, NULL, &sym))
				return 1;
			*(uint64_t*)addr = sym;
			break;
		}
#endif
#ifdef R_ABS32
		case R_ABS32:
		{
			uintptr_t sym;
			if (get_rel_sym(elf, symidx, NULL, &sym))
				return 1;
			*(uint32_t*)addr = sym + addend;
			break;
		}
#endif
#ifdef R_ABS64
		case R_ABS64:
		{
			uintptr_t sym;
			if (get_rel_sym(elf, symidx, NULL, &sym))
				return 1;
			*(uint64_t*)addr = sym + addend;
			break;
		}
#endif
#ifdef R_PC32
		case R_PC32:
		{
			uintptr_t sym;
			if (get_rel_sym(elf, symidx, NULL, &sym))
				return 1;
			*(uint32_t*)addr = sym - offset - elf->vaddr + addend;
			break;
		}
#endif
#ifdef R_TLS_DTPMOD32
		case R_TLS_DTPMOD32:
		{
			struct elf *symelf;
			if (symidx)
			{
				uintptr_t sym;
				if (get_rel_sym(elf, symidx, &symelf, &sym))
					return 1;
			}
			else
			{
				symelf = elf;
			}
			if (!symelf->pt_tls)
			{
				LD_ERR("R_TLS_DTPMOD32 without PT_TLS");
				return 1;
			}
			*(uint32_t*)addr = symelf->tls_module;
			break;
		}
#endif
#ifdef R_TLS_DTPMOD64
		case R_TLS_DTPMOD64:
		{
			struct elf *symelf;
			if (symidx)
			{
				uintptr_t sym;
				if (get_rel_sym(elf, symidx, &symelf, &sym))
					return 1;
			}
			else
			{
				symelf = elf;
			}
			if (!symelf->pt_tls)
			{
				LD_ERR("R_TLS_DTPMOD64 without PT_TLS");
				return 1;
			}
			*(uint64_t*)addr = symelf->tls_module;
			break;
		}
#endif
#ifdef R_TLS_DTPOFF32
		case R_TLS_DTPOFF32:
		{
			struct elf *symelf;
			uintptr_t sym;
			if (get_rel_sym(elf, symidx, &symelf, &sym))
				return 1;
			if (!symelf->pt_tls)
			{
				LD_ERR("R_TLS_DTPOFF32 without PT_TLS");
				return 1;
			}
			*(uint32_t*)addr = sym - symelf->vaddr;
			break;
		}
#endif
#ifdef R_TLS_DTPOFF64
		case R_TLS_DTPOFF64:
		{
			struct elf *symelf;
			uintptr_t sym;
			if (get_rel_sym(elf, symidx, &symelf, &sym))
				return 1;
			if (!symelf->pt_tls)
			{
				LD_ERR("R_TLS_DTPOFF64 without PT_TLS");
				return 1;
			}
			*(uint64_t*)addr = sym - symelf->vaddr;
			break;
		}
#endif
#ifdef R_TLS_TPOFF32
		case R_TLS_TPOFF32:
		{
			struct elf *symelf;
			uintptr_t sym;
			if (get_rel_sym(elf, symidx, &symelf, &sym))
				return 1;
			if (!symelf->pt_tls)
			{
				LD_ERR("R_TLS_TPOFF32 without PT_TLS");
				return 1;
			}
			*(uint32_t*)addr = sym - symelf->vaddr - symelf->tls_offset;
#ifdef __i386__
			*(uint32_t*)addr = -*(uint32_t*)addr;
#endif
			break;
		}
#endif
#ifdef R_TLS_TPOFF64
		case R_TLS_TPOFF64:
		{
			struct elf *symelf;
			uintptr_t sym;
			if (get_rel_sym(elf, symidx, &symelf, &sym))
				return 1;
			if (!symelf->pt_tls)
			{
				LD_ERR("R_TLS_TPOFF64 without PT_TLS");
				return 1;
			}
			*(uint64_t*)addr = sym - symelf->vaddr - symelf->tls_offset;
			break;
		}
#endif
		default:
			LD_ERR("unhandled reloc type: 0x%zx", type);
			return 1;
	}
	return 0;
}

static int
handle_rel(struct elf *elf, const Elf_Rel *rel)
{
	void *addr = (void*)(elf->vaddr + rel->r_offset);
	return handle_relocation(elf, addr, ELF_R_TYPE(rel->r_info),
	                         ELF_R_SYM(rel->r_info), rel->r_offset,
	                         *(size_t*)addr);
}

static int
handle_rela(struct elf *elf, const Elf_Rela *rela)
{
	void *addr = (void*)(elf->vaddr + rela->r_offset);
	return handle_relocation(elf, addr, ELF_R_TYPE(rela->r_info),
	                         ELF_R_SYM(rela->r_info), rela->r_offset,
	                         rela->r_addend);
}

static int
handle_dt_rela(struct elf *elf, const Elf_Dyn *rela,
                          const Elf_Dyn *relasz, size_t size)
{
	for (size_t i = 0; i < relasz->d_un.d_val; i += size)
	{
		const Elf_Rela *r = (const Elf_Rela*)(elf->vaddr
		                                    + rela->d_un.d_ptr
		                                    + i);
		if (handle_rela(elf, r))
			return 1;
	}
	return 0;
}

static int
handle_dt_rel(struct elf *elf,
              const Elf_Dyn *rel,
              const Elf_Dyn *relsz,
              size_t size)
{
	for (size_t i = 0; i < relsz->d_un.d_val; i += size)
	{
		const Elf_Rel *r = (const Elf_Rel*)(elf->vaddr
		                                  + rel->d_un.d_ptr
		                                  + i);
		if (handle_rel(elf, r))
			return 1;
	}
	return 0;
}

static int
elf_resolve(struct elf *elf)
{
	if (elf->dt_rel)
	{
		if (handle_dt_rel(elf, elf->dt_rel, elf->dt_relsz,
		                   elf->dt_relent->d_un.d_val))
			return 1;
	}
	if (elf->dt_rela)
	{
		if (handle_dt_rela(elf, elf->dt_rela, elf->dt_relasz,
		                   elf->dt_relaent->d_un.d_val))
			return 1;
	}
	if (elf->dt_jmprel)
	{
		switch (elf->dt_pltrel->d_un.d_val)
		{
			case DT_REL:
				if (handle_dt_rel(elf, elf->dt_jmprel, elf->dt_pltrelsz,
				                  sizeof(Elf_Rel)))
					return 1;
				break;
			case DT_RELA:
				if (handle_dt_rela(elf, elf->dt_jmprel, elf->dt_pltrelsz,
				                   sizeof(Elf_Rela)))
					return 1;
				break;
			default:
				LD_ERR("unhandled DT_PLTREL type: %zx",
				        (size_t)elf->dt_pltrel->d_un.d_val);
				return 1;
		}
	}
	return 0;
}

static int
elf_protect(struct elf *elf)
{
	size_t addr;
	size_t size;
	get_phdr_vmap(elf->pt_gnu_relro, &addr, &size);
	void *dst = (void*)(elf->vaddr + addr);
	if (mprotect(dst, size, PROT_READ) == -1)
	{
		LD_ERR("mprotect: %s", strerror(errno));
		return 1;
	}
	return 0;
}

static void
elf_init(struct elf *elf)
{
	if (elf->dt_init)
	{
		init_fn_t init_fn = (init_fn_t)(elf->vaddr
		                              + elf->dt_init->d_un.d_ptr);
		init_fn();
	}
	if (elf->dt_init_array && elf->dt_init_arraysz)
	{
		for (size_t i = 0; i < elf->dt_init_arraysz->d_un.d_val; i += sizeof(void*))
		{
			init_fn_t init_fn = *(init_fn_t*)(elf->vaddr
			                                + elf->dt_init_array->d_un.d_ptr
			                                + i);
			init_fn();
		}
	}
}

static void
elf_fini(struct elf *elf)
{
	if (elf->dt_fini_array && elf->dt_fini_arraysz)
	{
		for (size_t i = elf->dt_fini_arraysz->d_un.d_val; i >= sizeof(void*); i -= sizeof(void*))
		{
			fini_fn_t fini_fn = *(fini_fn_t*)(elf->vaddr
			                                + elf->dt_fini_array->d_un.d_ptr
			                                + i - sizeof(void*));
			fini_fn();
		}
	}
	if (elf->dt_fini)
	{
		fini_fn_t fini_fn = (fini_fn_t)(elf->vaddr
		                              + elf->dt_fini->d_un.d_ptr);
		fini_fn();
	}
}

static int
elf_dynamic(struct elf *elf)
{
	for (size_t i = 0; i < elf->pt_dynamic->p_filesz; i += sizeof(Elf_Dyn))
	{
		const Elf_Dyn *dyn = (const Elf_Dyn*)(elf->vaddr
		                                    + elf->pt_dynamic->p_vaddr
		                                    + i);
		switch (dyn->d_tag)
		{
			case DT_NULL:
				goto enditer;
			case DT_STRTAB:
				if (elf->dt_strtab)
				{
					LD_ERR("multiple DT_STRTAB");
					return 1;
				}
				elf->dt_strtab = dyn;
				break;
			case DT_STRSZ:
				if (elf->dt_strsz)
				{
					LD_ERR("multiple DT_STRSZ");
					return 1;
				}
				elf->dt_strsz = dyn;
				break;
			case DT_SYMTAB:
				if (elf->dt_symtab)
				{
					LD_ERR("multiple DT_SYMTAB");
					return 1;
				}
				elf->dt_symtab = dyn;
				break;
			case DT_SYMENT:
				if (elf->dt_syment)
				{
					LD_ERR("multiple DT_SYMENT");
					return 1;
				}
				elf->dt_syment = dyn;
				break;
			case DT_INIT_ARRAY:
				if (elf->dt_init_array)
				{
					LD_ERR("multiple DT_INIT_ARRAY");
					return 1;
				}
				elf->dt_init_array = dyn;
				break;
			case DT_INIT_ARRAYSZ:
				if (elf->dt_init_arraysz)
				{
					LD_ERR("multiple DT_INIT_ARRAYSZ");
					return 1;
				}
				elf->dt_init_arraysz = dyn;
				break;
			case DT_FINI_ARRAY:
				if (elf->dt_fini_array)
				{
					LD_ERR("multiple DT_FINI_ARRAY");
					return 1;
				}
				elf->dt_fini_array = dyn;
				break;
			case DT_FINI_ARRAYSZ:
				if (elf->dt_fini_arraysz)
				{
					LD_ERR("multiple DT_FINI_ARRAYSZ");
					return 1;
				}
				elf->dt_fini_arraysz = dyn;
				break;
			case DT_FLAGS_1:
				if (elf->dt_flags_1)
				{
					LD_ERR("multiple DT_FLAGS_1");
					return 1;
				}
				elf->dt_flags_1 = dyn;
				break;
			case DT_HASH:
				if (elf->dt_hash)
				{
					LD_ERR("multiple DT_HASH");
					return 1;
				}
				elf->dt_hash = dyn;
				break;
			case DT_GNU_HASH:
				if (elf->dt_gnu_hash)
				{
					LD_ERR("multiple DT_GNU_HASH");
					return 1;
				}
				elf->dt_gnu_hash = dyn;
				break;
			case DT_REL:
				if (elf->dt_rel)
				{
					LD_ERR("multiple DT_REL");
					return 1;
				}
				elf->dt_rel = dyn;
				break;
			case DT_RELSZ:
				if (elf->dt_relsz)
				{
					LD_ERR("multiple DT_RELSZ");
					return 1;
				}
				elf->dt_relsz = dyn;
				break;
			case DT_RELENT:
				if (elf->dt_relent)
				{
					LD_ERR("multiple DT_RELENT");
					return 1;
				}
				elf->dt_relent = dyn;
				break;
			case DT_RELA:
				if (elf->dt_rela)
				{
					LD_ERR("multiple DT_RELA");
					return 1;
				}
				elf->dt_rela = dyn;
				break;
			case DT_RELASZ:
				if (elf->dt_relasz)
				{
					LD_ERR("multiple DT_RELASZ");
					return 1;
				}
				elf->dt_relasz = dyn;
				break;
			case DT_RELAENT:
				if (elf->dt_relaent)
				{
					LD_ERR("multiple DT_RELAENT");
					return 1;
				}
				elf->dt_relaent = dyn;
				break;
			case DT_JMPREL:
				if (elf->dt_jmprel)
				{
					LD_ERR("multiple DT_JMPREL");
					return 1;
				}
				elf->dt_jmprel = dyn;
				break;
			case DT_PLTREL:
				if (elf->dt_pltrel)
				{
					LD_ERR("multiple DT_PLTREL");
					return 1;
				}
				elf->dt_pltrel = dyn;
				break;
			case DT_PLTRELSZ:
				if (elf->dt_pltrelsz)
				{
					LD_ERR("multiple DT_PLTRELSZ");
					return 1;
				}
				elf->dt_pltrelsz = dyn;
				break;
			case DT_INIT:
				if (elf->dt_init)
				{
					LD_ERR("multiple DT_INIT");
					return 1;
				}
				elf->dt_init = dyn;
				break;
			case DT_FINI:
				if (elf->dt_fini)
				{
					LD_ERR("multiple DT_FINI");
					return 1;
				}
				elf->dt_fini = dyn;
				break;
			case DT_FLAGS:
				if (elf->dt_flags)
				{
					LD_ERR("multiple DT_FLAGS");
					return 1;
				}
				elf->dt_flags = dyn;
				break;
			case DT_BIND_NOW:
				if (elf->dt_bind_now)
				{
					LD_ERR("multiple DT_BIND_NOW");
					return 1;
				}
				elf->dt_bind_now = dyn;
				break;
			case DT_NEEDED:
			case DT_SONAME:
				break;
			case DT_DEBUG:
			case DT_TEXTREL:
			case DT_RELACOUNT:
			case DT_PLTGOT:
			case DT_RPATH:
			case DT_RELCOUNT:
			case DT_VERSYM:
			case DT_VERDEF:
			case DT_VERDEFNUM:
			case DT_VERNEED:
			case DT_VERNEEDNUM:
				/* XXX */
				break;
			default:
				LD_ERR("unhandled dyn tag: %zx",
				        (size_t)dyn->d_tag);
				return 1;
		}
	}
enditer:
	if (!elf->dt_strtab)
	{
		LD_ERR("no DT_STRTAB");
		return 1;
	}
	if (!elf->dt_strsz)
	{
		LD_ERR("no DT_STRSZ");
		return 1;
	}
	if (!elf->dt_symtab)
	{
		LD_ERR("no DT_SYMTAB");
		return 1;
	}
	if (!elf->dt_syment)
	{
		LD_ERR("no DT_SYMENT");
		return 1;
	}
	if (!elf->dt_hash)
	{
		LD_ERR("no DT_HASH");
		return 1;
	}
	if (elf->dt_jmprel)
	{
		if (!elf->dt_pltrel)
		{
			LD_ERR("no DT_PLTREL on DT_JMPREL");
			return 1;
		}
		if (!elf->dt_pltrelsz)
		{
			LD_ERR("no DT_PLTRELSZ on DT_JMPREL");
			return 1;
		}
	}
	if (elf->dt_rel)
	{
		if (!elf->dt_relsz)
		{
			LD_ERR("no DT_RELSZ on DT_REL");
			return 1;
		}
		if (!elf->dt_relent)
		{
			LD_ERR("no DT_RELENT on DT_REL");
			return 1;
		}
	}
	if (elf->dt_rela)
	{
		if (!elf->dt_relasz)
		{
			LD_ERR("no DT_RELASZ on DT_RELA");
			return 1;
		}
		if (!elf->dt_relaent)
		{
			LD_ERR("no DT_RELAENT on DT_RELA");
			return 1;
		}
	}
	if (!elf->dt_flags_1)
	{
		LD_ERR("no DT_FLAGS_1");
		return 1;
	}
	if (!(elf->dt_flags_1->d_un.d_val & DF_1_NOW))
	{
		LD_ERR("no DF_1_NOW");
		return 1;
	}
	if (elf->dt_flags_1->d_un.d_val & DF_1_PIE)
	{
		if (elf != TAILQ_FIRST(&elf_list))
		{
			LD_ERR("unexpected DF_1_PIE");
			return 1;
		}
	}
	else if (elf == TAILQ_FIRST(&elf_list))
	{
		LD_ERR("no DF_1_PIE");
		return 1;
	}
	if (elf->dt_flags)
	{
		LD_ERR("unexpected DT_FLAGS");
		return 1;
	}
	if (!elf->dt_bind_now)
	{
		LD_ERR("no DT_BIND_NOW");
		return 1;
	}
	for (size_t i = 0; i < elf->pt_dynamic->p_filesz; i += sizeof(Elf_Dyn))
	{
		const Elf_Dyn *dyn = (const Elf_Dyn*)(elf->vaddr
		                                    + elf->pt_dynamic->p_vaddr
		                                    + i);
		if (dyn->d_tag != DT_NEEDED)
			continue;
		struct elf_link *link = malloc(sizeof(*link));
		if (!link)
		{
			LD_ERR("malloc failed");
			return 1;
		}
		struct elf *dep = load_needed(elf, dyn);
		if (!dep)
		{
			free(link);
			return 1;
		}
		if (dep == (void*)1)
		{
			free(link);
			continue;
		}
		link->elf = elf;
		link->dep = dep;
		TAILQ_INSERT_TAIL(&elf->neededs, link, elf_chain);
		TAILQ_INSERT_TAIL(&dep->parents, link, dep_chain);
	}
	return 0;
}

int
elf_finalize(struct elf *elf)
{
	if (elf->loaded)
		return 0;
	struct elf_link *link;
	TAILQ_FOREACH(link, &elf->neededs, elf_chain)
	{
		if (elf_finalize(link->dep))
			return 1;
	}
	if (elf_resolve(elf)
	 || elf_protect(elf))
		return 1;
	elf_init(elf);
	elf->loaded = 1;
	return 0;
}

struct elf *
elf_from_fd(const char *path, int fd)
{
	struct elf *elf = elf_alloc();
	if (!elf)
		return NULL;
	if (strlcpy(elf->path, path, sizeof(elf->path)) >= sizeof(elf->path))
		goto err;
	if (elf_read(elf, fd)
	 || elf_parse(elf)
	 || elf_map(elf, fd)
	 || elf_dynamic(elf))
		goto err;
	if (!g_initial_elf)
	{
		if (create_dynamic_tls(elf)
		 || elf_finalize(elf))
			goto err;
	}
	return elf;

err:
	elf_free(elf);
	return NULL;
}

struct elf *
elf_from_path(const char *name)
{
	if (name[0] == '/')
	{
		struct elf *elf = find_elf(name);
		if (elf)
			return elf;
		int fd = open(name, O_RDONLY);
		if (fd != -1)
		{
			elf = elf_from_fd(name, fd);
			close(fd);
			return elf;
		}
	}
	char *library_path = getenv("LD_LIBRARY_PATH");
	if (!library_path)
		library_path = "/lib";
	char *it = library_path;
	char *sp;
	while ((sp = strchrnul(it, ':')))
	{
		if (sp != it + 1)
		{
			char path[MAXPATHLEN];
			if (snprintf(path, sizeof(path), "%.*s/%s", (int)(sp - it), it, name) >= (int)sizeof(path))
			{
				LD_ERR("filepath too long");
				return NULL;
			}
			struct elf *elf = find_elf(path);
			if (elf)
				return elf;
			int fd = open(path, O_RDONLY);
			if (fd != -1)
			{
				elf = elf_from_fd(path, fd);
				close(fd);
				return elf;
			}
		}
		if (!*sp)
			break;
		it = sp + 1;
	}
	return NULL;
}

struct elf *
elf_from_auxv(const char *path)
{
	struct elf *elf = elf_alloc();
	if (!elf)
		return NULL;
	elf->from_auxv = 1;
	elf->refcount++;
	if (strlcpy(elf->path, path, sizeof(elf->path)) >= sizeof(elf->path))
		goto err;
	elf->phdr = (void*)getauxval(AT_PHDR);
	elf->phnum = getauxval(AT_PHNUM);
	size_t phentsize = getauxval(AT_PHENT);
	if (phentsize != sizeof(Elf_Phdr))
	{
		LD_ERR("invalid program entry size");
		goto err;
	}
	if (elf_parse(elf))
		goto err;
	elf->vaddr = (size_t)elf->phdr - elf->pt_phdr->p_vaddr;
	if (elf_dynamic(elf)
	 || create_initial_tls(elf)
	 || elf_finalize(elf))
		goto err;
	return elf;

err:
	elf_free(elf);
	return NULL;
}
