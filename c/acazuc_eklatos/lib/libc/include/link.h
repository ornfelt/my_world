#ifndef LINK_H
#define LINK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <elf32.h>
#include <elf64.h>

#if __SIZE_WIDTH__ == 32
# define ElfW(n) Elf32_##n
#elif __SIZE_WIDTH__ == 64
# define ElfW(n) Elf64_##n
#endif

struct dl_phdr_info
{
	ElfW(Addr) dlpi_addr;
	const char *dlpi_name;
	const ElfW(Phdr) *dlpi_phdr;
	ElfW(Half) dlpi_phnum;
	unsigned long long dlpi_adds;
	unsigned long long dlpi_subs;
	size_t dlpi_tls_modid;
	void *dlpi_tls_data;
};

int dl_iterate_phdr(int (*cb)(struct dl_phdr_info *info,
                              size_t size, void *data),
                    void *data);

#ifdef __cplusplus
}
#endif

#endif
