#include "ld.h"

#include <eklat/lock.h>

#include <sys/param.h>
#include <sys/auxv.h>

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <link.h>

struct _libc_lock;

int g_initial_elf = 1;
size_t g_page_size;
char g_ld_errbuf[4096];
char g_ld_err;

static uint32_t g_lock;

/* because ld.so functions are all (or at least, must / will be) thread-safe
 * wes can bypass the libc.so errno definitions and allow non-TLS errno
 * this avoids TLS handling in kernel ELF loader
 */
static int _errno;

int *__get_errno(void)
{
	return &_errno;
}

void __set_errno(int err)
{
	_errno = err;
}

#if defined(__i386__)
void __stack_chk_fail(void);

void __stack_chk_fail_local(void)
{
	__stack_chk_fail();
}
#endif

extern size_t *__libc_auxv;
extern char **environ;
extern const char *__libc_progname;
extern uintptr_t __stack_chk_guard;

typedef int (*jmp_t)(int argc, char **argv, char **envp, size_t *auxv);

int main(int argc, char **argv, char **envp)
{
	g_page_size = getauxval(AT_PAGESZ);
	if (argc < 1)
	{
		fprintf(stderr, "ld: no binary given\n");
		exit(EXIT_FAILURE);
	}
	struct elf *elf = elf_from_auxv(argv[0]);
	if (!elf)
	{
		fprintf(stderr, "ld: %s\n", _dl_error());
		exit(EXIT_FAILURE);
	}
	g_initial_elf = 0;
	jmp_t jmp = (jmp_t)getauxval(AT_ENTRY);
	int ret = jmp(argc, argv, envp, __libc_auxv);
	elf_free(elf);
	exit(ret);
}

static void ld_lock(void)
{
	_eklat_lock(&g_lock);
}

static void ld_unlock(void)
{
	_eklat_unlock(&g_lock, NULL);
}

void _libc_lock(struct _libc_lock *lock)
{
	(void)lock;
}

void _libc_unlock(struct _libc_lock *lock)
{
	(void)lock;
}

void *_dl_open(const char *filename, int flags)
{
	ld_lock();
	if (!filename)
	{
		struct elf *ret = TAILQ_FIRST(&elf_list);
		ld_unlock();
		return ret;
	}
	if (!g_page_size)
		g_page_size = getpagesize();
	struct elf *elf = NULL;
	if (!(flags & (RTLD_LAZY | RTLD_NOW))
	 || (flags & (RTLD_LAZY | RTLD_NOW)) == (RTLD_LAZY | RTLD_NOW))
		goto err;
	/* XXX use RTLD_GLOBAL */
	elf = elf_from_path(filename);
	if (!elf)
		goto err;
	ld_unlock();
	return elf;

err:
	if (elf)
		elf_free(elf);
	ld_unlock();
	return NULL;
}

int _dl_close(void *handle)
{
	ld_lock();
	elf_free(handle);
	ld_unlock();
	return 0;
}

char *_dl_error(void)
{
	ld_lock();
	if (!g_ld_err)
	{
		ld_unlock();
		return NULL;
	}
	g_ld_err = 0;
	ld_unlock();
	return g_ld_errbuf;
}

void *_dl_sym(void *handle, const char *symbol)
{
	ld_lock();
	if (handle == RTLD_DEFAULT)
		handle = TAILQ_FIRST(&elf_list);
	uintptr_t sym;
	if (find_elf_sym(handle, symbol, STT_FUNC, NULL, &sym)
	 && find_elf_sym(handle, symbol, STT_OBJECT, NULL, &sym))
		sym = 0;
	ld_unlock();
	return (void*)sym;
}

void *_dl_tls_alloc(void)
{
	ld_lock();
	struct tls_block *tls = tls_block_alloc();
	ld_unlock();
	return tls;
}

void _dl_tls_free(void *ptr)
{
	ld_lock();
	tls_block_free(ptr);
	ld_unlock();
}

int _dl_tls_set(void *ptr)
{
	struct tls_block *tls = ptr;
#if defined(__arm__) || defined(__aarch64__)
	return settls((uint8_t*)tls + sizeof(*tls) - sizeof(void*) * 2) == 0;
#elif defined(__riscv) || defined(__mips__)
	return settls((uint8_t*)tls + sizeof(*tls)) == 0;
#elif defined(__i386__) || defined(__x86_64__)
	return settls(tls) == 0;
#else
# error "unknown arch"
#endif
}

int _dl_iterate_phdr(int (*cb)(struct dl_phdr_info *info,
                               size_t size, void *data),
                     void *data)
{
	ld_lock();
	struct elf *elf;
	int ret = 0;
	TAILQ_FOREACH(elf, &elf_list, chain)
	{
		struct dl_phdr_info info;
		info.dlpi_addr = elf->vaddr;
		info.dlpi_name = elf->path;
		info.dlpi_phdr = elf->phdr;
		info.dlpi_phnum = elf->phnum;
		info.dlpi_adds = 0;
		info.dlpi_subs = 0;
		if (elf->has_tls_module)
		{
			info.dlpi_tls_modid = elf->tls_module;
			info.dlpi_tls_data = get_tls_block()->mods[elf->tls_module].data;
		}
		else
		{
			info.dlpi_tls_modid = 0;
			info.dlpi_tls_data =  NULL;
		}
		/* XXX should ld_lock be release on callback ? */
		ret = cb(&info, sizeof(info), data);
	}
	ld_unlock();
	return ret;
}
