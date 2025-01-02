#define ENABLE_TRACE

#include <errno.h>
#include <fdt.h>
#include <std.h>
#include <mem.h>

static void *clint_data;

static inline uint32_t clint_read(uint32_t reg)
{
	return *(uint32_t volatile*)&((uint8_t*)clint_data)[reg];
}

static inline void clint_write(uint32_t reg, uint32_t val)
{
	*(uint32_t volatile*)&((uint8_t*)clint_data)[reg] = val;
}

int clint_init_addr(uintptr_t addr)
{
	struct page page;
	pm_init_page(&page, addr / PAGE_SIZE);
	clint_data = vm_map(&page, 0x10000, VM_PROT_RW);
	if (!clint_data)
	{
		TRACE("clint: failed to map clint");
		return -ENOMEM;
	}
	return 0;
}

int clint_init_fdt(struct fdt_node *node)
{
	int ret;
	struct fdt_prop *reg = fdt_get_prop(node, "reg");
	if (!reg)
	{
		TRACE("clint: no 'reg' property");
		return -EINVAL;
	}
	uintptr_t mmio_base;
	size_t mmio_size;
	ret = fdt_get_base_size_reg(reg, 0, &mmio_base, &mmio_size);
	if (ret)
	{
		TRACE("clint: invalid mmio reg");
		return ret;
	}
	if (mmio_size < 0x10000)
	{
		TRACE("clint: invalid mmio size: 0x%zx", mmio_size);
		return -EINVAL;
	}
	return clint_init_addr(mmio_base);
}
