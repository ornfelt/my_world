#define ENABLE_TRACE

#include <arch/elf.h>

#include <multiboot.h>
#include <errno.h>
#include <ksym.h>
#include <std.h>
#include <sma.h>

struct ksym_ctx
{
	uintptr_t base_addr;
	uintptr_t map_base;
	uintptr_t map_size;
	const Elf_Dyn *dt_hash;
	const Elf_Dyn *dt_gnu_hash;
	const Elf_Dyn *dt_strtab;
	const Elf_Dyn *dt_symtab;
	const Elf_Dyn *dt_strsz;
	const Elf_Dyn *dt_syment;
};

struct ksym_ctx *g_kern_ksym_ctx;

static struct sma ksym_sma;

extern uint8_t _kernel_begin;
extern uint8_t _kernel_end;

static uint32_t elf_hash(const char *name)
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

static uint32_t gnu_hash(const char *name)
{
	uint32_t h = 5381;
	while (*name)
		h = (h << 5) + h + *(uint8_t*)(name++);
	return h;
}

static int test_sym(struct ksym_ctx *ctx, const Elf_Sym *sym,
                    const char *name, uint8_t type, void **addr)
{
	if (sym->st_shndx == SHN_UNDEF)
		return 1;
	if (ELF_ST_BIND(sym->st_info) != STB_GLOBAL)
		return 1;
	if (type && ELF_ST_TYPE(sym->st_info) != type)
		return 1;
	const char *sym_name = (const char *)(ctx->base_addr
	                                    + ctx->dt_strtab->d_un.d_ptr
	                                    + sym->st_name);
	if (strcmp(sym_name, name))
		return 1;
	*addr = (void*)sym->st_value;
	return 0;
}

static int find_elf_sym_hash(struct ksym_ctx *ctx, const char *name,
                             uint8_t type, void **addr)
{
	uint32_t *hashptr = (uint32_t*)(ctx->base_addr + ctx->dt_hash->d_un.d_ptr);
	uint32_t buckets_count = hashptr[0];
	uint32_t chain_count = hashptr[1];
	uint32_t *buckets = &hashptr[2];
	uint32_t *chain = &hashptr[2 + buckets_count];
	uint32_t hash = elf_hash(name);
	for (size_t i = buckets[hash % buckets_count];
	     i && i < chain_count;
	     i = chain[i])
	{
		const Elf_Sym *sym = (const Elf_Sym*)(ctx->base_addr
		                                    + ctx->dt_symtab->d_un.d_ptr
		                                    + ctx->dt_syment->d_un.d_val * i);
		if (!test_sym(ctx, sym, name, type, addr))
			return 0;
	}
	return 1;
}

static int find_elf_sym_gnuh(struct ksym_ctx *ctx, const char *name,
                             uint8_t type, void **addr)
{
	uint32_t *hashptr = (uint32_t*)(ctx->base_addr + ctx->dt_gnu_hash->d_un.d_ptr);
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
		const Elf_Sym *sym = (const Elf_Sym*)(ctx->base_addr
		                                    + ctx->dt_symtab->d_un.d_ptr
		                                    + ctx->dt_syment->d_un.d_val * i);
		uint32_t h = chain[i - symoffset];
		if ((hash & ~1) == (h & ~1))
		{
			if (!test_sym(ctx, sym, name, type, addr))
				return 0;
		}
		if (h & 1)
			break;
		i++;
	}
	return 1;
}

static const Elf_Shdr *get_dynamic_shdr(void)
{
#if defined(__aarch64__) || (defined(__riscv) && __riscv_xlen == 64)
	struct multiboot_tag_elf64_sections *tag = (struct multiboot_tag_elf64_sections*)multiboot_find_tag(MULTIBOOT_TAG_TYPE_ELF64_SECTIONS);
#else
	struct multiboot_tag_elf_sections *tag = (struct multiboot_tag_elf_sections*)multiboot_find_tag(MULTIBOOT_TAG_TYPE_ELF_SECTIONS);
#endif
	if (!tag)
		return NULL;
	const Elf_Shdr *dynamic = NULL;
	for (size_t i = 0; i < tag->num; ++i)
	{
		const Elf_Shdr *shdr = (const Elf_Shdr*)&tag->sections[tag->entsize * i];
		if (shdr->sh_type != SHT_DYNAMIC)
			continue;
		if (dynamic)
			panic("multiple SHT_DYNAMIC\n");
		dynamic = shdr;
		break;
	}
	return dynamic;
}

void ksym_init(void)
{
	sma_init(&ksym_sma, sizeof(struct ksym_ctx), NULL, NULL, "ksym_ctx");
	const Elf_Shdr *sh_dynamic = get_dynamic_shdr();
	if (!sh_dynamic)
		panic("no SHT_DYNAMIC\n");
	g_kern_ksym_ctx = ksym_alloc(0, (uintptr_t)&_kernel_begin,
	                             (uintptr_t)&_kernel_end - (uintptr_t)&_kernel_begin,
	                             sh_dynamic->sh_addr, sh_dynamic->sh_size);
}

struct ksym_ctx *ksym_alloc(uintptr_t base_addr,
                            uintptr_t map_base, uintptr_t map_size,
                            uintptr_t dyn_addr, size_t dyn_size)
{
	struct ksym_ctx *ctx = sma_alloc(&ksym_sma, M_ZERO);
	if (!ctx)
		return NULL;
	ctx->base_addr = base_addr;
	ctx->map_base = map_base;
	ctx->map_size = map_size;
	for (size_t i = 0; i < dyn_size; i += sizeof(Elf_Dyn))
	{
		const Elf_Dyn *dyn = (const Elf_Dyn*)&((uint8_t*)dyn_addr)[i];
		if (dyn->d_tag == DT_NULL)
			break;
		switch (dyn->d_tag)
		{
			case DT_HASH:
				if (ctx->dt_hash)
				{
					TRACE("multiple DT_HASH");
					goto err;
				}
				ctx->dt_hash = dyn;
				break;
			case DT_GNU_HASH:
				if (ctx->dt_gnu_hash)
				{
					TRACE("multiple DT_GNU_HASH");
					goto err;
				}
				ctx->dt_gnu_hash = dyn;
				break;
			case DT_STRTAB:
				if (ctx->dt_strtab)
				{
					TRACE("multiple DT_STRTAB");
					goto err;
				}
				ctx->dt_strtab = dyn;
				break;
			case DT_SYMTAB:
				if (ctx->dt_symtab)
				{
					TRACE("multiple DT_SYMTAB");
					goto err;
				}
				ctx->dt_symtab = dyn;
				break;
			case DT_STRSZ:
				if (ctx->dt_strsz)
				{
					TRACE("multiple DT_STRSZ");
					goto err;
				}
				ctx->dt_strsz = dyn;
				break;
			case DT_SYMENT:
				if (ctx->dt_syment)
				{
					TRACE("multiple DT_SYMENT");
					goto err;
				}
				ctx->dt_syment = dyn;
				break;
			default:
				break;
		}
	}
	if (!ctx->dt_hash)
	{
		TRACE("no DT_HASH");
		goto err;
	}
	if (!ctx->dt_gnu_hash)
	{
		TRACE("no DT_GNU_HASH");
		goto err;
	}
	if (!ctx->dt_strtab)
	{
		TRACE("no DT_STRTAB");
		goto err;
	}
	if (!ctx->dt_symtab)
	{
		TRACE("no DT_SYMTAB");
		goto err;
	}
	if (!ctx->dt_strsz)
	{
		TRACE("no DT_STRSZ");
		goto err;
	}
	if (!ctx->dt_syment)
	{
		TRACE("no DT_SYMENT");
		goto err;
	}
	return ctx;

err:
	sma_free(&ksym_sma, ctx);
	return NULL;
}

void ksym_free(struct ksym_ctx *ctx)
{
	if (!ctx)
		return;
	sma_free(&ksym_sma, ctx);
}

void *ksym_get(struct ksym_ctx *ctx, const char *name, uint8_t type)
{
	void *addr;
	if (ctx->dt_gnu_hash)
	{
		if (!find_elf_sym_gnuh(ctx, name, type, &addr))
			return addr;
	}
	else if (ctx->dt_hash)
	{
		if (!find_elf_sym_hash(ctx, name, type, &addr))
			return addr;
	}
	return NULL;
}

const char *ksym_find_by_addr(struct ksym_ctx *ctx, uintptr_t addr,
                              uintptr_t *off)
{
	if (addr < ctx->map_base || addr >= ctx->map_base + ctx->map_size)
		return NULL;
	addr -= ctx->base_addr;
	uint32_t *hashptr = (uint32_t*)(ctx->base_addr + ctx->dt_hash->d_un.d_ptr);
	uint32_t buckets_count = hashptr[0];
	uint32_t chain_count = hashptr[1];
	uint32_t *buckets = &hashptr[2];
	uint32_t *chain = &hashptr[2 + buckets_count];
	(void)chain;
	(void)buckets;
	const Elf_Sym *best = NULL;
	for (size_t i = 0; i < chain_count; ++i)
	{
		const Elf_Sym *sym = (const Elf_Sym*)(ctx->base_addr
		                                    + ctx->dt_symtab->d_un.d_ptr
		                                    + ctx->dt_syment->d_un.d_val * i);
		if (sym->st_shndx == SHN_UNDEF)
			continue;
		if (sym->st_value > addr)
			continue;
		if (!best || sym->st_value > best->st_value)
			best = sym;
	}
	if (!best)
		return NULL;
	*off = best ? addr - best->st_value : addr;
	return (const char *)(ctx->base_addr + ctx->dt_strtab->d_un.d_ptr + best->st_name);
}
