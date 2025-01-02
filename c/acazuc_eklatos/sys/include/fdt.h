#ifndef FDT_H
#define FDT_H

#include <queue.h>
#include <types.h>

TAILQ_HEAD(fdt_node_head, fdt_node);

struct fdt_prop
{
	const char *name;
	uint32_t len;
	TAILQ_ENTRY(fdt_prop) chain;
	uint8_t data[];
};

struct fdt_node
{
	char *name;
	struct fdt_node *parent;
	struct fdt_node_head children;
	TAILQ_HEAD(, fdt_prop) properties;
	TAILQ_ENTRY(fdt_node) chain;
};

extern struct fdt_node_head fdt_nodes;

int fdt_init(void);
struct fdt_prop *fdt_get_prop(const struct fdt_node *node, const char *name);
int fdt_get_base_size_reg(const struct fdt_prop *prop, size_t id,
                          uintptr_t *basep, size_t *sizep);
int fdt_get_ecam_addr(uint8_t bus, uint8_t slot, uint8_t func,
                      uintptr_t *poffp);
int fdt_check_compatible(const struct fdt_node *node, const char *str);
int fdt_find_phandle(uint32_t phandle, struct fdt_node **nodep);

#endif
