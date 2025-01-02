#ifndef KMOD_H
#define KMOD_H

#include <refcount.h>
#include <queue.h>
#include <types.h>
#include <proc.h>

#define KMOD_MAGIC     0xAC2FA34D
#define KMOD_NAME_SIZE 32

#define KMOD_FLAG_INITIALIZED (1 << 0)
#define KMOD_FLAG_UNLOADED    (1 << 1)

struct ksym_ctx;
struct file;

typedef int (*kmod_init_t)(void);
typedef void (*kmod_fini_t)(void);

struct kmod_info
{
	uint32_t magic;
	uint32_t version;
	char name[KMOD_NAME_SIZE];
	kmod_init_t init;
	kmod_fini_t fini;
};

struct kmod_dep
{
	struct kmod *mod;
	struct kmod *dep;
	TAILQ_ENTRY(kmod_dep) mod_chain;
	TAILQ_ENTRY(kmod_dep) dep_chain;
};

struct kmod
{
	refcount_t refcount;
	struct kmod_info *info;
	struct elf_info elf_info;
	TAILQ_ENTRY(kmod) chain;
	uint32_t flags;
	struct ksym_ctx *ksym;
	struct node *sysfs_node;
	TAILQ_HEAD(, kmod_dep) deps;
	TAILQ_HEAD(, kmod_dep) parents;
};

int kmod_load(struct file *file, struct kmod **kmod);
int kmod_unload(struct kmod *kmod);
void kmod_free(struct kmod *kmod);
void kmod_ref(struct kmod *kmod);
struct kmod *kmod_get(const char *name);
struct kmod *kmod_find_by_addr(uintptr_t addr);
const char *kmod_get_sym(struct kmod *kmod, uintptr_t addr, uintptr_t *off);

#endif
