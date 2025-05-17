#define ENABLE_TRACE

#include "pci.h"

#if defined(__i386__) || defined(__x86_64__)
#include <arch/asm.h>
#define PCI_ADDR 0xCF8
#define PCI_DATA 0xCFC
#endif

#include <queue.h>
#include <file.h>
#if WITH_ACPI
#include <acpi.h>
#endif
#include <kmod.h>
#include <std.h>
#include <uio.h>
#include <sma.h>
#include <vfs.h>
#if WITH_FDT
#include <fdt.h>
#endif

struct pci_map
{
	size_t addr;
	size_t offset;
	size_t size;
	union
	{
		size_t base;
		struct
		{
			struct page page;
			uint8_t *data;
		};
	};
};

struct pci_msix
{
	struct pci_map *map;
	uint16_t size;
};

static struct spinlock devices_lock = SPINLOCK_INITIALIZER(); /* XXX rwlock */
static TAILQ_HEAD(, pci_device) devices = TAILQ_HEAD_INITIALIZER(devices);

static struct sma pci_device_sma;
static struct sma pci_msix_sma;
static struct sma pci_map_sma;

static struct file_op sysfs_pci_fop;
static struct file_op sysfs_pci_dev_fop;

static void
enable_cmd(struct pci_device *device, uint16_t cmd)
{
	if (device->header.command & cmd)
		return;
	device->header.command |= cmd;
	pci_dev_write(device, 0x4, device->header.v4);
}

static void
disable_cmd(struct pci_device *device, uint16_t cmd)
{
	if (!(device->header.command & cmd))
		return;
	device->header.command &= ~cmd;
	pci_dev_write(device, 0x4, device->header.v4);
}

static void
enable_bus_mastering(struct pci_device *device)
{
	enable_cmd(device, 1 << 2);
}

static void
enable_mmio(struct pci_device *device)
{
	enable_cmd(device, 1 << 1);
}

static void
enable_pio(struct pci_device *device)
{
	enable_cmd(device, 1 << 0);
}

static int
find_capability(struct pci_device *device, uint8_t id, uint8_t *ptrp)
{
	uint8_t ptr;

	if (!(device->header.status & (1 << 4)))
		return -EXDEV;
	ptr = device->header0.capabilities & ~0x3;
	while (ptr)
	{
		uint32_t value = pci_dev_read(device, ptr);
		if ((value & 0xFF) == id)
		{
			if (ptrp)
				*ptrp = ptr;
			return 0;
		}
		ptr = (value >> 8) & 0xFF;
	}
	return -EXDEV;
}

static int
enable_msi(struct pci_device *device, uint64_t addr, uint32_t data)
{
	union pci_cap_msi cap;
	uint8_t ptr;
	int ret;

	ret = find_capability(device, PCI_CAP_MSI, &ptr);
	if (ret)
		return ret;
	cap.v0 = pci_dev_read(device, ptr + 0x0);
	cap.v4 = pci_dev_read(device, ptr + 0x4);
	cap.v8 = pci_dev_read(device, ptr + 0x8);
	cap.vC = pci_dev_read(device, ptr + 0xC);
	enable_cmd(device, 1 << 10);
	pci_dev_write(device, ptr + 0x0, cap.v0 | (1 << 16));
	pci_dev_write(device, ptr + 0x4, addr);
	pci_dev_write(device, ptr + 0x8, addr >> 32);
	pci_dev_write(device, ptr + 0xC, data);
	return 0;
}

static void
disable_msi(struct pci_device *device)
{
	disable_cmd(device, 1 << 10);
}

static int
enable_msix(struct pci_device *device,
            uint64_t addr,
            uint32_t data,
            uint16_t *vector)
{
	uint32_t id = UINT32_MAX;

	if (!device->msix)
		return -ENOENT;
	for (uint16_t i = 0; i < device->msix->size; ++i)
	{
		if (pci_r32(device->msix->map, i * 16 + 12) & 1)
		{
			id = i;
			break;
		}
	}
	if (id == UINT32_MAX)
	{
		TRACE("pci: no more msix vector available");
		return -EINVAL;
	}
	pci_w32(device->msix->map, id * 16 + 0x0, addr);
	pci_w32(device->msix->map, id * 16 + 0x4, addr >> 32);
	pci_w32(device->msix->map, id * 16 + 0x8, data);
	pci_w32(device->msix->map, id * 16 + 0xC, 0);
	*vector = id;
	return 0;
}

static void
disable_msix(struct pci_device *device, uint16_t vector)
{
	assert(device->msix, "disabling unexisting msix\n");
	assert(vector < device->msix->size, "invalid msix vector\n");
	assert(!(pci_r32(device->msix->map, vector * 16 + 12) & 1), "disabling masked msix vector\n");
	pci_w32(device->msix->map, vector * 16 + 12, 1);
}

static int
set_powerstate(struct pci_device *device, uint8_t state)
{
	static const struct timespec delay = {0, 10000000};
	union pci_cap_pm cap;
	uint8_t ptr;
	int ret;

	if (state > 3)
		return -EINVAL;
	ret = find_capability(device, PCI_CAP_PM, &ptr);
	if (ret)
		return 0;
	cap.v0 = pci_dev_read(device, ptr + 0x0);
	cap.v4 = pci_dev_read(device, ptr + 0x4);
	if ((cap.status_control & 0x3) == state)
		return 0;
	cap.status_control = (cap.status_control & ~0x3) | state;
	pci_dev_write(device, ptr + 0x4, cap.v4);
	/* wait 10ms for state transition (worst case scenario) */
	spinsleep(&delay);
	return 0;
}

static int
setup_msix(struct pci_device *device)
{
	union pci_cap_msix cap;
	uint8_t bir;
	uint8_t ptr;
	int ret;

	ret = find_capability(device, PCI_CAP_MSIX, &ptr);
	if (ret)
		return ret;
	device->msix = sma_alloc(&pci_msix_sma, M_ZERO);
	if (!device->msix)
	{
		TRACE("pci: msix allocation failed");
		return -ENOMEM;
	}
	cap.v0 = pci_dev_read(device, ptr + 0x0);
	cap.v4 = pci_dev_read(device, ptr + 0x4);
	cap.v8 = pci_dev_read(device, ptr + 0x8);
	bir = cap.table_offset & 0x7;
	if (bir > 5)
	{
		TRACE("pci: invalid bir: %" PRIu8, bir);
		return -EINVAL;
	}
	device->msix->size = (cap.control & 0x7FF) + 1;
	ret = pci_map((&device->header0.bar0)[bir], device->msix->size * 16,
	              cap.table_offset & ~0x7, &device->msix->map);
	if (ret)
	{
		TRACE("pci: failed to map msix");
		sma_free(&pci_msix_sma, device->msix);
		device->msix = NULL;
		return ret;
	}
	/* mask all the interrupts */
	for (size_t i = 0; i < device->msix->size; ++i)
		pci_w32(device->msix->map, i * 16 + 12, 1);
	device->header.command |= (1 << 10);
	pci_dev_write(device, 0x4, device->header.v4);
	pci_dev_write(device, ptr + 0x0, cap.v0 | (1 << 31));
	return 0;
}

static int
checkdev(uint8_t bus, uint8_t slot, uint8_t func)
{
	struct pci_device *device;
	uintptr_t ecam_poff;
	int ret;

	device = sma_alloc(&pci_device_sma, M_ZERO);
	if (!device)
	{
		TRACE("pci: device allocation failed");
		return -ENOMEM;
	}
	device->bus = bus;
	device->slot = slot;
	device->func = func;
	device->base = 0x80000000 | (bus << 16) | (slot << 11) | (func << 8);
#if WITH_ACPI
	ret = acpi_get_ecam_addr(bus, slot, func, &ecam_poff);
#elif WITH_FDT
	ret = fdt_get_ecam_addr(bus, slot, func, &ecam_poff);
#else
# error "no ecam"
#endif
#if !defined(__i386__) && !defined(__amd64__)
	if (ret)
	{
		sma_free(&pci_device_sma, device);
		return ret;
	}
#endif
	if (!ret)
	{
		pm_init_page(&device->ecam_page, ecam_poff);
		device->ecam = vm_map(&device->ecam_page, PAGE_SIZE,
		                      VM_PROT_RW | VM_MMIO);
		if (!device->ecam)
		{
			TRACE("pci: failed to vmap ecam");
			sma_free(&pci_device_sma, device);
			return -ENOMEM;
		}
	}
	device->header.v0 = pci_dev_read(device, 0x0);
	if (device->header.vendor == 0xFFFF)
	{
		sma_free(&pci_device_sma, device);
		return -EINVAL;
	}
	device->header.v4 = pci_dev_read(device, 0x4);
	device->header.v8 = pci_dev_read(device, 0x8);
	device->header.vC = pci_dev_read(device, 0xC);
	device->group = 0;
	device->bus = bus;
	device->slot = slot;
	device->func = func;
	switch (device->header.headertype & 0x7F)
	{
		case 0:
		{
			struct pci_header0 *header0 = &device->header0;
			header0->v0 = pci_dev_read(device, 0x10);
			header0->v1 = pci_dev_read(device, 0x14);
			header0->v2 = pci_dev_read(device, 0x18);
			header0->v3 = pci_dev_read(device, 0x1C);
			header0->v4 = pci_dev_read(device, 0x20);
			header0->v5 = pci_dev_read(device, 0x24);
			header0->v6 = pci_dev_read(device, 0x28);
			header0->v7 = pci_dev_read(device, 0x2C);
			header0->v8 = pci_dev_read(device, 0x30);
			header0->v9 = pci_dev_read(device, 0x34);
			header0->vA = pci_dev_read(device, 0x38);
			header0->vB = pci_dev_read(device, 0x3C);
			break;
		}
		case 1:
		{
			struct pci_header1 *header1 = &device->header1;
			header1->v0 = pci_dev_read(device, 0x10);
			header1->v1 = pci_dev_read(device, 0x14);
			header1->v2 = pci_dev_read(device, 0x18);
			header1->v3 = pci_dev_read(device, 0x1C);
			header1->v4 = pci_dev_read(device, 0x20);
			header1->v5 = pci_dev_read(device, 0x24);
			header1->v6 = pci_dev_read(device, 0x28);
			header1->v7 = pci_dev_read(device, 0x2C);
			header1->v8 = pci_dev_read(device, 0x30);
			header1->v9 = pci_dev_read(device, 0x34);
			header1->vA = pci_dev_read(device, 0x38);
			header1->vB = pci_dev_read(device, 0x3C);
			header1->vC = pci_dev_read(device, 0x40);
			header1->vD = pci_dev_read(device, 0x44);
			header1->vE = pci_dev_read(device, 0x48);
			header1->vF = pci_dev_read(device, 0x4C);
			break;
		}
	}
	enable_mmio(device);
	enable_pio(device);
	set_powerstate(device, 0); /* XXX only for successfully probed devices */
	enable_bus_mastering(device);
	char node_name[64];
	snprintf(node_name, sizeof(node_name), "pci/%04x:%02x:%02x.%1x",
	         device->group, device->bus, device->slot, device->func);
	ret = sysfs_mknode(node_name, 0, 0, 0400, &sysfs_pci_dev_fop,
	                   &device->sysfs_node);
	if (ret)
		TRACE("pci: failed to create sysfs node: %s", strerror(ret));
	else
		device->sysfs_node->userdata = device;
	setup_msix(device);
	spinlock_lock(&devices_lock);
	TAILQ_INSERT_TAIL(&devices, device, chain);
	spinlock_unlock(&devices_lock);
	return 0;
}

void
pci_probe(uint16_t vendor, uint16_t device, pci_probe_t probe, void *userdata)
{
	struct pci_device *dev;

	spinlock_lock(&devices_lock);
	TAILQ_FOREACH(dev, &devices, chain)
	{
		if (dev->header.vendor == vendor
		 && dev->header.device == device)
		{
			if (probe(dev, userdata))
				break;
		}
	}
	spinlock_unlock(&devices_lock);
}

void
pci_probe_progif(uint8_t class,
                 uint8_t subclass,
                 uint8_t progif,
                 pci_probe_t probe,
                 void *userdata)
{
	struct pci_device *dev;

	spinlock_lock(&devices_lock);
	TAILQ_FOREACH(dev, &devices, chain)
	{
		if (dev->header.class == class
		 && dev->header.subclass == subclass
		 && dev->header.progif == progif)
		{
			if (probe(dev, userdata))
				break;
		}
	}
	spinlock_unlock(&devices_lock);
}

static void
scan(void)
{
	for (uint32_t bus = 0; bus < 256; ++bus)
	{
		for (uint32_t slot = 0; slot < 32; ++slot)
		{
			for (uint32_t func = 0; func < 8; ++func)
			{
				checkdev(bus, slot, func);
			}
		}
	}
	if (sysfs_mknode("pci/list", 0, 0, 0444, &sysfs_pci_fop, NULL))
		TRACE("failed to create /sys/pci/list");
}

int
pci_register_irq(struct pci_device *device,
                 irq_fn_t fn,
                 void *userdata,
                 struct irq_handle *handle)
{
#if defined(__i386__) || defined(__x86_64__)
	if (!g_has_apic)
		return register_pic_irq(handle, 32 + device->header0.int_line, 0, fn, userdata);
#endif
	size_t cpuid;
	size_t irq;
	int ret = arch_get_msi_irq(&cpuid, &irq);
	if (ret)
		return ret;
	uint64_t msi_addr;
	uint32_t msi_data;
	if (!arch_get_msi_addr(cpuid, &msi_addr)
	 && !arch_get_msi_data(irq, &msi_data))
	{
		uint16_t msix_vector;
		if (!enable_msix(device, msi_addr, msi_data, &msix_vector))
		{
			register_irq(handle, IRQ_MSIX, irq, cpuid, fn, userdata);
			handle->msix.vector = msix_vector;
			arch_register_msi_irq(handle);
			return 0;
		}
		if (!enable_msi(device, msi_addr, msi_data))
		{
			register_irq(handle, IRQ_MSI, irq, cpuid, fn, userdata);
			arch_register_msi_irq(handle);
			return 0;
		}
	}
#if defined(__i386__) || defined(__x86_64__)
	if (!(device->header.headertype & 0x7F))
	{
		uint8_t ioapic;
		uint8_t line;
		int active_low;
		int level_trigger;
		ret = mptable_get_pci_irq(device->bus, device->slot, device->header0.int_pin,
		                          &ioapic, &line, &active_low, &level_trigger);
		if (!ret)
			return register_apic_irq(handle, 32 + line, ioapic, active_low, level_trigger, 0, fn, userdata);
	}
#endif
	return -EINVAL;
}

void
pci_unregister_irq(struct pci_device *device, struct irq_handle *handle)
{
	switch (handle->type)
	{
		case IRQ_NATIVE:
			arch_disable_native_irq(handle);
			break;
		case IRQ_MSI:
			disable_msi(device);
			break;
		case IRQ_MSIX:
			disable_msix(device, handle->msix.vector);
			break;
		default:
			break;
	}
	unregister_irq(handle);
}

int
pci_map(size_t addr, size_t size, size_t offset, struct pci_map **mapp)
{
	struct pci_map *map = NULL;
	int ret;

	map = sma_alloc(&pci_map_sma, M_ZERO);
	if (!map)
		return -ENOMEM;
	map->addr = addr;
	map->size = size;
	map->offset = offset;
	if (addr & 1)
	{
#if !defined(__i386__) && !defined(__amd64__)
		map->data = NULL;
		TRACE("pci: PIO not supported");
		ret = -EINVAL;
		goto err;
#endif
		map->base = (map->addr & ~3) + map->offset;
		*mapp = map;
		return 0;
	}
	uint32_t flags = VM_PROT_RW;
	if (!(addr & 0x8))
		flags |= VM_MMIO;
	size_t base = (addr & ~0xF) + offset;
	size_t pad = base & PAGE_MASK;
	size_t map_size = (size + pad + PAGE_MASK) & ~PAGE_MASK;
	pm_init_page(&map->page, base / PAGE_SIZE);
	map->data = vm_map(&map->page, map_size, flags);
	if (!map->data)
	{
		ret = -ENOMEM;
		goto err;
	}
	map->data += pad;
	*mapp = map;
	return 0;

err:
	sma_free(&pci_map_sma, map);
	return ret;
}

int
pci_map_bar(struct pci_device *device,
            size_t bar,
            size_t size,
            size_t offset,
            struct pci_map **mapp)
{
	switch (device->header.headertype & 0x7F)
	{
		case 0:
		{
			if (bar > 5)
			{
				TRACE("invalid bar id: %zu", bar);
				return -EINVAL;
			}
			size_t addr = (&device->header0.bar0)[bar];
			if (addr & 0x4)
			{
				if (bar > 4)
				{
					TRACE("invalid 64bit bar id: %zu", bar);
					return -EINVAL;
				}
				uint32_t next_bar = (&device->header0.bar0)[bar + 1];
#if __SIZE_WIDTH__ == 32
				if (next_bar)
				{
					TRACE("64bit on 32bit system");
					return -EINVAL;
				}
#else
				addr |= (uint64_t)next_bar << 32;
#endif
			}
			return pci_map(addr, size, offset, mapp);
		}
		case 1:
			if (bar > 1)
			{
				TRACE("invalid bar id: %zu", bar);
				return -EINVAL;
			}
			size_t addr = (&device->header1.bar0)[bar];
			if (addr & 0x4)
			{
#if __SIZE_WIDTH__ == 32
				TRACE("64bit on 32bit system");
				return -EINVAL;
#endif
				if (bar > 0)
				{
					TRACE("invalid 64bit bar id: %zu", bar);
					return -EINVAL;
				}
				uint32_t next_bar = (&device->header1.bar0)[bar + 1];
#if __SIZE_WIDTH__ == 32
				if (next_bar)
				{
					TRACE("64bit on 32bit system");
					return -EINVAL;
				}
#else
				addr |= (uint64_t)next_bar << 32;
#endif
			}
			return pci_map(addr, size, offset, mapp);
		default:
			TRACE("unknown pci header type %x",
			      (unsigned)(device->header.headertype & 0x7F));
			return -EINVAL;
	}
}

void
pci_unmap(struct pci_device *device, struct pci_map *map)
{
	if (!map)
		return;
	(void)device;
	if (!(map->addr & 1) && !map->data)
	{
		uint32_t base = (map->addr & ~0xF) + map->offset;
		uint32_t pad = base & PAGE_MASK;
		vm_unmap((void*)((uintptr_t)map->data & ~PAGE_MASK),
		         (map->size + pad + PAGE_MASK) & ~PAGE_MASK);
	}
	free(map);
}

#if defined(__i386__) || defined(__x86_64__)

uint32_t
pci_read(uint32_t addr)
{
	outl(PCI_ADDR, addr);
	return inl(PCI_DATA);
}

void
pci_write(uint32_t addr, uint32_t data)
{
	outl(PCI_ADDR, addr);
	outl(PCI_DATA, data);
}

#endif

uint8_t
pci_r8(struct pci_map *map, uint32_t off)
{
#if defined(__i386__) || defined(__amd64__)
	if (map->addr & 1)
		return inb(map->base + off);
#endif
	return *(uint8_t volatile*)&map->data[off];
}

uint16_t
pci_r16(struct pci_map *map, uint32_t off)
{
#if defined(__i386__) || defined(__amd64__)
	if (map->addr & 1)
		return inw(map->base + off);
#endif
	return *(uint16_t volatile*)&map->data[off];
}

uint32_t
pci_r32(struct pci_map *map, uint32_t off)
{
#if defined(__i386__) || defined(__amd64__)
	if (map->addr & 1)
		return inl(map->base + off);
#endif
	return *(uint32_t volatile*)&map->data[off];
}

uint64_t
pci_r64(struct pci_map *map, uint64_t off)
{
#if defined(__i386__) || defined(__amd64__)
	if (map->addr & 1)
		panic("64bit io port\n");
#endif
	return *(uint64_t volatile*)&map->data[off];
}

void
pci_r8v(struct pci_map *map, uint32_t off, uint8_t *data, size_t count)
{
#if defined(__i386__) || defined(__amd64__)
	if (map->addr & 1)
		return insb(map->base + off, data, count);
#endif
	for (size_t i = 0; i < count; ++i)
		data[i] = *(uint8_t volatile*)&map->data[off + i];
}

void
pci_r16v(struct pci_map *map, uint32_t off, uint16_t *data, size_t count)
{
#if defined(__i386__) || defined(__amd64__)
	if (map->addr & 1)
		return insw(map->base + off, data, count);
#endif
	for (size_t i = 0; i < count; ++i)
		data[i] = *(uint16_t volatile*)&map->data[off + i];
}

void
pci_r32v(struct pci_map *map, uint32_t off, uint32_t *data, size_t count)
{
#if defined(__i386__) || defined(__amd64__)
	if (map->addr & 1)
		return insl(map->base + off, data, count);
#endif
	for (size_t i = 0; i < count; ++i)
		data[i] = *(uint32_t volatile*)&map->data[off + i];
}

void
pci_r64v(struct pci_map *map, uint32_t off, uint64_t *data, size_t count)
{
#if defined(__i386__) || defined(__amd64__)
	if (map->addr & 1)
		panic("64bit io port\n");
#endif
	for (size_t i = 0; i < count; ++i)
		data[i] = *(uint64_t volatile*)&map->data[off + i];
}

void
pci_w8(struct pci_map *map, uint32_t off, uint8_t v)
{
#if defined(__i386__) || defined(__amd64__)
	if (map->addr & 1)
	{
		outb(map->base + off, v);
		return;
	}
#endif
	*(uint8_t volatile*)&map->data[off] = v;
}

void
pci_w16(struct pci_map *map, uint32_t off, uint16_t v)
{
#if defined(__i386__) || defined(__amd64__)
	if (map->addr & 1)
	{
		outw(map->base + off, v);
		return;
	}
#endif
	*(uint16_t volatile*)&map->data[off] = v;
}

void
pci_w32(struct pci_map *map, uint32_t off, uint32_t v)
{
#if defined(__i386__) || defined(__amd64__)
	if (map->addr & 1)
	{
		outl(map->base + off, v);
		return;
	}
#endif
	*(uint32_t volatile*)&map->data[off] = v;
}

void
pci_w64(struct pci_map *map, uint32_t off, uint64_t v)
{
#if defined(__i386__) || defined(__amd64__)
	if (map->addr & 1)
		panic("64bit io port\n");
#endif
	*(uint64_t volatile*)&map->data[off] = v;
}

void
pci_w8v(struct pci_map *map, uint32_t off, const uint8_t *data, size_t count)
{
#if defined(__i386__) || defined(__amd64__)
	if (map->addr & 1)
	{
		outsb(map->base + off, data, count);
		return;
	}
#endif
	for (size_t i = 0; i < count; ++i)
		*(uint8_t volatile*)&map->data[off + i] = data[i];
}

void
pci_w16v(struct pci_map *map, uint32_t off, const uint16_t *data, size_t count)
{
#if defined(__i386__) || defined(__amd64__)
	if (map->addr & 1)
	{
		outsw(map->base + off, data, count);
		return;
	}
#endif
	for (size_t i = 0; i < count; ++i)
		*(uint16_t volatile*)&map->data[off + i] = data[i];
}

void
pci_w32v(struct pci_map *map, uint32_t off, const uint32_t *data, size_t count)
{
#if defined(__i386__) || defined(__amd64__)
	if (map->addr & 1)
	{
		outsl(map->base + off, data, count);
		return;
	}
#endif
	for (size_t i = 0; i < count; ++i)
		*(uint32_t volatile*)&map->data[off + i] = data[i];
}

void
pci_w64v(struct pci_map *map, uint32_t off, const uint64_t *data, size_t count)
{
#if defined(__i386__) || defined(__amd64__)
	if (map->addr & 1)
		panic("64bit io port\n");
#endif
	for (size_t i = 0; i < count; ++i)
		*(uint64_t volatile*)&map->data[off + i] = data[i];
}

uint32_t
pci_dev_read(struct pci_device *device, uint32_t off)
{
	if (device->ecam)
		return *(uint32_t volatile*)&device->ecam[off];
#if defined(__i386__) || defined(__x86_64__)
	return pci_read(device->base + off);
#else
	panic("unhandled legacy PCI\n");
#endif
}

void
pci_dev_write(struct pci_device *device, uint32_t off, uint32_t val)
{
	if (device->ecam)
	{
		*(uint32_t volatile*)&device->ecam[off] = val;
		return;
	}
#if defined(__i386__) || defined(__x86_64__)
	else
	{
		pci_write(device->base + off, val);
	}
#else
	panic("unhandled legacy PCI\n");
#endif
}

static void
print_capabilities(struct uio *uio, struct pci_device *device, uint8_t ptr)
{
	if (!(device->header.status & (1 << 4)))
		return;
	uprintf(uio, "capabilities:");
	while (ptr)
	{
		uint32_t value = pci_dev_read(device, ptr);
		uprintf(uio, " %02" PRIx32, value & 0xFF);
		ptr = (value >> 8) & 0xFF;
	}
	uprintf(uio, "\n");
}

static int
pci_dev_print(struct uio *uio, struct pci_device *device)
{
	union pci_header *header = &device->header;

	uprintf(uio, "address   : %02" PRIx8 ":%02" PRIx8 ".%01" PRIx8 "\n"
	             "vendor    : 0x%04" PRIx16 "\n"
	             "device    : 0x%04" PRIx16 "\n"
	             "command   : 0x%04" PRIx16 "\n"
	             "status    : 0x%04" PRIx16 "\n"
	             "revision  : 0x%02" PRIx8 "\n"
	             "progif    : 0x%02" PRIx8 "\n"
	             "subclass  : 0x%02" PRIx8 "\n"
	             "class     : 0x%02" PRIx8 "\n"
	             "cacheline : 0x%02" PRIx8 "\n"
	             "latency   : 0x%02" PRIx8 "\n"
	             "headertype: 0x%02" PRIx8 "\n"
	             "bist      : 0x%02" PRIx8 "\n",
	             device->bus,
	             device->slot,
	             device->func,
	             header->vendor,
	             header->device,
	             header->command,
	             header->status,
	             header->revision,
	             header->progif,
	             header->subclass,
	             header->class,
	             header->cacheline,
	             header->latency,
	             header->headertype,
	             header->bist);
	switch (header->headertype & 0x7F)
	{
		case 0:
		{
			struct pci_header0 *header0 = &device->header0;
			uprintf(uio, "bar0      : 0x%08" PRIx32 "\n"
			             "bar1      : 0x%08" PRIx32 "\n"
			             "bar2      : 0x%08" PRIx32 "\n"
			             "bar3      : 0x%08" PRIx32 "\n"
			             "bar4      : 0x%08" PRIx32 "\n"
			             "bar5      : 0x%08" PRIx32 "\n"
			             "cardbus   : 0x%08" PRIx32 "\n"
			             "subsystem : 0x%04" PRIx16 "\n"
			             "subvendor : 0x%04" PRIx16 "\n"
			             "ROM bar   : 0x%08" PRIx32 "\n"
			             "cap ptr   : 0x%02" PRIx8 "\n"
			             "max lat   : 0x%02" PRIx8 "\n"
			             "min grant : 0x%02" PRIx8 "\n"
			             "int pin   : 0x%02" PRIx8 "\n"
			             "int line  : 0x%02" PRIx8 "\n",
			             header0->bar0,
			             header0->bar1,
			             header0->bar2,
			             header0->bar3,
			             header0->bar4,
			             header0->bar5,
			             header0->cis_pointer,
			             header0->subsystem_id,
			             header0->subsystem_vendor_id,
			             header0->rom_bar,
			             header0->capabilities,
			             header0->max_latency,
			             header0->min_grant,
			             header0->int_pin,
			             header0->int_line);
			print_capabilities(uio, device, header0->capabilities & ~3);
			break;
		}
		case 1:
		{
			struct pci_header1 *header1 = &device->header1;
			uprintf(uio, "bar0      : 0x%08" PRIx32 "\n"
			             "bar1      : 0x%08" PRIx32 "\n"
			             "pri bus   : 0x%02" PRIx8 "\n"
			             "sec bus   : 0x%02" PRIx8 "\n"
			             "sub bus   : 0x%02" PRIx8 "\n"
			             "sec lat   : 0x%02" PRIx8 "\n"
			             "io base   : 0x%02" PRIx8 "\n"
			             "io limit  : 0x%02" PRIx8 "\n"
			             "sec status: 0x%04" PRIx16 "\n"
			             "mem base  : 0x%04" PRIx16 "\n"
			             "mem limit : 0x%04" PRIx16 "\n"
			             "pre basel : 0x%04" PRIx16 "\n"
			             "pre limitl: 0x%04" PRIx16 "\n"
			             "pre baseh : 0x%08" PRIx32 "\n"
			             "pre limith: 0x%08" PRIx32 "\n"
			             "io baseh  : 0x%04" PRIx16 "\n"
			             "io limith : 0x%04" PRIx16 "\n"
			             "cap ptr   : 0x%02" PRIx8 "\n"
			             "exp rom   : 0x%08" PRIx32 "\n"
			             "int line  : 0x%02" PRIx8 "\n"
			             "int pin   : 0x%02" PRIx8 "\n"
			             "bridge ctl: 0x%04" PRIx16 "\n",
			             header1->bar0,
			             header1->bar1,
			             header1->primary_bus,
			             header1->secondary_bus,
			             header1->subordinate_bus,
			             header1->secondary_latency,
			             header1->io_base,
			             header1->io_limit,
			             header1->secondary_status,
			             header1->memory_base,
			             header1->memory_limit,
			             header1->prefetchable_base,
			             header1->prefetchable_limit,
			             header1->prefetchable_base_upper,
			             header1->prefetchable_limit_upper,
			             header1->io_base_upper,
			             header1->io_limit_upper,
			             header1->capability_pointer,
			             header1->expansion_rom_addr,
			             header1->int_line,
			             header1->int_pin,
			             header1->bridge_control);
			print_capabilities(uio, device, header1->capability_pointer & ~3);
			break;
		}
	}
	return 0;
}

static int
pci_print(struct uio *uio)
{
	struct pci_device *device;

	spinlock_lock(&devices_lock);
	TAILQ_FOREACH(device, &devices, chain)
	{
		uprintf(uio, "%04" PRIx16 ":%02" PRIx8 ":%02" PRIx8 ".%01" PRIx8 " %04" PRIx16 ":%04" PRIx16 "\n",
		        device->group, device->bus, device->slot, device->func,
		        device->header.vendor, device->header.device);
	}
	spinlock_unlock(&devices_lock);
	return 0;
}

static ssize_t
sysfs_pci_read(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	int ret = pci_print(uio);
	if (ret < 0)
		return ret;
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static struct file_op
sysfs_pci_fop =
{
	.read = sysfs_pci_read,
};

static int
sysfs_pci_dev_open(struct file *file, struct node *node)
{
	file->userdata = node->userdata;
	return 0;
}

static ssize_t
sysfs_pci_dev_read(struct file *file, struct uio *uio)
{
	size_t count = uio->count;
	off_t off = uio->off;
	int ret = pci_dev_print(uio, file->userdata);
	if (ret < 0)
		return ret;
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static struct file_op
sysfs_pci_dev_fop =
{
	.open = sysfs_pci_dev_open,
	.read = sysfs_pci_dev_read,
};

int
init(void)
{
	sma_init(&pci_device_sma, sizeof(struct pci_device), NULL, NULL, "pci_device");
	sma_init(&pci_msix_sma, sizeof(struct pci_msix), NULL, NULL, "pci_msix");
	sma_init(&pci_map_sma, sizeof(struct pci_map), NULL, NULL, "pci_map");
	scan();
	return 0;
}

void
fini(void)
{
}

struct kmod_info
kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "pci",
	.init = init,
	.fini = fini,
};
