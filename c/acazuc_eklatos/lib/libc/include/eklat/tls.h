#ifndef EKLAT_TLS_H
#define EKLAT_TLS_H

#include <sys/queue.h>

#include <stdint.h>
#include <stddef.h>

struct elf;

struct tls_module
{
	uint8_t *data;
	struct elf *elf;
};

struct tls_block
{
	void *static_ptr;
	struct tls_module *mods;
	size_t mods_size;
	size_t initial_mods_count;
	size_t initial_size;
	void *initial_data;
	void *static_allocation;
	TAILQ_ENTRY(tls_block) chain;
};

static inline struct tls_block *get_tls_block(void)
{
#if defined(__arm__) || defined(__aarch64__)
	return (struct tls_block*)((uint8_t*)__builtin_thread_pointer() - sizeof(struct tls_block) + sizeof(void*) * 2);
#elif defined(__riscv) || defined(__mips__)
	return (struct tls_block*)((uint8_t*)__builtin_thread_pointer() - sizeof(struct tls_block));
#elif defined(__i386__) || defined(__x86_64__)
	return __builtin_thread_pointer();
#else
# error "unknown arch"
#endif
}

#endif
