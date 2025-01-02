#ifndef LD_H
#define LD_H

#include <eklat/tls.h>

#include <sys/queue.h>
#include <sys/param.h>

#include <stddef.h>

#define LD_ERR(fmt, ...) \
do \
{ \
	snprintf(g_ld_errbuf, sizeof(g_ld_errbuf), fmt, ##__VA_ARGS__); \
	g_ld_err = 1; \
} while (0)

#if defined(__i386__)
# include "elf_i386.h"
#elif defined(__x86_64__)
# include "elf_amd64.h"
#elif defined(__arm__)
# include "elf_arm.h"
#elif defined(__aarch64__)
# include "elf_aarch64.h"
#elif defined(__riscv_xlen) && __riscv_xlen == 32
# include "elf_riscv32.h"
#elif defined(__riscv_xlen) && __riscv_xlen == 64
# include "elf_riscv64.h"
#elif defined(__mips__)
# if defined(__mips64)
#  include "elf_mips64.h"
# else
#  include "elf_mips.h"
# endif
#else
# error "unknown arch"
#endif

TAILQ_HEAD(tls_head, tls_block);

struct elf_link
{
	struct elf *elf;
	struct elf *dep;
	TAILQ_ENTRY(elf_link) elf_chain;
	TAILQ_ENTRY(elf_link) dep_chain;
};

TAILQ_HEAD(elf_link_head, elf_link);

struct elf
{
	char path[MAXPATHLEN];
	int from_auxv;
	void *entry;
	size_t phnum;
	size_t vaddr_min;
	size_t vaddr_max;
	size_t vaddr;
	size_t vsize;
	const Elf_Phdr *phdr;
	struct elf_link_head neededs;
	struct elf_link_head parents;
	const Elf_Dyn *dt_symtab;
	const Elf_Dyn *dt_syment;
	const Elf_Dyn *dt_strtab;
	const Elf_Dyn *dt_strsz;
	const Elf_Dyn *dt_init_array;
	const Elf_Dyn *dt_init_arraysz;
	const Elf_Dyn *dt_fini_array;
	const Elf_Dyn *dt_fini_arraysz;
	const Elf_Dyn *dt_hash;
	const Elf_Dyn *dt_gnu_hash;
	const Elf_Dyn *dt_rel;
	const Elf_Dyn *dt_relsz;
	const Elf_Dyn *dt_relent;
	const Elf_Dyn *dt_rela;
	const Elf_Dyn *dt_relasz;
	const Elf_Dyn *dt_relaent;
	const Elf_Dyn *dt_jmprel;
	const Elf_Dyn *dt_pltrel;
	const Elf_Dyn *dt_pltrelsz;
	const Elf_Dyn *dt_init;
	const Elf_Dyn *dt_fini;
	const Elf_Dyn *dt_flags;
	const Elf_Dyn *dt_flags_1;
	const Elf_Dyn *dt_bind_now;
	const Elf_Phdr *pt_phdr;
	const Elf_Phdr *pt_tls;
	const Elf_Phdr *pt_dynamic;
	const Elf_Phdr *pt_gnu_stack;
	const Elf_Phdr *pt_gnu_relro;
	int loaded;
	int has_tls_module;
	size_t tls_module;
	size_t tls_offset;
	size_t refcount;
	TAILQ_ENTRY(elf) chain;
};

TAILQ_HEAD(elf_head, elf);

extern struct tls_head tls_list;
extern struct elf_head elf_list;

typedef void (*init_fn_t)(void);
typedef void (*fini_fn_t)(void);

struct dl_phdr_info;

void *_dl_open(const char *filenae, int flags);
int _dl_close(void *handle);
char *_dl_error(void);
void *_dl_sym(void *handle, const char *symbol);
void *_dl_tls_alloc(void);
void _dl_tls_free(void *ptr);
int _dl_tls_set(void *ptr);
int _dl_iterate_phdr(int (*cb)(struct dl_phdr_info *info,
                               size_t size, void *data),
                      void *data);

struct elf *elf_from_path(const char *path);
struct elf *elf_from_auxv(const char *path);
struct elf *elf_from_fd(const char *path, int fd);
void elf_free(struct elf *elf);
int elf_finalize(struct elf *elf);
int find_elf_sym(struct elf *elf, const char *name, uint8_t type,
                 struct elf **symelf, uintptr_t *addr);

int create_dynamic_tls(struct elf *elf);
void cleanup_dynamic_tls(struct elf *elf);
int create_initial_tls(struct elf *elf);
int generate_dynamic_tls_pointers(struct tls_block *dup,
                                  struct tls_block *tls);
struct tls_block *tls_block_alloc(void);
void tls_block_free(struct tls_block *tls);

extern int g_initial_elf;
extern size_t g_page_size;
extern char g_ld_errbuf[4096];
extern char g_ld_err;

#endif
