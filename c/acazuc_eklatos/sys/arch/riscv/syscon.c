#define ENABLE_TRACE

#include <endian.h>
#include <errno.h>
#include <std.h>
#include <fdt.h>
#include <mem.h>

static uint32_t *syscon_poweroff_data;
static uint32_t syscon_poweroff_value;
static uint32_t *syscon_reboot_data;
static uint32_t syscon_reboot_value;

static int get_syscon_regmap(uint32_t regmap, uintptr_t *addrp, size_t *sizep)
{
	const struct fdt_node *node;
	int ret = fdt_find_phandle(regmap, (struct fdt_node**)&node);
	if (ret)
	{
		TRACE("syscon: failed to find phande");
		return ret;
	}
	struct fdt_prop *reg = fdt_get_prop(node, "reg");
	if (!reg)
	{
		TRACE("syscon: no 'reg' property");
		return -EINVAL;
	}
	ret = fdt_get_base_size_reg(reg, 0, addrp, sizep);
	if (ret)
	{
		TRACE("syscon: invalid reg");
		return ret;
	}
	return 0;
}

static int extract_mmio_value(const struct fdt_node *node, uint32_t **datap,
                              uint32_t *valuep)
{
	const struct fdt_prop *offset_prop = fdt_get_prop(node, "offset");
	if (!offset_prop)
	{
		TRACE("syscon: no offset property");
		return -EINVAL;
	}
	if (offset_prop->len != 4)
	{
		TRACE("syscon: invalid offset len");
		return -EINVAL;
	}
	const struct fdt_prop *value_prop = fdt_get_prop(node, "value");
	if (!value_prop)
	{
		TRACE("syscon: no value property");
		return -EINVAL;
	}
	if (value_prop->len != 4)
	{
		TRACE("syscon: invalid value len");
		return -EINVAL;
	}
	const struct fdt_prop *regmap_prop = fdt_get_prop(node, "regmap");
	if (!regmap_prop)
	{
		TRACE("syscon: no regmap property");
		return -EINVAL;
	}
	if (regmap_prop->len != 4)
	{
		TRACE("syscon: invalid regmap len");
		return -EINVAL;
	}
	uint32_t offset = be32toh(*(uint32_t*)offset_prop->data);
	uint32_t value = be32toh(*(uint32_t*)value_prop->data);
	uint32_t regmap = be32toh(*(uint32_t*)regmap_prop->data);
	if (offset % 4)
	{
		TRACE("syscon: unaligned offset");
		return -EINVAL;
	}
	uintptr_t syscon_addr;
	size_t syscon_size;
	int ret = get_syscon_regmap(regmap, &syscon_addr, &syscon_size);
	if (ret)
		return ret;
	if (syscon_size < 4
	 || offset > syscon_size - 4)
	{
		TRACE("syscon: offset out of bounds");
		return -EINVAL;
	}
	struct page page;
	pm_init_page(&page, (syscon_addr + offset) / PAGE_SIZE);
	void *data = vm_map(&page, PAGE_SIZE, VM_PROT_RW);
	if (!data)
	{
		TRACE("syscon: failed to map syscon data");
		return -ENOMEM;
	}
	*datap = (uint32_t*)((uint8_t*)data + (syscon_addr + offset) % PAGE_SIZE);
	*valuep = value;
	return 0;
}

int syscon_init_poweroff_fdt(const struct fdt_node *node)
{
	return extract_mmio_value(node, &syscon_poweroff_data,
	                          &syscon_poweroff_value);
}

int syscon_init_reboot_fdt(const struct fdt_node *node)
{
	return extract_mmio_value(node, &syscon_reboot_data,
	                          &syscon_reboot_value);
}

int syscon_poweroff(void)
{
	if (!syscon_poweroff_data)
		return -EXDEV;
	*syscon_poweroff_data = syscon_poweroff_value;
	return 0;
}

int syscon_reboot(void)
{
	if (!syscon_reboot_data)
		return -EXDEV;
	*syscon_reboot_data = syscon_reboot_value;
	return 0;
}
