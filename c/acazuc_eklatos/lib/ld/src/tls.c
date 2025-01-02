#include "ld.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

static void init_tls_data(const struct tls_module *module)
{
	struct elf *elf = module->elf;
	const Elf_Phdr *pt_tls = elf->pt_tls;
	memcpy(module->data,
	       (uint8_t*)elf->vaddr + pt_tls->p_vaddr,
	       pt_tls->p_filesz);
	memset(module->data + pt_tls->p_filesz, 0,
	       pt_tls->p_memsz - pt_tls->p_filesz);

}

static void generate_initial_tls_offsets(struct elf *elf, size_t *total_size,
                                         size_t *mods_count)
{
	if (elf->pt_tls && !elf->has_tls_module)
	{
		elf->has_tls_module = 1;
		elf->tls_module = (*mods_count)++;
#if defined(__arm__) || defined(__aarch64__) || defined(__riscv) || defined(__mips__)
		elf->tls_offset = *total_size;
		*total_size += elf->pt_tls->p_memsz;
#elif defined(__i386__) || defined(__x86_64__)
		*total_size += elf->pt_tls->p_memsz;
		elf->tls_offset = *total_size;
#else
# error "unknown arch"
#endif
	}
	struct elf_link *link;
	TAILQ_FOREACH(link, &elf->neededs, elf_chain)
		generate_initial_tls_offsets(link->dep, total_size, mods_count);
}

static void generate_initial_tls_pointers(struct elf *elf,
                                          struct tls_block *tls)
{
	if (elf->pt_tls)
	{
		struct tls_module *module = &tls->mods[elf->tls_module];
#if defined(__arm__) || defined(__aarch64__) || defined(__riscv) || defined(__mips__)
		module->data = (uint8_t*)tls->initial_data + elf->tls_offset;
#elif defined(__i386__) || defined(__x86_64__)
		module->data = (uint8_t*)tls->static_ptr - elf->tls_offset;
#else
# error "unknown arch"
#endif
		module->elf = elf;
		init_tls_data(module);
	}
	struct elf_link *link;
	TAILQ_FOREACH(link, &elf->neededs, elf_chain)
		generate_initial_tls_pointers(link->dep, tls);
}

static struct tls_block *allocate_tls(size_t static_size)
{
	uint8_t *data = malloc(static_size + sizeof(struct tls_block));
	if (!data)
	{
		LD_ERR("tls malloc: %s", strerror(errno));
		return NULL;
	}
#if defined(__arm__) || defined(__aarch64__) || defined(__riscv) || defined(__mips__)
	uint8_t *static_data = &data[sizeof(struct tls_block)];
	struct tls_block *tls = (struct tls_block*)&data[0];
#elif defined(__i386__) || defined(__x86_64__)
	uint8_t *static_data = &data[0];
	struct tls_block *tls = (struct tls_block*)&data[static_size];
#else
# error "unknown arch"
#endif
	memset(tls, 0, sizeof(*tls));
	tls->static_ptr = tls;
	tls->initial_data = static_data;
	tls->initial_size = static_size;
	tls->static_allocation = data;
	return tls;
}

int create_initial_tls(struct elf *elf)
{
	size_t total_size = 0;
	size_t mods_count = 1;
	generate_initial_tls_offsets(elf, &total_size, &mods_count);
	struct tls_block *tls = allocate_tls(total_size);
	if (!tls)
		return 1;
	tls->initial_mods_count = mods_count;
	tls->mods_size = mods_count;
	if (total_size)
	{
		tls->mods = malloc(sizeof(*tls->mods) * mods_count);
		if (!tls->mods)
		{
			LD_ERR("tls malloc: %s", strerror(errno));
			free(tls->static_allocation);
			return 1;
		}
		generate_initial_tls_pointers(elf, tls);
	}
	if (!_dl_tls_set(tls))
	{
		LD_ERR("settls: %s", strerror(errno));
		free(tls->mods);
		free(tls->static_allocation);
		return 1;
	}
	TAILQ_INSERT_TAIL(&tls_list, tls, chain);
	return 0;
}

void cleanup_dynamic_tls(struct elf *elf)
{
	struct tls_block *tls;
	TAILQ_FOREACH(tls, &tls_list, chain)
	{
		if (elf->tls_module >= tls->mods_size)
			continue;
		struct tls_module *module = &tls->mods[elf->tls_module];
		free(module->data);
		module->data = NULL;
		module->elf = NULL;
	}
}

int create_dynamic_tls(struct elf *elf)
{
	if (!elf->pt_tls)
		return 0;
	elf->tls_offset = elf->pt_tls->p_memsz;
	struct tls_block *cur_tls = get_tls_block();
	/* NB: because of the locking & cleanup policies,
	 * testing just the current tls is allowed
	 */
	elf->tls_module = 1;
	while (elf->tls_module < cur_tls->mods_size
	    && cur_tls->mods[elf->tls_module].data)
		++elf->tls_module;
	struct tls_block *tls;
	TAILQ_FOREACH(tls, &tls_list, chain)
	{
		if (tls->mods_size <= elf->tls_module)
		{
			struct tls_module *mods = realloc(tls->mods,
			                                  sizeof(*mods)
			                                * (tls->mods_size + 1));
			if (!mods)
			{
				LD_ERR("malloc: %s", strerror(errno));
				cleanup_dynamic_tls(elf);
				return 1;
			}
			tls->mods = mods;
			tls->mods_size++;
		}
		uint8_t *data = malloc(elf->pt_tls->p_memsz);
		if (!data)
		{
			LD_ERR("malloc: %s", strerror(errno));
			cleanup_dynamic_tls(elf);
			return 1;
		}
		struct tls_module *module = &tls->mods[elf->tls_module];
		module->data = data;
		module->elf = elf;
		init_tls_data(module);
	}
	elf->has_tls_module = 1;
	return 0;
}

int generate_dynamic_tls_pointers(struct tls_block *dup,
                                  struct tls_block *tls)
{
	for (size_t i = dup->initial_mods_count; i < dup->mods_size; ++i)
	{
		struct elf *elf = tls->mods[i].elf;
		if (!elf)
			continue;
		if (!elf->pt_tls)
			continue;
		struct tls_module *module = &dup->mods[elf->tls_module];
		if (module->data)
			continue;
		module->data = malloc(elf->tls_offset);
		if (!module->data)
			return 1;
		module->elf = elf;
		init_tls_data(module);
	}
	return 0;
}

struct tls_block *tls_block_alloc(void)
{
	struct tls_block *tls = TAILQ_FIRST(&tls_list);
	struct tls_block *dup = allocate_tls(tls->initial_size);
	if (!dup)
		return NULL;
	dup->mods_size = tls->mods_size;
	dup->mods = calloc(dup->mods_size, sizeof(*dup->mods));
	if (!dup->mods)
	{
		free(dup->static_allocation);
		return NULL;
	}
	dup->initial_mods_count = tls->initial_mods_count;
	for (size_t i = 0; i < dup->initial_mods_count; ++i)
	{
		struct elf *elf = tls->mods[i].elf;
		if (!elf)
			continue;
		struct tls_module *module = &dup->mods[i];
#if defined(__arm__) || defined(__aarch64__) || defined(__riscv) || defined(__mips__)
		module->data = (uint8_t*)dup->initial_data + elf->tls_offset;
#elif defined(__i386__) || defined(__x86_64__)
		module->data = (uint8_t*)dup->static_ptr - elf->tls_offset;
#else
# error "unknown arch"
#endif
		module->elf = elf;
		init_tls_data(module);
	}
	if (generate_dynamic_tls_pointers(dup, tls))
	{
		for (size_t i = dup->initial_mods_count; i < dup->mods_size; ++i)
			free(dup->mods[i].data);
		free(dup->mods);
		free(dup->static_allocation);
		return NULL;
	}
	TAILQ_INSERT_TAIL(&tls_list, dup, chain);
	return dup;
}

void tls_block_free(struct tls_block *tls)
{
	TAILQ_REMOVE(&tls_list, tls, chain);
	for (size_t i = tls->initial_mods_count; i < tls->mods_size; ++i)
		free(tls->mods[i].data);
	free(tls->mods);
	free(tls->static_allocation);
}
