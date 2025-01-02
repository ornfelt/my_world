#include "arch/x86/x86.h"

#include <file.h>
#include <std.h>
#include <uio.h>
#include <vfs.h>
#include <mem.h>

struct mp_fp_st
{
	uint8_t signature[4];
	uint32_t physical_address;
	uint8_t length;
	uint8_t spec_rev;
	uint8_t checksum;
	uint8_t feature1;
	uint8_t feature2;
	uint8_t feature3;
	uint8_t feature4;
	uint8_t feature5;
} __attribute__ ((packed));

struct mp_cfg_st
{
	uint8_t signature[4];
	uint16_t length;
	uint8_t spec_rev;
	uint8_t checksum;
	uint64_t oem_id;
	uint8_t product_id[12];
	uint32_t oem_table;
	uint16_t oem_table_size;
	uint16_t entry_count;
	uint32_t lapic_address;
	uint16_t extended_table_length;
	uint8_t extended_table_checksum;
	uint8_t reserved;
} __attribute__ ((packed));

enum mp_cfg_entry_type
{
	MP_CFG_PROCESSOR = 0,
	MP_CFG_BUS       = 1,
	MP_CFG_IOAPIC    = 2,
	MP_CFG_IO_INT    = 3,
	MP_CFG_LOCAL_INT = 4,
};

struct mp_cfg_processor
{
	uint8_t type;
	uint8_t lapic_id;
	uint8_t lapic_version;
	uint8_t cpu_flags;
	uint32_t cpu_signature;
	uint32_t feature_flags;
	uint32_t reserved[2];
} __attribute__ ((packed));

struct mp_cfg_bus
{
	uint8_t type;
	uint8_t id;
	uint8_t string[6];
} __attribute__ ((packed));

struct mp_cfg_ioapic
{
	uint8_t type;
	uint8_t id;
	uint8_t version;
	uint8_t flags;
	uint32_t address;
} __attribute__ ((packed));

#define IO_INT_PCI_BUS(io_int) (io_int->src_bus_id)
#define IO_INT_PCI_SLOT(io_int) ((io_int->src_bus_irq >> 2) & 0x1F)
#define IO_INT_PCI_FUNC(io_int) (io_int->src_bus_irq & 0x3)

struct mp_cfg_io_int
{
	uint8_t type;
	uint8_t int_type;
	uint16_t flags;
	uint8_t src_bus_id;
	uint8_t src_bus_irq;
	uint8_t dst_ioapic_id;
	uint8_t dst_ioapic_int;
} __attribute__ ((packed));

struct mp_cfg_local_int
{
	uint8_t type;
	uint8_t int_type;
	uint16_t flags;
	uint8_t src_bus_id;
	uint8_t src_bus_irq;
	uint8_t dst_lapic_id;
	uint8_t dst_lapic_int;
} __attribute__ ((packed));

static const struct mp_fp_st *mp_fp;
static const struct mp_cfg_st *mp_cfg;
static struct page mp_cfg_page;
const uint8_t *mp_cfg_addr;

static ssize_t mpinfo_read(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;

	uprintf(uio, "MP floating pointer table:\n");
	uprintf(uio, "physical addr: 0x%" PRIx32 "\n", mp_fp->physical_address);
	uprintf(uio, "length: %" PRIu8 "\n", mp_fp->length);
	uprintf(uio, "spec rev: %" PRIu8 "\n", mp_fp->spec_rev);
	uprintf(uio, "features: %02" PRIx8 ":%02" PRIx8 "\n", mp_fp->feature1, mp_fp->feature2);
	uprintf(uio, "\n");
	uprintf(uio, "MP configuration table:\n");
	uprintf(uio, "spec rev: %" PRIu8 "\n", mp_cfg->spec_rev);
	uprintf(uio, "oem id: %" PRIu64 "\n", mp_cfg->oem_id);
	uprintf(uio, "product id: %.12s\n", mp_cfg->product_id);
	uprintf(uio, "oem table: %" PRIu32 "\n", mp_cfg->oem_table);
	uprintf(uio, "oem table size: %" PRIu16 "\n", mp_cfg->oem_table_size);
	uprintf(uio, "entry count: %" PRIu16 "\n", mp_cfg->entry_count);
	uprintf(uio, "lapic address: 0x%" PRIx32 "\n", mp_cfg->lapic_address);
	uprintf(uio, "ext table length: %" PRIu16 "\n", mp_cfg->extended_table_length);

#if 1
#define SHORT_OUTPUT
#endif

	const uint8_t *entry = (const uint8_t*)&mp_cfg[1];
	for (uint32_t i = 0; i < mp_cfg->entry_count; ++i)
	{
		switch (*entry)
		{
			case MP_CFG_PROCESSOR:
			{
				struct mp_cfg_processor *processor = (struct mp_cfg_processor*)entry;
#ifdef SHORT_OUTPUT
				uprintf(uio, "processor: 0x%02" PRIx8 ", 0x%02" PRIu8 ", 0x%02" PRIx8 ", 0x%08" PRIx32 ", 0x%08" PRIx32 "\n",
				        processor->lapic_id,
				        processor->lapic_version,
				        processor->cpu_flags,
				        processor->cpu_signature,
				        processor->feature_flags);
#else
				uprintf(uio, "\nprocessor:\n");
				uprintf(uio, "lapic id: %" PRIu8 "\n", processor->lapic_id);
				uprintf(uio, "lapic version: %" PRIu8 "\n", processor->lapic_version);
				uprintf(uio, "cpu flags: 0x%" PRIx8 "\n", processor->cpu_flags);
				uprintf(uio, "cpu signature: 0x%" PRIx32 "\n", processor->cpu_signature);
				uprintf(uio, "feature flags: 0x%" PRIx32 "\n", processor->feature_flags);
#endif
				entry += sizeof(*processor);
				break;
			}
			case MP_CFG_BUS:
			{
				struct mp_cfg_bus *bus = (struct mp_cfg_bus*)entry;
#ifdef SHORT_OUTPUT
				uprintf(uio, "bus: 0x%02" PRIx8 ", %.6s\n",
				        bus->id,
				        bus->string);
#else
				uprintf(uio, "\nbus:\n");
				uprintf(uio, "id: %" PRIu8 "\n", bus->id);
				uprintf(uio, "value: %.6s\n", bus->string);
#endif
				entry += sizeof(*bus);
				break;
			}
			case MP_CFG_IOAPIC:
			{
				struct mp_cfg_ioapic *ioapic = (struct mp_cfg_ioapic*)entry;
#ifdef SHORT_OUTPUT
				uprintf(uio, "ioapic: 0x%02" PRIx8 ", 0x%02" PRIx8 ", 0x%02" PRIx8 ", 0x%08" PRIx32 "\n",
				        ioapic->id,
				        ioapic->version,
				        ioapic->flags,
				        ioapic->address);
#else
				uprintf(uio, "\nioapic:\n");
				uprintf(uio, "id: %" PRIu8 "\n", ioapic->id);
				uprintf(uio, "version: %" PRIu8 "\n", ioapic->version);
				uprintf(uio, "flags: %" PRIx8 "\n", ioapic->flags);
				uprintf(uio, "address: %" PRIx32 "\n", ioapic->address);
#endif
				entry += sizeof(*ioapic);
				break;
			}
			case MP_CFG_IO_INT:
			{
				struct mp_cfg_io_int *io_int = (struct mp_cfg_io_int*)entry;
#ifdef SHORT_OUTPUT
				uprintf(uio, "io int: 0x%02" PRIx8 ", 0x%04" PRIx16 ", 0x%02" PRIx8 ", 0x%02" PRIx8 ", 0x%02" PRIx8 ", 0x%02" PRIx8 ", 0x%02" PRIx8 ".0x%02" PRIx8 "\n",
				        io_int->int_type,
				        io_int->flags,
				        io_int->src_bus_id,
				        io_int->src_bus_irq,
				        io_int->dst_ioapic_id,
				        io_int->dst_ioapic_int,
				        IO_INT_PCI_SLOT(io_int),
				        IO_INT_PCI_FUNC(io_int));
#else
				uprintf(uio, "\nio int:\n");
				uprintf(uio, "type: %" PRIu8 "\n", io_int->int_type);
				uprintf(uio, "flags: %" PRIx16 "\n", io_int->flags);
				uprintf(uio, "src bus id: %" PRIu8 "\n", io_int->src_bus_id);
				uprintf(uio, "src bus irq: %" PRIu8 "\n", io_int->src_bus_irq);
				uprintf(uio, "dst ioapic id: %" PRIu8 "\n", io_int->dst_ioapic_id);
				uprintf(uio, "dst ioapic int: %" PRIu8 "\n", io_int->dst_ioapic_int);
				uprintf(uio, "PCI device: %" PRIu8 ".%" PRIu8 "\n", IO_INT_PCI_SLOT(io_int), IO_INT_PCI_FUNC(io_int));
#endif
				entry += sizeof(*io_int);
				break;
			}
			case MP_CFG_LOCAL_INT:
			{
				struct mp_cfg_local_int *local_int = (struct mp_cfg_local_int*)entry;
#ifdef SHORT_OUTPUT
				uprintf(uio, "local int: 0x%" PRIx8 ", 0x%02" PRIx8 ", 0x%02" PRIx8 ", 0x%02" PRIx8 ", 0x%02" PRIx8 ", 0x%02" PRIx8 "\n",
				        local_int->int_type,
				        local_int->flags,
				        local_int->src_bus_id,
				        local_int->src_bus_irq,
				        local_int->dst_lapic_id,
				        local_int->dst_lapic_int);
#else
				uprintf(uio, "\nlocal int:\n");
				uprintf(uio, "type: %" PRIu8 "\n", local_int->int_type);
				uprintf(uio, "flags: %" PRIx8 "\n", local_int->flags);
				uprintf(uio, "src bus id: %" PRIu8 "\n", local_int->src_bus_id);
				uprintf(uio, "src bus irq: %" PRIu8 "\n", local_int->src_bus_irq);
				uprintf(uio, "dst lapic id: %" PRIu8 "\n", local_int->dst_lapic_id);
				uprintf(uio, "dst lapic int: %" PRIu8 "\n", local_int->dst_lapic_int);
#endif
				entry += sizeof(*local_int);
				break;
			}
			default:
				panic("mpinfo: unknown MP entry type: %" PRIu8 "\n", *entry);
		}
	}


	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op mpinfo_fop =
{
	.read = mpinfo_read,
};

static int get_active_low(uint8_t flags, int dfl)
{
	switch (flags)
	{
		case 0:
			return dfl;
		case 1:
			return 0;
		default:
		case 2:
			panic("mptable: reserved polarity\n");
		case 3:
			return 1;
	}
}

static int get_level_trigger(uint8_t flags, int dfl)
{
	switch (flags)
	{
		case 0:
			return dfl;
		case 1:
			return 0;
		default:
		case 2:
			panic("mptable: reserved trigger mode\n");
		case 3:
			return 1;
	}
}

int mptable_get_pci_irq(uint8_t bus, uint8_t slot, uint8_t pin, uint8_t *ioapic,
                        uint8_t *irq, int *active_low, int *level_trigger)
{
	if (!mp_cfg)
		return -ENOENT;
	if (!pin || pin > 4)
		return -ENOENT;
	pin--;
	const uint8_t *entry = (const uint8_t*)&mp_cfg[1];
	for (uint32_t i = 0; i < mp_cfg->entry_count; ++i)
	{
		switch (*entry)
		{
			case MP_CFG_PROCESSOR:
				entry += sizeof(struct mp_cfg_processor);
				break;
			case MP_CFG_BUS:
				entry += sizeof(struct mp_cfg_bus);
				break;
			case MP_CFG_IOAPIC:
				entry += sizeof(struct mp_cfg_ioapic);
				break;
			case MP_CFG_IO_INT:
			{
				struct mp_cfg_io_int *io_int = (struct mp_cfg_io_int*)entry;
				if (IO_INT_PCI_BUS(io_int) == bus
				 && IO_INT_PCI_SLOT(io_int) == slot
				 && IO_INT_PCI_FUNC(io_int) == pin)
				{
					*ioapic = io_int->dst_ioapic_id;
					*irq = io_int->dst_ioapic_int;
					if (active_low)
						*active_low = get_active_low(io_int->flags & 0x3, 0);
					if (level_trigger)
						*level_trigger = get_level_trigger((io_int->flags >> 2) & 0x3, 0);
					return 0;
				}
				entry += sizeof(struct mp_cfg_io_int);
				break;
			}
			case MP_CFG_LOCAL_INT:
				entry += sizeof(struct mp_cfg_local_int);
				break;
		}
	}
	return -ENOENT;
}

int mptable_get_isa_irq(uint8_t isa, uint8_t *ioapic, uint8_t *irq,
                        int *active_low, int *level_trigger)
{
	if (!mp_cfg)
		return -ENOENT;
	const uint8_t *entry = (const uint8_t*)&mp_cfg[1];
	int isa_bus = -1;
	for (uint32_t i = 0; i < mp_cfg->entry_count; ++i)
	{
		switch (*entry)
		{
			case MP_CFG_PROCESSOR:
				entry += sizeof(struct mp_cfg_processor);
				break;
			case MP_CFG_BUS:
			{
				struct mp_cfg_bus *bus = (struct mp_cfg_bus*)entry;
				if (!memcmp(bus->string, "ISA", 3))
					isa_bus = bus->id; /* XXX verify there only a single ISA bus */
				entry += sizeof(struct mp_cfg_bus);
				break;
			}
			case MP_CFG_IOAPIC:
				entry += sizeof(struct mp_cfg_ioapic);
				break;
			case MP_CFG_IO_INT:
			{
				struct mp_cfg_io_int *io_int = (struct mp_cfg_io_int*)entry;
				if (io_int->src_bus_id == isa_bus
				 && io_int->src_bus_irq == isa)
				{
					*ioapic = io_int->dst_ioapic_id;
					*irq = io_int->dst_ioapic_int;
					if (active_low)
						*active_low = get_active_low(io_int->flags & 0x3, 0);
					if (level_trigger)
						*level_trigger = get_level_trigger((io_int->flags >> 2) & 0x3, 0);
					return 0;
				}
				entry += sizeof(struct mp_cfg_io_int);
				break;
			}
			case MP_CFG_LOCAL_INT:
				entry += sizeof(struct mp_cfg_local_int);
				break;
		}
	}
	return -ENOENT;
}

int mptable_init(void)
{
	int ret;
	struct page ebda_page;
	struct page bios_mem_page;
	const uint8_t *ebda_addr = NULL;
	const uint8_t *bios_mem_addr = NULL;

	pm_init_page(&ebda_page, 0x80);
	ebda_addr = vm_map(&ebda_page, 0x1000, VM_PROT_R);
	if (!ebda_addr)
	{
		printf("mpinfo: failed to map EBDA\n");
		ret = -ENOMEM;
		goto end;
	}
	pm_init_page(&bios_mem_page, 0xE0);
	bios_mem_addr = vm_map(&bios_mem_page, 0x20000, VM_PROT_R);
	if (!bios_mem_addr)
	{
		printf("mpinfo: failed to map BIOS memory\n");
		ret = -ENOMEM;
		goto end;
	}
	const uint8_t *hdr = NULL;
	for (uint32_t i = 0; i < 0x1000; i += 16)
	{
		if (!memcmp(&ebda_addr[i], "_MP_", 4))
		{
			hdr = &ebda_addr[i];
			break;
		}
	}
	if (!hdr)
	{
		for (uint32_t i = 0; i < 0x20000; i += 16)
		{
			if (!memcmp(&bios_mem_addr[i], "_MP_", 4))
			{
				hdr = &bios_mem_addr[i];
				break;
			}
		}
	}
	/* XXX must search at the top os system physical memory */
	if (!hdr)
	{
		printf("mpinfo: failed to find MP table\n");
		ret = -EINVAL;
		goto end;
	}
	uint8_t checksum = 0;
	for (uint8_t i = 0; i < 16; ++i)
		checksum += hdr[i];
	if (checksum)
	{
		printf("mpinfo: invalid MP table checksum\n");
		ret = -EINVAL;
		goto end;
	}
	mp_fp = (struct mp_fp_st*)hdr;
	if (!mp_fp->physical_address)
	{
		printf("mpinfo: no MP configuration table\n");
		ret = -EINVAL;
		goto end;
	}
	pm_init_page(&mp_cfg_page, mp_fp->physical_address / 0x1000);
	mp_cfg_addr = vm_map(&mp_cfg_page, 0x2000, VM_PROT_R);
	if (!mp_cfg_addr)
	{
		printf("mpinfo: failed to map MP cfg table\n");
		ret = -ENOMEM;
		goto end;
	}
	mp_cfg = (const struct mp_cfg_st*)&mp_cfg_addr[mp_fp->physical_address % 0x1000];
	if (memcmp(mp_cfg->signature, "PCMP", 4))
	{
		printf("mpinfo: invalid MP configuration table signature\n");
		ret = -EINVAL;
		goto end;
	}
	checksum = 0;
	for (uint32_t i = 0; i < mp_cfg->length; ++i)
		checksum += ((uint8_t*)mp_cfg)[i];
	if (checksum)
	{
		printf("mpinfo: invalid MP configuration table checksum\n");
		ret = -EINVAL;
		goto end;
	}
	ret = sysfs_mknode("mpinfo", 0, 0, 0444, &mpinfo_fop, NULL);
	if (ret)
		printf("mpinfo: failed to create sysfs node\n");

end:
	if (ebda_addr)
		vm_unmap((void*)ebda_addr, 0x1000);
	if (bios_mem_addr)
		vm_unmap((void*)bios_mem_addr, 0x20000);
	if (ret && mp_cfg_addr)
		vm_unmap((void*)mp_cfg_addr, 0x2000);
	return ret;
}
