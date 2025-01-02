#ifndef KSYM_H
#define KSYM_H

#include <types.h>

struct ksym_ctx;

extern struct ksym_ctx *g_kern_ksym_ctx;

void ksym_init(void);
struct ksym_ctx *ksym_alloc(uintptr_t base_addr,
                            uintptr_t map_base, uintptr_t map_size,
                            uintptr_t dyn_addr, size_t dyn_size);
void ksym_free(struct ksym_ctx *ctx);
void *ksym_get(struct ksym_ctx *ctx, const char *name, uint8_t type);
const char *ksym_find_by_addr(struct ksym_ctx *ctx, uintptr_t addr,
                              uintptr_t *off);

#endif
