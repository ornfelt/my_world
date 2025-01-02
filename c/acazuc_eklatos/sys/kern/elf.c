#define ENABLE_TRACE

#include <arch/elf.h>

#include <errno.h>
#include <file.h>
#include <ksym.h>
#include <proc.h>
#include <vfs.h>
#include <elf.h>
#include <std.h>
#include <mem.h>

#if 0
# define DEBUG_RELOCATION(name, reladdr, type, dst) \
do \
{ \
	printf(name " [0x%0*zx] = 0x%0*zx\n", \
	       (int)(sizeof(size_t) * 2), reladdr, \
	       (int)(sizeof(type) * 2), (size_t)*(type*)dst); \
} while (0)
#else
#define DEBUG_RELOCATION(name, reladdr, type, dst) \
do \
{ \
} while (0)
#endif

struct elf_ctx
{
	struct vm_space *vm_space;
	struct file *file;
	Elf_Ehdr ehdr;
	Elf_Phdr *phdr;
	int flags;
	uintptr_t base_addr;
	uintptr_t map_base;
	size_t map_size;
	size_t min_addr;
	size_t max_addr;
	size_t addr_align;
	const Elf_Dyn *dt_symtab;
	const Elf_Dyn *dt_syment;
	const Elf_Dyn *dt_strtab;
	const Elf_Dyn *dt_strsz;
	void *reloc_cache_map;
	size_t reloc_cache_offset;
	size_t reloc_cache_size;
	void *sym_cache_map;
	size_t sym_cache_offset;
	size_t sym_cache_size;
	const Elf_Phdr *pt_dynamic;
	const Elf_Phdr *pt_interp;
	const Elf_Phdr *pt_gnu_stack;
	const Elf_Phdr *pt_gnu_relro;
	const Elf_Phdr *pt_tls;
	const Elf_Phdr *pt_phdr;
	elf_dep_handler_t dep_handler;
	elf_sym_resolver_t sym_resolver;
	void *userdata;
};

struct elf_sym
{
	const char *name;
	void *addr;
};

static int map_ctx(struct elf_ctx *ctx, size_t offset, size_t size,
                   void **map_ptr, size_t *map_size, void **ptr)
{
	if (!size)
		return -EINVAL;
	if (offset < ctx->min_addr)
		return -EINVAL;
	uintptr_t end;
	if (__builtin_add_overflow(offset, size, &end))
		return -EINVAL;
	if (end > ctx->max_addr)
		return -EINVAL;
	uintptr_t rel_addr;
	if (__builtin_add_overflow(ctx->base_addr, offset, &rel_addr))
		return -EINVAL;
	if (__builtin_add_overflow(size, rel_addr, &end))
		return -EINVAL;
	if (!ctx->vm_space)
	{
		if (map_ptr)
			*map_ptr = NULL;
		if (map_size)
			*map_size = 0;
		*ptr = (void*)rel_addr;
		return 0;
	}
	uintptr_t pad = rel_addr & PAGE_MASK;
	uintptr_t map_addr = rel_addr & ~PAGE_MASK;
	*map_size = PAGE_SIZE + (((end - 1) & ~PAGE_MASK) - (rel_addr & ~PAGE_MASK));
	int ret = vm_map_user(ctx->vm_space, map_addr, *map_size, VM_PROT_RW,
	                      map_ptr);
	if (ret)
		return ret;
	*ptr = (uint8_t*)*map_ptr + pad;
	return 0;
}

static int map_ctx_cache(struct elf_ctx *ctx, size_t offset, size_t size,
                         void **cache_map, size_t *cache_offset,
                         size_t *cache_size, void **ptr)
{
	if (!ctx->vm_space)
		return map_ctx(ctx, offset, size, cache_map, cache_size, ptr);
	if (offset >= *cache_offset
	 && offset + size <= *cache_offset + *cache_size)
	{
		*ptr = &((uint8_t*)*cache_map)[offset - *cache_offset];
		return 0;
	}
	if (*cache_map)
		vm_unmap(*cache_map, *cache_size);
	int ret = map_ctx(ctx, offset, size, cache_map, cache_size, ptr);
	if (ret)
		return ret;
	*cache_offset = offset - ((uint8_t*)*ptr - (uint8_t*)*cache_map);
	return 0;
}

static int get_sym(struct elf_ctx *ctx, size_t symidx, uintptr_t *addr)
{
	size_t sym_off = ctx->dt_symtab->d_un.d_ptr
	               + symidx * ctx->dt_syment->d_un.d_val;
	const Elf_Sym *sym;
	int ret = map_ctx_cache(ctx, sym_off, sizeof(sym),
	                        &ctx->sym_cache_map,
	                        &ctx->sym_cache_offset,
	                        &ctx->sym_cache_size,
	                        (void**)&sym);
	if (ret)
	{
		TRACE("failed to map sym");
		return ret;
	}
	if (sym->st_shndx != SHN_UNDEF)
	{
		*addr = sym->st_value;
		return 0;
	}
	if (ctx->vm_space)
	{
		TRACE("sym is undef");
		return -EINVAL;
	}
	if (sym->st_name >= ctx->dt_strsz->d_un.d_val)
	{
		TRACE("sym name out of bounds");
		return -EINVAL;
	}
	char *sym_name;
	size_t sym_max_len = ctx->dt_strsz->d_un.d_val - sym->st_name;
	ret = map_ctx(ctx, ctx->dt_strtab->d_un.d_ptr + sym->st_name,
	              sym_max_len, NULL, NULL, (void**)&sym_name);
	if (ret)
		return ret;
	void *ksym = ksym_get(g_kern_ksym_ctx, sym_name,
	                      ELF_ST_TYPE(sym->st_info));
	if (ksym)
	{
		*addr = (uintptr_t)ksym - ctx->base_addr;
		return 0;
	}
	if (ctx->flags & ELF_KMOD)
	{
		ksym = ctx->sym_resolver(sym_name, ELF_ST_TYPE(sym->st_info),
		                         ctx->userdata);
		if (ksym)
		{
			*addr = (uintptr_t)ksym - ctx->base_addr;
			return 0;
		}
	}
	TRACE("sym %.*s not found", (int)sym_max_len, sym_name);
	return -EINVAL;
}

static int handle_relocation(struct elf_ctx *ctx, void *dst, size_t reladdr,
                             size_t type, size_t symidx, size_t addend,
                             size_t offset)
{
	(void)offset;
	(void)reladdr;
	int ret = 0;
	switch (type)
	{
#ifdef R_NONE
		case R_NONE:
			break;
#endif
#ifdef R_RELATIVE32
		case R_RELATIVE32:
		{
			*(uint32_t*)dst = ctx->base_addr + addend;
			DEBUG_RELOCATION("R_RELATIVE32", reladdr, uint32_t, dst);
			break;
		}
#endif
#ifdef R_RELATIVE64
		case R_RELATIVE64:
		{
			*(uint64_t*)dst = ctx->base_addr + addend;
			DEBUG_RELOCATION("R_RELATIVE64", reladdr, uint64_t, dst);
			break;
		}
#endif
#ifdef R_JMP_SLOT32
		case R_JMP_SLOT32:
		{
			uintptr_t sym;
			ret = get_sym(ctx, symidx, &sym);
			if (ret)
				break;
			*(uint32_t*)dst = ctx->base_addr + sym;
			DEBUG_RELOCATION("R_JMP_SLOT32", reladdr, uint32_t, dst);
			break;
		}
#endif
#ifdef R_JMP_SLOT64
		case R_JMP_SLOT64:
		{
			uintptr_t sym;
			ret = get_sym(ctx, symidx, &sym);
			if (ret)
				break;
			*(uint64_t*)dst = ctx->base_addr + sym;
			DEBUG_RELOCATION("R_JMP_SLOT64", reladdr, uint64_t, dst);
			break;
		}
#endif
#ifdef R_GLOB_DAT32
		case R_GLOB_DAT32:
		{
			uintptr_t sym;
			ret = get_sym(ctx, symidx, &sym);
			if (ret)
				break;
			*(uint32_t*)dst = ctx->base_addr + sym;
			DEBUG_RELOCATION("R_GLOB_DAT32", reladdr, uint32_t, dst);
			break;
		}
#endif
#ifdef R_GLOB_DAT64
		case R_GLOB_DAT64:
		{
			uintptr_t sym;
			ret = get_sym(ctx, symidx, &sym);
			if (ret)
				break;
			*(uint64_t*)dst = ctx->base_addr + sym;
			DEBUG_RELOCATION("R_GLOB_DAT64", reladdr, uint64_t, dst);
			break;
		}
#endif
#ifdef R_ABS32
		case R_ABS32:
		{
			uintptr_t sym;
			ret = get_sym(ctx, symidx, &sym);
			if (ret)
				break;
			*(uint32_t*)dst = ctx->base_addr + sym + addend;
			DEBUG_RELOCATION("R_ABS32", reladdr, uint32_t, dst);
			break;
		}
#endif
#ifdef R_ABS64
		case R_ABS64:
		{
			uintptr_t sym;
			ret = get_sym(ctx, symidx, &sym);
			if (ret)
				break;
			*(uint64_t*)dst = ctx->base_addr + sym + addend;
			DEBUG_RELOCATION("R_ABS64", reladdr, uint64_t, dst);
			break;
		}
#endif
#ifdef R_PC32
		case R_PC32:
		{
			uintptr_t sym;
			ret = get_sym(ctx, symidx, &sym);
			if (ret)
				break;
			*(uint32_t*)dst = sym - offset + addend;
			DEBUG_RELOCATION("R_PC32", reladdr, uint32_t, dst);
			break;
		}
#endif
		default:
			TRACE("unhandled reloc type: 0x%" PRIx32,
			       type);
			return -ENOEXEC;
	}
	return ret;
}

static int handle_rela(struct elf_ctx *ctx, const Elf_Rela *rela)
{
	void *dst;
	int ret = map_ctx_cache(ctx, rela->r_offset, sizeof(uintptr_t),
	                        &ctx->reloc_cache_map,
	                        &ctx->reloc_cache_offset,
	                        &ctx->reloc_cache_size,
	                        (void**)&dst);
	if (ret)
	{
		TRACE("failed to map rela");
		return ret;
	}
	return handle_relocation(ctx, dst,
	                         ctx->base_addr + rela->r_offset,
	                         ELF_R_TYPE(rela->r_info),
	                         ELF_R_SYM(rela->r_info),
	                         rela->r_addend,
	                         rela->r_offset);
}

static int handle_rel(struct elf_ctx *ctx, const Elf_Rel *rel)
{
	void *dst;
	int ret = map_ctx_cache(ctx, rel->r_offset, sizeof(uintptr_t),
	                        &ctx->reloc_cache_map,
	                        &ctx->reloc_cache_offset,
	                        &ctx->reloc_cache_size,
	                        (void**)&dst);
	if (ret)
	{
		TRACE("failed to map rel");
		return ret;
	}
	return handle_relocation(ctx, dst,
	                         ctx->base_addr + rel->r_offset,
	                         ELF_R_TYPE(rel->r_info),
	                         ELF_R_SYM(rel->r_info),
	                         *(uintptr_t*)dst,
	                         rel->r_offset);
}

static int handle_dt_rel(struct elf_ctx *ctx, const Elf_Dyn *rel,
                         const Elf_Dyn *relsz, size_t size)
{
	void *map_ptr;
	size_t map_size;
	uint8_t *rel_ptr;
	int ret = map_ctx(ctx, rel->d_un.d_ptr, relsz->d_un.d_val,
	                  &map_ptr, &map_size, (void**)&rel_ptr);
	if (ret)
	{
		TRACE("failed to map DT_REL");
		return ret;
	}
	for (size_t i = 0; i < relsz->d_un.d_val; i += size)
	{
		const Elf_Rel *r = (const Elf_Rel*)&rel_ptr[i];
		ret = handle_rel(ctx, r);
		if (ret)
			goto end;
	}
	ret = 0;

end:
	if (map_ptr)
		vm_unmap(map_ptr, map_size);
	return ret;
}

static int handle_dt_rela(struct elf_ctx *ctx, const Elf_Dyn *rela,
                          const Elf_Dyn *relasz, size_t size)
{
	void *map_ptr;
	size_t map_size;
	uint8_t *rela_ptr;
	int ret = map_ctx(ctx, rela->d_un.d_ptr, relasz->d_un.d_val,
	                  &map_ptr, &map_size, (void**)&rela_ptr);
	if (ret)
	{
		TRACE("failed to map DT_RELA");
		return ret;
	}
	for (size_t i = 0; i < relasz->d_un.d_val; i += size)
	{
		const Elf_Rela *r = (const Elf_Rela*)&rela_ptr[i];
		ret = handle_rela(ctx, r);
		if (ret)
			goto end;
	}
	ret = 0;

end:
	if (map_ptr)
		vm_unmap(map_ptr, map_size);
	return ret;
}

static int handle_pt_dynamic(struct elf_ctx *ctx)
{
	const Elf_Dyn *dt_rel = NULL;
	const Elf_Dyn *dt_relsz = NULL;
	const Elf_Dyn *dt_relent = NULL;
	const Elf_Dyn *dt_rela = NULL;
	const Elf_Dyn *dt_relasz = NULL;
	const Elf_Dyn *dt_relaent = NULL;
	const Elf_Dyn *dt_jmprel = NULL;
	const Elf_Dyn *dt_pltrel = NULL;
	const Elf_Dyn *dt_pltrelsz = NULL;
	const Elf_Dyn *dt_hash = NULL;
	const Elf_Dyn *dt_bind_now = NULL;
	const Elf_Dyn *dt_flags_1 = NULL;
	if (ctx->pt_dynamic->p_memsz != ctx->pt_dynamic->p_filesz)
	{
		TRACE("PT_DYNAMIC filesz != memsz");
		return -ENOEXEC;
	}
	void *map_ptr;
	size_t map_size;
	uint8_t *dyn_ptr;
	int ret = map_ctx(ctx, ctx->pt_dynamic->p_vaddr, ctx->pt_dynamic->p_memsz,
	                  &map_ptr, &map_size, (void**)&dyn_ptr);
	if (ret)
	{
		TRACE("failed to map PT_DYNAMIC");
		return ret;
	}
	for (size_t i = 0; i < ctx->pt_dynamic->p_filesz; i += sizeof(Elf_Dyn))
	{
		const Elf_Dyn *dyn = (const Elf_Dyn*)&dyn_ptr[i];
		if (dyn->d_tag == DT_NULL)
			break;
		switch (dyn->d_tag)
		{
			case DT_STRTAB:
				if (ctx->dt_strtab)
				{
					TRACE("multiple DT_STRTAB");
					ret = -ENOEXEC;
					goto end;
				}
				ctx->dt_strtab = dyn;
				break;
			case DT_STRSZ:
				if (ctx->dt_strsz)
				{
					TRACE("multiple DT_STRSZ");
					ret = -ENOEXEC;
					goto end;
				}
				ctx->dt_strsz = dyn;
				break;
			case DT_SYMTAB:
				if (ctx->dt_symtab)
				{
					TRACE("multiple DT_SYMTAB");
					ret = -ENOEXEC;
					goto end;
				}
				ctx->dt_symtab = dyn;
				break;
			case DT_SYMENT:
				if (ctx->dt_syment)
				{
					TRACE("multiple DT_DYMENT");
					ret = -ENOEXEC;
					goto end;
				}
				ctx->dt_syment = dyn;
				break;
			case DT_REL:
				if (dt_rel)
				{
					TRACE("multiple DT_REL");
					ret = -ENOEXEC;
					goto end;
				}
				dt_rel = dyn;
				break;
			case DT_RELSZ:
				if (dt_relsz)
				{
					TRACE("multiple DT_RELSZ");
					ret = -ENOEXEC;
					goto end;
				}
				dt_relsz = dyn;
				break;
			case DT_RELENT:
				if (dt_relent)
				{
					TRACE("multiple DT_RELENT");
					ret = -ENOEXEC;
					goto end;
				}
				dt_relent = dyn;
				break;
			case DT_RELA:
				if (dt_rela)
				{
					TRACE("multiple DT_RELA");
					ret = -ENOEXEC;
					goto end;
				}
				dt_rela = dyn;
				break;
			case DT_RELASZ:
				if (dt_relasz)
				{
					TRACE("multiple DT_RELASZ");
					ret = -ENOEXEC;
					goto end;
				}
				dt_relasz = dyn;
				break;
			case DT_RELAENT:
				if (dt_relaent)
				{
					TRACE("multiple DT_RELAENT");
					ret = -ENOEXEC;
					goto end;
				}
				dt_relaent = dyn;
				break;
			case DT_JMPREL:
				if (dt_jmprel)
				{
					TRACE("multiple DT_JMPREL");
					ret = -ENOEXEC;
					goto end;
				}
				dt_jmprel = dyn;
				break;
			case DT_PLTREL:
				if (dt_pltrel)
				{
					TRACE("multiple DT_PTLREL");
					ret = -ENOEXEC;
					goto end;
				}
				dt_pltrel = dyn;
				break;
			case DT_PLTRELSZ:
				if (dt_pltrelsz)
				{
					TRACE("multiple DT_PLTRELSZ");
					ret = -ENOEXEC;
					goto end;
				}
				dt_pltrelsz = dyn;
				break;
			case DT_HASH:
				if (dt_hash)
				{
					TRACE("multiple DT_HASH");
					ret = -ENOEXEC;
					goto end;
				}
				dt_hash = dyn;
				break;
			case DT_BIND_NOW:
				if (dt_bind_now)
				{
					TRACE("multiple DT_BIND_NOW");
					ret = -ENOEXEC;
					goto end;
				}
				dt_bind_now = dyn;
				break;
			case DT_FLAGS_1:
				if (dt_flags_1)
				{
					TRACE("multiple DT_FLAGS_1");
					ret = -ENOEXEC;
					goto end;
				}
				dt_flags_1 = dyn;
				break;
			case DT_NEEDED:
				if (ctx->flags & ELF_INTERP)
				{
					TRACE("DT_NEEDED in interpreter");
					ret = -ENOEXEC;
					goto end;
				}
				break;
			case DT_GNU_HASH:
			case DT_DEBUG:
			case DT_TEXTREL:
			case DT_FLAGS:
			case DT_RELCOUNT:
			case DT_PLTGOT:
			case DT_RELACOUNT:
			case DT_SONAME:
			case DT_VERSYM:
			case DT_VERDEF:
			case DT_VERDEFNUM:
			case DT_VERNEED:
			case DT_VERNEEDNUM:
				break;
			default:
				TRACE("unhandled dyn tag 0x%zx",
				      dyn->d_tag);
				ret = -ENOEXEC;
				goto end;
		}
	}
	if (!ctx->dt_strtab)
	{
		TRACE("no DT_STRTAB");
		ret = -ENOEXEC;
		goto end;
	}
	if (!ctx->dt_strsz)
	{
		TRACE("no DT_STRSZ");
		ret = -ENOEXEC;
		goto end;
	}
	if (!ctx->dt_symtab)
	{
		TRACE("no DT_SYMTAB");
		ret = -ENOEXEC;
		goto end;
	}
	if (!ctx->dt_syment)
	{
		TRACE("no DT_SYMENT");
		ret = -ENOEXEC;
		goto end;
	}
	if (!dt_hash)
	{
		TRACE("no DT_HASH");
		ret = -ENOEXEC;
		goto end;
	}
	if (!dt_bind_now)
	{
		TRACE("no DT_BID_NOW");
		ret = -ENOEXEC;
		goto end;
	}
	if (!dt_flags_1)
	{
		TRACE("no DT_FLAGS_1");
		ret = -ENOEXEC;
		goto end;
	}
	if (!(dt_flags_1->d_un.d_val & DF_1_NOW))
	{
		TRACE("no DF_1_NOW");
		ret = -ENOEXEC;
		goto end;
	}
	if ((ctx->flags & ELF_INTERP) || !ctx->vm_space)
	{
		if (dt_flags_1->d_un.d_val & DF_1_PIE)
		{
			TRACE("unexpected DF_1_PIE");
			ret = -ENOEXEC;
			goto end;
		}
	}
	else if (!(dt_flags_1->d_un.d_val & DF_1_PIE))
	{
		TRACE("no DF_1_PIE");
		ret = -ENOEXEC;
		goto end;
	}
	if (ctx->flags & ELF_KMOD)
	{
		for (size_t i = 0; i < ctx->pt_dynamic->p_filesz; i += sizeof(Elf_Dyn))
		{
			const Elf_Dyn *dyn = (const Elf_Dyn*)&dyn_ptr[i];
			if (dyn->d_tag == DT_NULL)
				break;
			if (dyn->d_tag != DT_NEEDED)
				continue;
			if (dyn->d_un.d_val >= ctx->dt_strsz->d_un.d_val)
			{
				TRACE("sym name out of bounds");
				ret = -EINVAL;
				goto end;
			}
			char *dep_name;
			size_t dep_max_len = ctx->dt_strsz->d_un.d_val - dyn->d_un.d_val;
			ret = map_ctx(ctx, ctx->dt_strtab->d_un.d_ptr + dyn->d_un.d_val,
			              dep_max_len, NULL, NULL, (void**)&dep_name);
			if (ret)
				goto end;
			ret = ctx->dep_handler(dep_name, ctx->userdata);
			if (ret)
				goto end;
		}
	}
	if (dt_rel)
	{
		if (!dt_relsz || !dt_relent)
		{
			TRACE("DT_REL without DT_RELSZ or DT_RELENT");
			ret = -ENOEXEC;
			goto end;
		}
		ret = handle_dt_rel(ctx, dt_rel, dt_relsz,
		                    dt_relent->d_un.d_val);
		if (ret)
			goto end;
	}
	if (dt_rela)
	{
		if (!dt_relasz || !dt_relaent)
		{
			TRACE("DT_RELA without DT_RELASZ or DT_RELAENT");
			ret = -ENOEXEC;
			goto end;
		}
		ret = handle_dt_rela(ctx, dt_rela, dt_relasz,
		                     dt_relaent->d_un.d_val);
		if (ret)
			goto end;
	}
	if (dt_jmprel)
	{
		if (!dt_pltrel || !dt_pltrelsz)
		{
			TRACE("DT_JMPREL without DT_PLTREL or DT_PLTRELSZ");
			ret = -ENOEXEC;
			goto end;
		}
		switch (dt_pltrel->d_un.d_val)
		{
			case DT_REL:
				ret = handle_dt_rel(ctx, dt_jmprel,
				                    dt_pltrelsz,
				                    sizeof(Elf_Rel));
				if (ret)
					goto end;
				break;
			case DT_RELA:
				ret = handle_dt_rela(ctx, dt_jmprel,
				                     dt_pltrelsz,
				                     sizeof(Elf_Rela));
				if (ret)
					goto end;
				break;
			default:
				TRACE("invalid DT_PLTREL type: 0x%zx",
				      dt_pltrel->d_un.d_val);
				ret = -ENOEXEC;
				goto end;
		}
	}
	ret = 0;

end:
	if (map_ptr)
		vm_unmap(map_ptr, map_size);
	return ret;
}

static uint32_t phdr_flags_to_prot(const Elf_Phdr *phdr)
{
	uint32_t prot = 0;
	if (phdr->p_flags & PF_X)
		prot |= VM_PROT_X;
	if (phdr->p_flags & PF_W)
		prot |= VM_PROT_W;
	if (phdr->p_flags & PF_R)
		prot |= VM_PROT_R;
	return prot;
}

static int handle_pt_load_user(struct elf_ctx *ctx, const Elf_Phdr *phdr)
{
	size_t align = PAGE_SIZE;
	if (phdr->p_align > align)
		align = phdr->p_align;
	size_t vaddr = ctx->base_addr + phdr->p_vaddr;
	size_t valign = vaddr % PAGE_SIZE;
	vaddr -= valign;
	size_t fsize = phdr->p_filesz;
	size_t vsize = phdr->p_memsz + valign + PAGE_SIZE - 1;
	vsize -= vsize % PAGE_SIZE;
	size_t poffset = phdr->p_offset;
	size_t poffset_align = poffset % PAGE_SIZE;
	poffset -= poffset_align;
	if (poffset_align != valign)
	{
		TRACE("invalid PT_LOAD align");
		return -EINVAL;
	}
	struct vm_zone *zone;
	fsize += valign;
	size_t fsize_pad = (PAGE_SIZE - fsize) % PAGE_SIZE;
	fsize += fsize_pad;
	uint32_t prot = phdr_flags_to_prot(phdr);
	int ret = vm_alloc(ctx->vm_space, vaddr, poffset,
	                   fsize, 0, prot, MAP_PRIVATE, ctx->file, &zone);
	if (ret)
	{
		TRACE("failed to allocate PT_LOAD from vm space");
		return ret;
	}
	ret = file_mmap(ctx->file, zone);
	if (ret)
	{
		TRACE("failed to mmap PT_LOAD");
		vm_free(ctx->vm_space, vaddr, fsize);
		return ret;
	}
	if (fsize_pad)
	{
		uintptr_t page_addr = vaddr + fsize - PAGE_SIZE;
		void *ptr;
		ret = vm_map_user(ctx->vm_space, page_addr, PAGE_SIZE,
		                  VM_PROT_W, &ptr);
		if (ret)
		{
			TRACE("failed to vmap PT_LOAD");
			vm_free(ctx->vm_space, vaddr, fsize);
			return ret;
		}
		memset(&((uint8_t*)ptr)[PAGE_SIZE - fsize_pad], 0, fsize_pad);
		vm_unmap(ptr, PAGE_SIZE);
	}
	if (vsize != fsize)
	{
		ret = vm_alloc(ctx->vm_space, vaddr + fsize, 0,
		               vsize - fsize, 0, prot,
		               MAP_PRIVATE | MAP_ANONYMOUS, NULL, &zone);
		if (ret)
		{
			TRACE("failed to allocate PT_LOAD virtual padding");
			vm_free(ctx->vm_space, vaddr, fsize);
			return ret;
		}
	}
	return 0;
}

static int handle_pt_load_kern(struct elf_ctx *ctx, const Elf_Phdr *phdr)
{
	size_t vaddr = ctx->base_addr + phdr->p_vaddr;
	size_t valign = vaddr & PAGE_MASK;
	vaddr -= valign;
	size_t vsize = (phdr->p_memsz + valign + PAGE_MASK) & ~PAGE_MASK;
	for (size_t i = 0; i < vsize; i += PAGE_SIZE)
	{
		struct page *page;
		int ret = pm_alloc_page(&page);
		if (ret)
			panic("failed to allocate page\n"); /* XXX */
		ret = arch_vm_map(NULL, vaddr + i, page->offset, PAGE_SIZE, VM_PROT_RW);
		pm_free_page(page);
		if (ret)
			panic("XXX: unmap prev pages, return NULL\n"); /* XXX */
	}
	ssize_t ret = file_readseq(ctx->file, (void*)(vaddr + valign),
	                           phdr->p_filesz, phdr->p_offset);
	if (ret < 0)
	{
		TRACE("failed to read PT_LOAD");
		return ret;
	}
	if ((size_t)ret != phdr->p_filesz)
	{
		TRACE("failed to read full PT_LOAD");
		return -ENOEXEC;
	}
	if (phdr->p_filesz != phdr->p_memsz)
		memset(&((uint8_t*)(vaddr + valign))[phdr->p_filesz], 0, phdr->p_memsz - phdr->p_filesz);
	uint32_t prot = phdr_flags_to_prot(phdr);
	ret = vm_protect(NULL, vaddr, vsize, prot);
	if (ret)
	{
		TRACE("failed to protect PT_LOAD");
		return ret;
	}
	return 0;
}

static int handle_pt_gnu_relro(struct elf_ctx *ctx)
{
	size_t vaddr = ctx->pt_gnu_relro->p_vaddr;
	size_t vaddr_align = vaddr % PAGE_SIZE;
	vaddr -= vaddr_align;
	size_t vsize = ctx->pt_gnu_relro->p_memsz;
	vsize += vaddr_align;
	vsize += PAGE_SIZE - 1;
	vsize -= vsize % PAGE_SIZE;
	int ret;
	if (ctx->vm_space)
		ret = vm_space_protect(ctx->vm_space, vaddr + ctx->base_addr,
		                       vsize, VM_PROT_R);
	else
		ret = vm_protect(NULL, vaddr + ctx->base_addr,
		                 vsize, VM_PROT_R);
	if (ret)
	{
		TRACE("failed to vprotect region");
		return ret;
	}
	return 0;
}

static int get_min_max_addr(struct elf_ctx *ctx)
{
	ctx->min_addr = SIZE_MAX;
	ctx->max_addr = 0;
	for (size_t i = 0; i < ctx->ehdr.e_phnum; ++i)
	{
		const Elf_Phdr *phdr = &ctx->phdr[i];
		if (phdr->p_type != PT_LOAD)
			continue;
		size_t align = PAGE_SIZE;
		if (phdr->p_align > align)
			align = phdr->p_align;
		uintptr_t vaddr = phdr->p_vaddr;
		vaddr -= vaddr % align;
		size_t vsize = phdr->p_memsz + phdr->p_vaddr - vaddr;
		vsize += align - 1;
		vsize -= vsize % align;
		if (vaddr < ctx->min_addr)
			ctx->min_addr = vaddr;
		if (vaddr + vsize > ctx->max_addr)
			ctx->max_addr = vaddr + vsize;
	}
	if (ctx->min_addr >= ctx->max_addr)
	{
		TRACE("invalid vaddr");
		return -ENOEXEC;
	}
	ctx->map_size = ctx->max_addr - ctx->min_addr;
	return 0;
}

static int handle_map_user(struct elf_ctx *ctx)
{
	int ret = get_min_max_addr(ctx);
	if (ret)
		return ret;
	if (ctx->flags & ELF_INTERP)
	{
		struct vm_zone *zone;
		ret = vm_alloc(ctx->vm_space, 0, 0, ctx->map_size, 0,
		               VM_PROT_RW, MAP_PRIVATE | MAP_ANONYMOUS,
		               NULL, &zone);
		if (ret)
		{
			TRACE("failed to allocate interp vmem");
			return ret;
		}
		ctx->base_addr = zone->addr - ctx->min_addr;
		ret = vm_free(ctx->vm_space, zone->addr, zone->size);
		if (ret)
		{
			TRACE("failed to release interp vmem");
			return ret;
		}
	}
	else
	{
		ctx->base_addr = ctx->vm_space->region.addr;
	}
	ctx->base_addr += ctx->addr_align - 1;
	ctx->base_addr -= ctx->base_addr % ctx->addr_align;
	ctx->map_base = ctx->base_addr + ctx->min_addr;
	mutex_lock(&ctx->vm_space->mutex);
	for (size_t i = 0; i < ctx->ehdr.e_phnum; ++i)
	{
		const Elf_Phdr *phdr = &ctx->phdr[i];
		if (phdr->p_type != PT_LOAD)
			continue;
		ret = handle_pt_load_user(ctx, phdr);
		if (ret)
		{
			mutex_unlock(&ctx->vm_space->mutex);
			return ret;
		}
	}
	mutex_unlock(&ctx->vm_space->mutex);
	return 0;
}

static int handle_map_kern(struct elf_ctx *ctx)
{
	int ret = get_min_max_addr(ctx);
	if (ret)
		return ret;
	if (ctx->map_size > 128 * 1024 * 1024) /* XXX less arbitrary, per-arch ? */
	{
		TRACE("kmod too big");
		return -ENOEXEC;
	}
	mutex_lock(&g_vm_mutex);
	if (vm_region_alloc(&g_vm_heap, 0, ctx->map_size + ctx->addr_align,
	                    0, &ctx->map_base))
	{
		TRACE("failed to alloc kmod vmem (0x%zx bytes)", ctx->map_size);
		mutex_unlock(&g_vm_mutex);
		return -ENOMEM;
	}
	ctx->base_addr = ctx->map_base - ctx->min_addr;
	ctx->base_addr += ctx->addr_align - 1;
	ctx->base_addr -= ctx->base_addr % ctx->addr_align;
	for (size_t i = 0; i < ctx->ehdr.e_phnum; ++i)
	{
		const Elf_Phdr *phdr = &ctx->phdr[i];
		if (phdr->p_type != PT_LOAD)
			continue;
		ret = handle_pt_load_kern(ctx, phdr);
		if (ret)
		{
			mutex_unlock(&g_vm_mutex);
			return ret;
		}
	}
	mutex_unlock(&g_vm_mutex);
	return 0;
}

static int handle_interp(struct elf_ctx *ctx, struct elf_info *info)
{
	if (ctx->flags & ELF_INTERP)
	{
		TRACE("recursive interpreter not supported");
		return -ENOEXEC;
	}
	if (ctx->flags & ELF_KMOD)
	{
		TRACE("interpreter in kmod");
		return -ENOEXEC;
	}
	char path[MAXPATHLEN];
	if (ctx->pt_interp->p_filesz >= MAXPATHLEN)
	{
		TRACE("PT_INTERP too big");
		return -ENOEXEC;
	}
	if (!ctx->pt_interp->p_filesz)
	{
		TRACE("empty PT_INTERP");
		return -ENOEXEC;
	}
	ssize_t ret = file_readseq(ctx->file, path, ctx->pt_interp->p_filesz,
	                           ctx->pt_interp->p_offset);
	if (ret < 0)
	{
		TRACE("failed to read PT_INTERP");
		return ret;
	}
	if ((size_t)ret != ctx->pt_interp->p_filesz)
	{
		TRACE("failed to read full PT_INTERP");
		return -ENOEXEC;
	}
	path[ctx->pt_interp->p_filesz] = '\0';
	struct node *node;
	ret = vfs_getnode(NULL, path, 0, &node);
	if (ret)
	{
		TRACE("failed to get interp node");
		return ret;
	}
	struct file *interp_file;
	ret = file_fromnode(node, 0, &interp_file);
	node_free(node);
	if (ret)
	{
		TRACE("failed to create file from interp node");
		return ret;
	}
	ret = elf_createctx(interp_file, ctx->vm_space, ctx->flags | ELF_INTERP,
	                    ctx->dep_handler, ctx->sym_resolver,
	                    ctx->userdata, info);
	file_free(interp_file);
	return ret;
}

static int check_ehdr(const Elf_Ehdr *ehdr)
{
	if (ehdr->e_ident[EI_MAG0] != ELFMAG0
	 || ehdr->e_ident[EI_MAG1] != ELFMAG1
	 || ehdr->e_ident[EI_MAG2] != ELFMAG2
	 || ehdr->e_ident[EI_MAG3] != ELFMAG3)
	{
		TRACE("invalid header magic");
		return -ENOEXEC;
	}
	if (ehdr->e_ident[EI_CLASS] != ELFCLASS)
	{
		TRACE("invalid header class");
		return -ENOEXEC;
	}
	if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB)
	{
		TRACE("invalid header data");
		return -ENOEXEC;
	}
	if (ehdr->e_ident[EI_VERSION] != EV_CURRENT)
	{
		TRACE("invalid header version");
		return -ENOEXEC;
	}
	if (ehdr->e_type != ET_DYN)
	{
		TRACE("not a dynamic binary");
		return -ENOEXEC;
	}
	if (ehdr->e_machine != ELFEM)
	{
		TRACE("invalid header machine");
		return -ENOEXEC;
	}
	if (ehdr->e_version != EV_CURRENT)
	{
		TRACE("invalid header version");
		return -ENOEXEC;
	}
	if (ehdr->e_shentsize != sizeof(Elf_Shdr))
	{
		TRACE("invalid section entry size");
		return -ENOEXEC;
	}
	if (ehdr->e_phentsize != sizeof(Elf_Phdr))
	{
		TRACE("invalid program entry size");
		return -ENOEXEC;
	}
	if (ehdr->e_shstrndx >= ehdr->e_shnum)
	{
		TRACE("invalid shstrtab position");
		return -ENOEXEC;
	}
	return 0;
}

static int load_phdr(struct elf_ctx *ctx)
{
	ctx->phdr = malloc(ctx->ehdr.e_phentsize * ctx->ehdr.e_phnum, 0);
	if (!ctx->phdr)
	{
		TRACE("failed to malloc phdr");
		return -ENOMEM;
	}
	ssize_t ret = file_readseq(ctx->file, ctx->phdr,
	                           ctx->ehdr.e_phentsize * ctx->ehdr.e_phnum,
	                           ctx->ehdr.e_phoff);
	if (ret < 0)
	{
		TRACE("failed to read phdr");
		return ret;
	}
	if ((size_t)ret != ctx->ehdr.e_phentsize * ctx->ehdr.e_phnum)
	{
		TRACE("failed to read full phdr");
		return -ENOEXEC;
	}
	for (size_t i = 0; i < ctx->ehdr.e_phnum; ++i)
	{
		const Elf_Phdr *phdr = &ctx->phdr[i];
		switch (phdr->p_type)
		{
			case PT_INTERP:
				if (ctx->pt_interp)
				{
					TRACE("multiple PT_INTERP");
					return -ENOEXEC;
				}
				ctx->pt_interp = phdr;
				break;
			case PT_GNU_STACK:
				if (ctx->pt_gnu_stack)
				{
					TRACE("multiple PT_GNU_STACK");
					return -ENOEXEC;
				}
				if (phdr->p_flags != (PF_R | PF_W))
				{
					TRACE("invalid PT_GNU_STACK protection");
					return -ENOEXEC;
				}
				ctx->pt_gnu_stack = phdr;
				break;
			case PT_GNU_RELRO:
				if (ctx->pt_gnu_relro)
				{
					TRACE("multiple PT_GNU_RELRO");
					return -ENOEXEC;
				}
				ctx->pt_gnu_relro = phdr;
				break;
			case PT_LOAD:
				if (phdr->p_filesz > phdr->p_memsz)
				{
					TRACE("p_filesz > p_memsz");
					return -ENOEXEC;
				}
				if ((phdr->p_flags & (PF_W | PF_X)) == (PF_W | PF_X))
				{
					TRACE("PT_LOAD has PF_W | PF_X");
					return -ENOEXEC;
				}
				if (!phdr->p_align)
				{
					TRACE("PT_LOAD has invalid alignment");
					return -ENOEXEC;
				}
				if (phdr->p_align > 0x10000)
				{
					TRACE("PT_LOAD alignment too big");
					return -ENOEXEC;
				}
				if (phdr->p_align > ctx->addr_align)
					ctx->addr_align = phdr->p_align;
				break;
			case PT_DYNAMIC:
				if (ctx->pt_dynamic)
				{
					TRACE("multiple PT_DYNAMIC");
					return -ENOEXEC;
				}
				ctx->pt_dynamic = phdr;
				break;
			case PT_TLS:
				if (ctx->pt_tls)
				{
					TRACE("multiple PT_TLS");
					return -ENOEXEC;
				}
				ctx->pt_tls = phdr;
				break;
			case PT_PHDR:
				if (ctx->pt_phdr)
				{
					TRACE("multiple PT_PHDR");
					return -ENOEXEC;
				}
				ctx->pt_phdr = phdr;
				break;
			case PT_EXIDX:
			case PT_RISCV_ATTRIBUTES:
				break;
			default:
				TRACE("unknown phdr: 0x%" PRIx32, phdr->p_type);
				return -ENOEXEC;
		}
	}
	if (!ctx->pt_gnu_stack)
	{
		TRACE("no PT_GNU_STACK");
		return -ENOEXEC;
	}
	if (!ctx->pt_gnu_relro)
	{
		TRACE("no PT_GNU_RELRO");
		return -ENOEXEC;
	}
	if (!ctx->pt_dynamic)
	{
		TRACE("no PT_DYNAMIC");
		return -ENOEXEC;
	}
	if (!ctx->pt_phdr && !(ctx->flags & (ELF_INTERP | ELF_KMOD)))
	{
		TRACE("no PT_PHDR");
		return -ENOEXEC;
	}
	if (ctx->pt_tls && !ctx->pt_interp)
	{
		TRACE("PT_TLS found");
		return -ENOEXEC;
	}
	if (ctx->pt_interp && (ctx->flags & ELF_INTERP))
	{
		TRACE("nested interpreter");
		return -ENOEXEC;
	}
	return 0;
}

static int load_ehdr(struct elf_ctx *ctx)
{
	ssize_t ret = file_readseq(ctx->file, &ctx->ehdr, sizeof(ctx->ehdr), 0);
	if (ret < 0)
	{
		TRACE("failed to read ehdr");
		return ret;
	}
	if ((size_t)ret < sizeof(ctx->ehdr))
	{
		TRACE("file too short");
		return -ENOEXEC;
	}
	return check_ehdr(&ctx->ehdr);
}

static int ctx_init(struct elf_ctx *ctx, struct file *file,
                    struct vm_space *vm_space, int flags)
{
	memset(ctx, 0, sizeof(*ctx));
	ctx->vm_space = vm_space;
	ctx->file = file;
	ctx->flags = flags;
	return 0;
}

static void ctx_destroy(struct elf_ctx *ctx)
{
	if (ctx->reloc_cache_map)
		vm_unmap(ctx->reloc_cache_map, ctx->reloc_cache_size);
	if (ctx->sym_cache_map)
		vm_unmap(ctx->sym_cache_map, ctx->sym_cache_size);
	if (!ctx->vm_space && ctx->base_addr)
		vfree((void*)ctx->base_addr, ctx->map_size);
	free(ctx->phdr);
}

int elf_createctx(struct file *file, struct vm_space *vm_space, int flags,
                  elf_dep_handler_t dep_handler,
                  elf_sym_resolver_t sym_resolver,
                  void *userdata,
                  struct elf_info *info)
{
	ssize_t ret;
	struct elf_ctx ctx;

	ret = ctx_init(&ctx, file, vm_space, flags);
	if (ret)
		return ret;
	ctx.dep_handler = dep_handler;
	ctx.sym_resolver = sym_resolver;
	ctx.userdata = userdata;
	ret = load_ehdr(&ctx);
	if (ret)
		goto end;
	ret = load_phdr(&ctx);
	if (ret)
		goto end;
	/* XXX verify PT_LOAD are non-overlapping */
	/* XXX max the number of PT_LOAD */
	if (vm_space)
		ret = handle_map_user(&ctx);
	else
		ret = handle_map_kern(&ctx);
	if (ret)
		goto end;
	if (ctx.pt_interp)
	{
		ret = handle_interp(&ctx, info);
		if (ret)
			goto end;
	}
	else
	{
		ret = handle_pt_dynamic(&ctx);
		if (ret)
			goto end;
		ret = handle_pt_gnu_relro(&ctx);
		if (ret)
			goto end;
	}

	info->base_addr = ctx.base_addr;
	info->map_base = ctx.map_base;
	info->map_size = ctx.map_size;
	info->min_addr = ctx.min_addr;
	info->max_addr = ctx.max_addr;
	if (ctx.pt_phdr)
		info->phaddr = ctx.base_addr + ctx.pt_phdr->p_vaddr;
	else
		info->phaddr = ctx.base_addr + ctx.ehdr.e_phoff;
	info->phnum = ctx.ehdr.e_phnum;
	info->phent = ctx.ehdr.e_phentsize;
	if (ctx.pt_interp)
		info->real_entry = info->entry;
	info->entry = ctx.base_addr + ctx.ehdr.e_entry;
	if (!ctx.pt_interp)
		info->real_entry = info->entry;
	ctx.base_addr = 0;
	ret = 0;

end:
	ctx_destroy(&ctx);
	return ret;
}
