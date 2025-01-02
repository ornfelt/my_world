#define ENABLE_TRACE

#include <arch/elf.h>

#include <errno.h>
#include <kmod.h>
#include <file.h>
#include <proc.h>
#include <ksym.h>
#include <sma.h>
#include <vfs.h>
#include <uio.h>
#include <std.h>
#include <mem.h>

TAILQ_HEAD(kmod_head, kmod);

static struct spinlock kmod_list_lock = SPINLOCK_INITIALIZER(); /* XXX rwlock */
static struct kmod_head kmod_list = TAILQ_HEAD_INITIALIZER(kmod_list);

static struct sma kmod_sma;
static struct sma kmod_dep_sma;

void kmod_init(void)
{
	sma_init(&kmod_sma, sizeof(struct kmod), NULL, NULL, "kmod");
	sma_init(&kmod_dep_sma, sizeof(struct kmod_dep), NULL, NULL, "kmod_dep");
}

static int kmod_sys_open(struct file *file, struct node *node)
{
	file->userdata = node->userdata;
	return 0;
}

static ssize_t kmod_sys_read(struct file *file, struct uio *uio)
{
	struct kmod *kmod = file->userdata;
	size_t count = uio->count;
	off_t off = uio->off;
	uprintf(uio, "%s\n", kmod->info->name);
	uprintf(uio, "base addr: 0x%zx\n", kmod->elf_info.base_addr);
	uprintf(uio, "map: 0x%zx-0x%zx\n", kmod->elf_info.map_base,
	        kmod->elf_info.map_base + kmod->elf_info.map_size);
	uprintf(uio, "dependencies:");
	struct kmod_dep *dep;
	TAILQ_FOREACH(dep, &kmod->deps, mod_chain)
		uprintf(uio, " %s", dep->dep->info->name);
	uprintf(uio, "\n");
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op kmod_fop =
{
	.open = kmod_sys_open,
	.read = kmod_sys_read,
};

void kmod_free(struct kmod *kmod)
{
	if (refcount_dec(&kmod->refcount))
		return;
	if (kmod->flags & KMOD_FLAG_INITIALIZED)
	{
		assert(kmod->flags & KMOD_FLAG_UNLOADED, "freeing initialized and non-unloaded module\n");
		spinlock_lock(&kmod_list_lock);
		if (refcount_get(&kmod->refcount))
		{
			spinlock_unlock(&kmod_list_lock);
			return;
		}
		TAILQ_REMOVE(&kmod_list, kmod, chain);
		spinlock_unlock(&kmod_list_lock);
	}
	vfree((void*)kmod->elf_info.map_base, kmod->elf_info.map_size);
	sma_free(&kmod_sma, kmod);
}

static int load_ksym(struct kmod *kmod)
{
	for (size_t i = 0; i < kmod->elf_info.phnum; ++i)
	{
		const Elf_Phdr *phdr = (const Elf_Phdr*)(kmod->elf_info.phaddr
		                                       + kmod->elf_info.phent * i);
		if (phdr->p_type == PT_DYNAMIC)
		{
			kmod->ksym = ksym_alloc(kmod->elf_info.base_addr,
			                        kmod->elf_info.map_base,
			                        kmod->elf_info.map_size,
			                        kmod->elf_info.base_addr + phdr->p_vaddr,
			                        phdr->p_filesz);
			if (!kmod->ksym)
				return -ENOEXEC; /* XXX */
			return 0;
		}
	}
	return -ENOEXEC;
}

static int handle_dep(const char *name, void *userdata)
{
	struct kmod *kmod = userdata;
	char dep_name[256];
	if (strlcpy(dep_name, name, sizeof(dep_name)) >= sizeof(dep_name))
		return -EINVAL;
	size_t len = strlen(dep_name);
	if (len <= 5 || strcmp(&dep_name[len - 5], ".kmod"))
		return -EINVAL;
	dep_name[len - 5] = '\0';
	struct kmod *dep = kmod_get(dep_name);
	if (!dep)
		return -EINVAL;
	struct kmod_dep *dep_link = sma_alloc(&kmod_dep_sma, 0);
	if (!dep_link)
		kmod_free(dep);
	dep_link->mod = kmod;
	dep_link->dep = dep;
	TAILQ_INSERT_TAIL(&kmod->deps, dep_link, mod_chain);
	TAILQ_INSERT_TAIL(&dep->parents, dep_link, dep_chain);
	return 0;
}

static void *resolve_sym(const char *name, int type, void *userdata)
{
	struct kmod *kmod = userdata;
	struct kmod_dep *dep;
	TAILQ_FOREACH(dep, &kmod->deps, mod_chain)
	{
		void *ksym = ksym_get(dep->dep->ksym, name, type);
		if (ksym)
			return (uint8_t*)ksym + dep->dep->elf_info.base_addr;
	}
	return NULL;
}

int kmod_load(struct file *file, struct kmod **kmodp)
{
	struct kmod *kmod;
	int ret;

	kmod = sma_alloc(&kmod_sma, M_ZERO);
	if (!kmod)
		return -ENOMEM;
	refcount_init(&kmod->refcount, 1);
	TAILQ_INIT(&kmod->deps);
	TAILQ_INIT(&kmod->parents);
	ret = elf_createctx(file, NULL, ELF_KMOD, handle_dep, resolve_sym,
	                    kmod, &kmod->elf_info);
	if (ret)
	{
		sma_free(&kmod_sma, kmod);
		return ret;
	}
	kmod->info = (void*)kmod->elf_info.real_entry;
	if (kmod->info->magic != KMOD_MAGIC)
	{
		TRACE("invalid kmod magic");
		kmod_free(kmod);
		return -ENOEXEC;
	}
	if (kmod->info->version != 1)
	{
		TRACE("invalid kmod api version");
		kmod_free(kmod);
		return -ENOEXEC;
	}
	if (!*kmod->info->name)
	{
		TRACE("empty kmod name");
		kmod_free(kmod);
		return -ENOEXEC;
	}
	struct kmod *tmp = kmod_get(kmod->info->name);
	if (tmp)
	{
		kmod_free(tmp);
		kmod_free(kmod);
		return -EEXIST;
	}
	if (load_ksym(kmod))
	{
		kmod_free(kmod);
		return -ENOEXEC;
	}
	spinlock_lock(&kmod_list_lock);
	TAILQ_INSERT_TAIL(&kmod_list, kmod, chain);
	spinlock_unlock(&kmod_list_lock);
	if (kmod->info->init)
	{
		ret = kmod->info->init();
		if (ret)
		{
			kmod_free(kmod);
			return ret;
		}
	}
#if 0
	printf("loaded module %s @ %lx\n", kmod->info->name,
	       kmod->elf_info.base_addr);
#endif
	char sysfs_path[1024];
	snprintf(sysfs_path, sizeof(sysfs_path), "kmod/%s", kmod->info->name);
	ret = sysfs_mknode(sysfs_path, 0, 0, 0400, &kmod_fop, &kmod->sysfs_node);
	if (ret)
		printf("failed to create kmod sysfs node: %s\n", strerror(ret));
	else
		kmod->sysfs_node->userdata = kmod;
	kmod->flags |= KMOD_FLAG_INITIALIZED;
	*kmodp = kmod;
	return 0;
}

int kmod_unload(struct kmod *kmod)
{
	if (kmod->flags & KMOD_FLAG_UNLOADED)
		return -EBUSY;
	if (kmod->info->fini)
		kmod->info->fini();
	struct kmod_dep *dep = TAILQ_FIRST(&kmod->deps);
	while (dep)
	{
		TAILQ_REMOVE(&dep->dep->parents, dep, dep_chain);
		kmod_free(dep->dep);
		sma_free(&kmod_dep_sma, dep);
		dep = TAILQ_FIRST(&kmod->deps);
	}
	kmod->flags |= KMOD_FLAG_UNLOADED;
	ksym_free(kmod->ksym);
	kmod_free(kmod);
	return 0;
}

void kmod_ref(struct kmod *kmod)
{
	refcount_inc(&kmod->refcount);
}

struct kmod *kmod_get(const char *name)
{
	struct kmod *kmod;
	spinlock_lock(&kmod_list_lock);
	TAILQ_FOREACH(kmod, &kmod_list, chain)
	{
		if (!strcmp(kmod->info->name, name))
		{
			kmod_ref(kmod);
			break;
		}
	}
	spinlock_unlock(&kmod_list_lock);
	return kmod;
}

struct kmod *kmod_find_by_addr(uintptr_t addr)
{
	struct kmod *kmod;
	spinlock_lock(&kmod_list_lock);
	TAILQ_FOREACH(kmod, &kmod_list, chain)
	{
		if (addr >= kmod->elf_info.map_base
		 && addr < kmod->elf_info.map_base + kmod->elf_info.map_size)
		{
			kmod_ref(kmod);
			break;
		}
	}
	spinlock_unlock(&kmod_list_lock);
	return kmod;
}

const char *kmod_get_sym(struct kmod *kmod, uintptr_t addr, uintptr_t *off)
{
	return ksym_find_by_addr(kmod->ksym, addr, off);
}
