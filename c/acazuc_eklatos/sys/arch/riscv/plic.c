#define ENABLE_TRACE

#include <endian.h>
#include <errno.h>
#include <cpu.h>
#include <fdt.h>
#include <std.h>
#include <mem.h>

static const struct fdt_node *plic_fdt_node;
static uintptr_t plic_paddr;
static uint32_t *plic_data; /* source priority + pending bits */

int plic_init_fdt(const struct fdt_node *node)
{
	plic_fdt_node = node;
	struct fdt_prop *reg = fdt_get_prop(node, "reg");
	if (!reg)
	{
		TRACE("plic: no 'reg' property");
		return -EINVAL;
	}
	uintptr_t mmio_base;
	size_t mmio_size;
	int ret = fdt_get_base_size_reg(reg, 0, &mmio_base, &mmio_size);
	if (ret)
	{
		TRACE("plic: invalid reg");
		return ret;
	}
	if (mmio_size < PAGE_SIZE * 2)
	{
		TRACE("plic: invalid reg mmio size");
		return -EINVAL;
	}
	plic_paddr = mmio_base;
	struct page page;
	pm_init_page(&page, mmio_base / PAGE_SIZE);
	plic_data = vm_map(&page, PAGE_SIZE * 2, VM_PROT_RW);
	if (!plic_data)
	{
		TRACE("plic: failed to map data");
		return -ENOMEM;
	}
	return 0;
}

/* XXX could be cached */
static int get_cpus_node(const struct fdt_node **nodep)
{
	const struct fdt_node *node;
	TAILQ_FOREACH(node, &fdt_nodes, chain)
	{
		const struct fdt_node *child;
		TAILQ_FOREACH(child, &node->children, chain)
		{
			if (strcmp(child->name, "cpus"))
				continue;
			*nodep = child;
			return 0;
		}
	}
	return -ENOENT;
}

static int get_cpu_node(struct cpu *cpu, const struct fdt_node **nodep)
{
	const struct fdt_node *cpus_node;
	int ret = get_cpus_node(&cpus_node);
	if (ret)
		return ret;
	const struct fdt_node *node;
	TAILQ_FOREACH(node, &cpus_node->children, chain)
	{
		const struct fdt_prop *cpu_reg = fdt_get_prop(node, "reg");
		if (!cpu_reg)
			continue;
		if (cpu_reg->len != 4)
			continue;
		if (cpu->arch.hartid != be32toh(*(uint32_t*)cpu_reg->data))
			continue;
		*nodep = node;
		return 0;
	}
	return -ENOENT;
}

static int get_intc_node(const struct fdt_node *cpu_node,
                         const struct fdt_node **intc_node)
{
	const struct fdt_node *node;
	TAILQ_FOREACH(node, &cpu_node->children, chain)
	{
		if (fdt_check_compatible(node, "riscv,cpu-intc"))
			continue;
		*intc_node = node;
		return 0;
	}
	return -ENOENT;
}

static int get_intc_phandle(const struct fdt_node *intc_node, uint32_t *phandlep)
{
	struct fdt_prop *intc_phandle_prop = fdt_get_prop(intc_node, "phandle");
	if (!intc_phandle_prop)
		return -ENOENT;
	if (intc_phandle_prop->len != 4)
		return -EINVAL;
	*phandlep = be32toh(*(uint32_t*)intc_phandle_prop->data);
	return 0;
}

static int get_cpu_intc_phandle(struct cpu *cpu, uint32_t *phandle)
{
	const struct fdt_node *cpu_node;
	int ret = get_cpu_node(cpu, &cpu_node);
	if (ret)
	{
		TRACE("plic: failed to find cpu node");
		return ret;
	}
	const struct fdt_node *intc_node;
	ret = get_intc_node(cpu_node, &intc_node);
	if (ret)
	{
		TRACE("plic: failed to find intc node");
		return ret;
	}
	ret = get_intc_phandle(intc_node, phandle);
	if (ret)
	{
		TRACE("pliuc: failed to get intc phandle");
		return ret;
	}
	return 0;
}

int plic_setup_cpu(struct cpu *cpu)
{
	struct fdt_prop *int_prop = fdt_get_prop(plic_fdt_node,
	                                         "interrupts-extended");
	if (!int_prop)
	{
		TRACE("plic: no 'interrupts-extended' property");
		return -EINVAL;
	}
	if (int_prop->len % 8)
	{
		TRACE("plic: invalid 'interrupts-extended' property length");
		return -EINVAL;
	}
	uint32_t intc_phandle;
	int ret = get_cpu_intc_phandle(cpu, &intc_phandle);
	if (ret)
		return ret;
	cpu->arch.plic_ctx_id = UINT16_MAX;
	size_t nint = int_prop->len / 8;
	for (size_t i = 0; i < nint; ++i)
	{
		uint32_t phandle = be32toh(((uint32_t*)int_prop->data)[i * 2 + 0]);
		uint32_t mode = be32toh(((uint32_t*)int_prop->data)[i * 2 + 1]);
		if (phandle != intc_phandle)
			continue;
		if (mode != 9) /* S-mode external interrupt source */
			continue;
		cpu->arch.plic_ctx_id = i;
	}
	if (cpu->arch.plic_ctx_id == UINT16_MAX)
	{
		TRACE("plic: failed to find context id");
		return -ENOENT;
	}
	struct page page;
	pm_init_page(&page, (plic_paddr + 0x2000 + cpu->arch.plic_ctx_id * 0x80) / PAGE_SIZE);
	cpu->arch.plic_enable_data = vm_map(&page, PAGE_SIZE, VM_PROT_RW);
	if (!cpu->arch.plic_enable_data)
	{
		TRACE("plic: failed to map enable data");
		return -ENOMEM;
	}
	cpu->arch.plic_enable_data = (uint8_t*)cpu->arch.plic_enable_data + ((cpu->arch.plic_ctx_id * 0x80) % PAGE_SIZE);
	pm_init_page(&page, (plic_paddr + 0x200000 + cpu->arch.plic_ctx_id * 0x1000) / PAGE_SIZE);
	cpu->arch.plic_control_data = vm_map(&page, PAGE_SIZE, VM_PROT_RW);
	if (!cpu->arch.plic_control_data)
	{
		TRACE("plic: failed to map control data");
		return -ENOMEM;
	}
	*(uint32_t volatile*)&((uint8_t*)cpu->arch.plic_control_data)[0] = 0;
	return 0;
}

void plic_enable_interrupt(size_t id)
{
	assert(id && id <= 1023, "invalid IRQ id\n");
	*(uint32_t volatile*)&((uint8_t*)plic_data)[id * 4] = 1;
}

void plic_disable_interrupt(size_t id)
{
	assert(id && id <= 1023, "invalid IRQ id\n");
	*(uint32_t volatile*)&((uint8_t*)plic_data)[id * 4] = 0;
}

void plic_enable_cpu_int(struct cpu *cpu, size_t id)
{
	assert(id && id <= 1023, "invalid IRQ id\n");
	*(uint32_t volatile*)&((uint8_t*)cpu->arch.plic_enable_data)[id / 32] |= (1 << (id % 32));
}

void plic_eoi(struct cpu *cpu, size_t id)
{
	*(uint32_t volatile*)&((uint8_t*)cpu->arch.plic_control_data)[4] = id;
}

int plic_get_active_interrupt(struct cpu *cpu, size_t *irq)
{
	*irq = *(uint32_t volatile*)&((uint8_t*)cpu->arch.plic_control_data)[4];
	if (!*irq)
		return -ENOENT;
	return 0;
}
