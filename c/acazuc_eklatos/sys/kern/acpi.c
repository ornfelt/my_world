#define ENABLE_TRACE

#include "acpi.h"

#if defined(__i386__) || defined(__x86_64__)
#include "arch/x86/apic.h"
#include "arch/x86/asm.h"
#endif

#if defined(__aarch64__)
#include "arch/aarch64/gicv2.h"
#include "arch/aarch64/psci.h"
#endif

#include <multiboot.h>
#include <file.h>
#include <std.h>
#include <uio.h>
#include <vfs.h>
#include <mem.h>

#define MAX_SSDT 32

#define GAS_FMT "{0x%02" PRIx8 ", 0x%02" PRIx8 ", 0x%02" PRIx8 ", 0x%02" PRIx8 ", 0x%016" PRIx64 "}"
#define GAS_VAL(gas) (gas).address_space_id, (gas).bit_width, (gas).bit_offset, (gas).access_size, (gas).address

#define PM1_TMR_STS         (1 << 0)
#define PM1_BM_STS          (1 << 4)
#define PM1_GBL_STS         (1 << 5)
#define PM1_PWRBTN_STS      (1 << 8)
#define PM1_SLPBTN_STS      (1 << 9)
#define PM1_RTC_STS         (1 << 10)
#define PM1_PCIEXP_WAKE_STS (1 << 14)
#define PM1_WAK_STS         (1 << 15)

#define PM1_TMR_EN          (1 << 0)
#define PM1_GBL_EN          (1 << 5)
#define PM1_PWRBTN_EN       (1 << 8)
#define PM1_SLPBTN_EN       (1 << 9)
#define PM1_RTC_EN          (1 << 10)
#define PM1_PCIEXP_WAKE_DIS (1 << 14)

#define PM1_SCI_EN  (1 << 0)
#define PM1_BM_RLD  (1 << 1)
#define PM1_GBL_RLS (1 << 2)
#define PM1_SLP_TYP (1 << 10)
#define PM1_SLP_EN  (1 << 13)

#define FADT_ARM_PSCI (1 << 0)
#define FADT_ARM_HVC  (1 << 1)

/*
 * Advanced Configuration and Power
 * Interface (ACPI) Specification
 * Version 6.3
 */

struct rsdp
{
	char signature[8];
	uint8_t checksum;
	char oem_id[6];
	uint8_t revision;
	uint32_t rsdt;
} __attribute__ ((packed));

struct rsdp2
{
	struct rsdp rsdp;
	uint32_t length;
	uint64_t xsdt;
	uint8_t ext_checksum;
	uint8_t reserved[3];
} __attribute__ ((packed));

struct acpi_hdr
{
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[6];
	char oem_tableid[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} __attribute__ ((packed));

struct rsdt
{
	struct acpi_hdr hdr;
	uint32_t data[];
} __attribute__ ((packed));

struct xsdt
{
	struct acpi_hdr hdr;
	uint64_t data[];
} __attribute__ ((packed));

struct madt
{
	struct acpi_hdr hdr;
	uint32_t apic_addr;
	uint32_t flags;
} __attribute__ ((packed));

struct madt_entry
{
	uint8_t type;
	uint8_t length;
} __attribute__ ((packed));

enum madt_type
{
	MADT_LOCAL_APIC               = 0x0,
	MADT_IO_APIC                  = 0x1,
	MADT_INT_SRC_OVERRIDE         = 0x2,
	MADT_NMI_SRC                  = 0x3,
	MADT_LOCAL_APIC_NMI           = 0x4,
	MADT_LOCAL_ACIC_ADDR_OVERRIDE = 0x5,
	MADT_IO_SAPIC                 = 0x6,
	MADT_LOCAL_SAPIC              = 0x7,
	MADT_PLATFORM_INT_SOURCE      = 0x8,
	MADT_LOCAL_X2APIC             = 0x9,
	MADT_LOCAL_X2APIC_NMI         = 0xA,
	MADT_GICC                     = 0xB,
	MADT_GICD                     = 0xC,
	MADT_GIC_MSI_FRAME            = 0xD,
	MADT_GICCR                    = 0xE,
	MADT_ITS                      = 0xF,
	MADT_RESERVED                 = 0x10, /* 0x10 - 0x7F */
	MADT_OEM                      = 0x80, /* 0x80 - 0xFF */
};

struct madt_local_apic
{
	struct madt_entry entry;
	uint8_t acpi_cpuid;
	uint8_t apic_id;
	uint32_t flags;
} __attribute__ ((packed));

enum madt_local_apic_flags
{
	MADT_LOCAL_APIC_F_ENABLED = 0x1,
	MADT_LOCAL_APIC_F_CAPABLE = 0x2,
};

struct madt_io_apic
{
	struct madt_entry entry;
	uint8_t apic_id;
	uint8_t reserved;
	uint32_t apic_addr;
	uint32_t gsib;
} __attribute__ ((packed));

struct madt_int_src_override
{
	struct madt_entry entry;
	uint8_t bus;
	uint8_t source;
	uint32_t gsi;
	uint16_t flags;
} __attribute__ ((packed));

enum madt_int_src_override_flags
{
	MADT_INT_SRC_OVERRIDE_F_POLARITY_CONFORM = 0x0,
	MADT_INT_SRC_OVERRIDE_F_POLARITY_HIGH    = 0x1,
	MADT_INT_SRC_OVERRIDE_F_POLARITY_LOW     = 0x3,
	MADT_INT_SRC_OVERRIDE_F_POLARITY_MASK    = 0x3,
	MADT_INT_SRC_OVERRIDE_F_TRIGGER_CONFORM  = 0x0,
	MADT_INT_SRC_OVERRIDE_F_TRIGGER_EDGE     = 0x4,
	MADT_INT_SRC_OVERRIDE_F_TRIGGER_LEVEL    = 0xC,
	MADT_INT_SRC_OVERRIDE_F_TRIGGER_MASK     = 0xC,
};

struct madt_local_apic_nmi
{
	struct madt_entry *entry;
	uint8_t acpi_processor_uid;
	uint16_t flags;
	uint8_t local_apic_lint;
} __attribute__ ((packed));

struct madt_gicc
{
	struct madt_entry entry;
	uint16_t reserved;
	uint32_t cpu_interface_number;
	uint32_t acpi_processor_uid;
	uint32_t flags;
	uint32_t parking_protocol_version;
	uint32_t performance_interrupt_gsiv;
	uint64_t parked_address;
	uint64_t physical_base_address;
	uint64_t gicv;
	uint64_t gich;
	uint32_t vgic_maintenance_interrupt;
	uint64_t gicr_base_address;
	uint64_t mpidr;
	uint8_t processor_efficiency_class;
	uint8_t reserved2;
	uint16_t spe_overflow_interrupt;
	uint16_t trbe_interrupt;
} __attribute__ ((packed));

enum madt_gicc_flags
{
	MADT_GICC_ENABLE                    = 0x1,
	MADT_GICC_PERFORMANCE_INT_MODE      = 0x2,
	MADT_GICC_VGIC_MAINTENANCE_INT_MODE = 0x4,
	MADT_GICC_ONLINE_CAPABLE            = 0x8,
};

struct madt_gicd
{
	struct madt_entry entry;
	uint16_t reserved;
	uint32_t gic_id;
	uint64_t physical_base_address;
	uint32_t system_vector_base;
	uint8_t gic_version;
	uint8_t reserved2[3];
} __attribute__ ((packed));

struct madt_gic_msi_frame
{
	struct madt_entry entry;
	uint16_t reserved;
	uint32_t gic_msi_frame_id;
	uint64_t physical_base_address;
	uint32_t flags;
	uint16_t spi_count;
	uint16_t spi_base;
} __attribute__ ((packed));

enum madt_gic_msi_frame_flags
{
	MADT_GIC_MSI_FRAME_SPI_COUNT_BASE_SELECT = 0x1,
};

enum acpi_address_space_id
{
	ACPI_ADDR_SYSTEM_MEM = 0x0,
	ACPI_ADDR_SYSTEM_IO  = 0x1,
	ACPI_ADDR_PCI_CONF   = 0x2,
	ACPI_ADDR_EMBED_CTRL = 0x3,
	ACPI_ADDR_SMBUS      = 0x4,
	ACPI_ADDR_SYSTEMCMOS = 0x5,
	ACPI_ADDR_PRICBAR    = 0x6,
	ACPI_ADDR_IPMI       = 0x7,
	ACPI_ADDR_GPIO       = 0x8,
	ACPI_ADDR_GEN_SERIAL = 0x9,
	ACPI_ADDR_PCC        = 0xA,
	ACPI_ADDR_FIXED_HARD = 0x7F,
	ACPI_ADDR_RESERVED   = 0x80, /* 0x80 - 0xBF */
	ACPI_ADDR_OEM        = 0xC0, /* 0xC0 - 0xFF */
};

struct acpi_gas
{
	uint8_t address_space_id;
	uint8_t bit_width;
	uint8_t bit_offset;
	uint8_t access_size;
	uint64_t address;
} __attribute__ ((packed));

struct fadt
{
	struct acpi_hdr hdr;
	uint32_t firmware_ctrl;
	uint32_t dsdt;
	uint8_t reserved;
	uint8_t preferred_pm_profile;
	uint16_t sci_int;
	uint32_t smi_cmd;
	uint8_t acpi_enable;
	uint8_t acpi_disable;
	uint8_t s4bios_req;
	uint8_t pstate_cnt;
	uint32_t pm1a_evt_blk;
	uint32_t pm1b_evt_blk;
	uint32_t pm1a_cnt_blk;
	uint32_t pm1b_cnt_blk;
	uint32_t pm2_cnt_blk;
	uint32_t pm_tmr_blk;
	uint32_t gpe0_blk;
	uint32_t gpe1_blk;
	uint8_t pm1_evt_len;
	uint8_t pm1_cnt_len;
	uint8_t pm2_cnt_len;
	uint8_t pm_tmr_len;
	uint8_t gpe0_blk_len;
	uint8_t gpe1_blk_len;
	uint8_t gpe1_base;
	uint8_t cst_cnt;
	uint16_t p_lvl2_lat;
	uint16_t p_lvl3_lat;
	uint16_t flush_size;
	uint16_t flush_stride;
	uint8_t duty_offset;
	uint8_t duty_width;
	uint8_t day_alrm;
	uint8_t month_alrm;
	uint8_t century;
	uint16_t iapc_boot_arch;
	uint8_t  reserved2;
	uint32_t flags;
	struct acpi_gas reset_reg;
	uint8_t reset_value;
	uint16_t arm_boot_arch;
	uint8_t minor_version;
	uint64_t x_firmware_ctrl;
	uint64_t x_dsdt;
	struct acpi_gas x_pm1a_evt_blk;
	struct acpi_gas x_pm1b_evt_blk;
	struct acpi_gas x_pm1a_cnt_blk;
	struct acpi_gas x_pm1b_cnt_blk;
	struct acpi_gas x_pm2_cnt_blk;
	struct acpi_gas x_pm_tmr_blk;
	struct acpi_gas x_gpe0_blk;
	struct acpi_gas x_gpe1_blk;
	struct acpi_gas sleep_control_reg;
	struct acpi_gas sleep_status_reg;
	uint64_t hypervisor_vendor_identity;
} __attribute__ ((packed));

struct hpet
{
	struct acpi_hdr hdr;
	uint32_t hw_id;
	struct acpi_gas base_addr;
	uint8_t number;
	uint16_t min_clock_ticks;
	uint8_t page_prot;
} __attribute__ ((packed));

struct mcfg_entry
{
	uint32_t base_addr[2];
	uint16_t pci_seg_group;
	uint8_t pci_bus_start;
	uint8_t pci_bus_end;
	uint32_t reserved;
} __attribute__ ((packed));

struct mcfg
{
	struct acpi_hdr hdr;
	uint32_t reserved[2];
	struct mcfg_entry entries[];
} __attribute__ ((packed));

struct dsdt
{
	struct acpi_hdr hdr;
	uint8_t aml[];
};

struct tpm2
{
	struct acpi_hdr hdr;
	uint16_t platform_class;
	uint16_t reserved;
	uint64_t base_address;
	uint32_t start_method;
	uint8_t parameters[12];
	uint32_t laml;
	uint64_t lasa;
};

struct facs
{
	char signature[4];
	uint32_t length;
	uint32_t hardware_signature;
	uint32_t waking_vector;
	uint32_t global_lock;
	uint32_t flags;
	uint64_t x_waking_vector;
	uint8_t version;
	uint8_t reserved33[3];
	uint32_t ospm_flags;
	uint8_t reserved40[24];
};

static const struct rsdp *rsdp;
static const struct rsdt *rsdt;
static const struct xsdt *xsdt;
static const struct fadt *fadt;
static const struct madt *madt;
static const struct hpet *hpet;
static const struct mcfg *mcfg;
static const struct dsdt *dsdt;
static const struct ssdt *ssdt[MAX_SSDT];
static const struct tpm2 *tpm2;
static struct facs *facs;

static struct node *rsdp_node;
static struct node *rsdt_node;
static struct node *xsdt_node;
static struct node *fadt_node;
static struct node *madt_node;
static struct node *hpet_node;
static struct node *mcfg_node;
static struct node *dsdt_node;
static struct node *ssdt_nodes[MAX_SSDT];
static struct node *tpm2_node;
static struct node *facs_node;
static struct node *acpi_node;

static struct aml_state *aml_state;

static const void *map_table(uint32_t addr);
static void unmap_table(const struct acpi_hdr *hdr);

void wakeup_trampoline(void); /* XXX arch-dep */

static void print_acpi_hdr(struct uio *uio, const struct acpi_hdr *hdr)
{
	uprintf(uio, "signature  : %c%c%c%c\n"
	             "length     : 0x%08" PRIx32 "\n"
	             "revision   : 0x%02" PRIx8 "\n"
	             "checksum   : 0x%02" PRIx8 "\n"
	             "oem id     : %c%c%c%c%c%c\n"
	             "oem tbl id : %c%c%c%c%c%c%c%c\n"
	             "oem rev    : 0x%08" PRIx32 "\n"
	             "creator id : 0x%08" PRIx32 "\n"
	             "creator rev: 0x%08" PRIx32 "\n",
	             hdr->signature[0],
	             hdr->signature[1],
	             hdr->signature[2],
	             hdr->signature[3],
	             hdr->length,
	             hdr->revision,
	             hdr->checksum,
	             hdr->oem_id[0],
	             hdr->oem_id[1],
	             hdr->oem_id[2],
	             hdr->oem_id[3],
	             hdr->oem_id[4],
	             hdr->oem_id[5],
	             hdr->oem_tableid[0],
	             hdr->oem_tableid[1],
	             hdr->oem_tableid[2],
	             hdr->oem_tableid[3],
	             hdr->oem_tableid[4],
	             hdr->oem_tableid[5],
	             hdr->oem_tableid[6],
	             hdr->oem_tableid[7],
	             hdr->oem_revision,
	             hdr->creator_id,
	             hdr->creator_revision);
}

static ssize_t rsdp_fread(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	uprintf(uio, "signature: 0x%02" PRIx8 "%02" PRIx8 "%02" PRIx8 "%02" PRIx8 "%02" PRIx8 "%02" PRIx8 "%02" PRIx8 "%02" PRIx8 "\n"
	             "checksum : 0x%02" PRIx8 "\n"
	             "oem id   : %c%c%c%c%c%c\n"
	             "revision : 0x%02" PRIx8 "\n"
	             "rsdt     : 0x%08" PRIx32 "\n",
	             rsdp->signature[0],
	             rsdp->signature[1],
	             rsdp->signature[2],
	             rsdp->signature[3],
	             rsdp->signature[4],
	             rsdp->signature[5],
	             rsdp->signature[6],
	             rsdp->signature[7],
	             rsdp->checksum,
	             rsdp->oem_id[0],
	             rsdp->oem_id[1],
	             rsdp->oem_id[2],
	             rsdp->oem_id[3],
	             rsdp->oem_id[4],
	             rsdp->oem_id[5],
	             rsdp->revision,
	             rsdp->rsdt);
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op rsdp_fop =
{
	.read = rsdp_fread,
};

static ssize_t rsdt_fread(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	print_acpi_hdr(uio, &rsdt->hdr);
	uprintf(uio, "entries    : ");
	for (size_t i = 0; i < (rsdt->hdr.length - sizeof(*rsdt)) / 4; ++i)
	{
		const struct acpi_hdr *hdr = rsdt->data[i] ? map_table(rsdt->data[i]) : NULL;
		if (i)
			uprintf(uio, "             ");
		char tbl_name[5];
		if (hdr)
		{
			*(uint32_t*)&tbl_name[0] = *(uint32_t*)hdr;
			tbl_name[4] = '\0';
		}
		else
		{
			tbl_name[0] = '\0';
		}
		uprintf(uio, "0x%08" PRIx32 " (%s)\n", rsdt->data[i], tbl_name);
		if (hdr)
			unmap_table(hdr);
	}
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op rsdt_fop =
{
	.read = rsdt_fread,
};

static ssize_t xsdt_fread(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	print_acpi_hdr(uio, &xsdt->hdr);
	uprintf(uio, "entries    : ");
	for (size_t i = 0; i < (xsdt->hdr.length - sizeof(*xsdt)) / 8; ++i)
	{
		const struct acpi_hdr *hdr = xsdt->data[i] ? map_table(xsdt->data[i]) : NULL;
		if (i)
			uprintf(uio, "             ");
		char tbl_name[5];
		if (hdr)
		{
			*(uint32_t*)&tbl_name[0] = *(uint32_t*)hdr;
			tbl_name[4] = '\0';
		}
		else
		{
			tbl_name[0] = '\0';
		}
		uprintf(uio, "0x%016" PRIx64 " (%s)\n", xsdt->data[i], tbl_name);
		if (hdr)
			unmap_table(hdr);
	}
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op xsdt_fop =
{
	.read = xsdt_fread,
};

static ssize_t fadt_fread(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	print_acpi_hdr(uio, &fadt->hdr);
	uprintf(uio, "fw ctrl    : 0x%08" PRIx32 "\n"
	             "dsdt       : 0x%08" PRIx32 "\n"
	             "pref pm    : 0x%02" PRIx8  "\n"
	             "sci int    : 0x%04" PRIx16 "\n"
	             "smi cmd    : 0x%08" PRIx32 "\n"
	             "acpi en    : 0x%02" PRIx8  "\n"
	             "acpi dis   : 0x%02" PRIx8  "\n"
	             "s4bios req : 0x%02" PRIx8  "\n"
	             "pstate cnt : 0x%02" PRIx8  "\n"
	             "pm1a evt   : 0x%08" PRIx32 "\n"
	             "pm1b evt   : 0x%08" PRIx32 "\n"
	             "pm1a cnt   : 0x%08" PRIx32 "\n"
	             "pm1b cnt   : 0x%08" PRIx32 "\n"
	             "pm2 cnt    : 0x%08" PRIx32 "\n"
	             "pm tmr     : 0x%08" PRIx32 "\n"
	             "gpe0       : 0x%08" PRIx32 "\n"
	             "gpe1       : 0x%08" PRIx32 "\n"
	             "pm1 evt len: 0x%02" PRIx8  "\n"
	             "pm1 cnt len: 0x%02" PRIx8  "\n"
	             "pm2 cnt len: 0x%02" PRIx8  "\n"
	             "pm tmr len : 0x%02" PRIx8  "\n"
	             "gpe0 len   : 0x%02" PRIx8  "\n"
	             "gpe1 len   : 0x%02" PRIx8  "\n"
	             "gpe1 base  : 0x%02" PRIx8  "\n"
	             "cst cnt    : 0x%02" PRIx8  "\n"
	             "p lvl2 lat : 0x%04" PRIx16 "\n"
	             "p lvl3 lat : 0x%04" PRIx16 "\n"
	             "flush size : 0x%04" PRIx16 "\n"
	             "flush strde: 0x%04" PRIx16 "\n"
	             "duty off   : 0x%02" PRIx8  "\n"
	             "duty width : 0x%02" PRIx8  "\n"
	             "day alarm  : 0x%02" PRIx8  "\n"
	             "mon alarm  : 0x%02" PRIx8  "\n"
	             "century    : 0x%02" PRIx8  "\n"
	             "ipac boot  : 0x%04" PRIx16 "\n"
	             "flags      : 0x%08" PRIx32 "\n"
	             "reset reg  : " GAS_FMT "\n"
	             "reset val  : 0x%02" PRIx8  "\n"
	             "arm boot   : 0x%04" PRIx16 "\n"
	             "min version: 0x%02" PRIx8  "\n"
	             "x fw ctrl  : 0x%016" PRIx64 "\n"
	             "x dsdt     : 0x%016" PRIx64 "\n"
	             "x pm1a evt : " GAS_FMT "\n"
	             "x pm1b evt : " GAS_FMT "\n"
	             "x pm1a cnt : " GAS_FMT "\n"
	             "x pm1b cnt : " GAS_FMT "\n"
	             "x pm2 cnt  : " GAS_FMT "\n"
	             "x pm tmr   : " GAS_FMT "\n"
	             "x gpe0     : " GAS_FMT "\n"
	             "x gpe1     : " GAS_FMT "\n"
	             "sleep cnt  : " GAS_FMT "\n"
	             "sleep sts  : " GAS_FMT "\n"
	             "hypervisor : 0x%016" PRIx64 "\n",
	             fadt->firmware_ctrl,
	             fadt->dsdt,
	             fadt->preferred_pm_profile,
	             fadt->sci_int,
	             fadt->smi_cmd,
	             fadt->acpi_enable,
	             fadt->acpi_disable,
	             fadt->s4bios_req,
	             fadt->pstate_cnt,
	             fadt->pm1a_evt_blk,
	             fadt->pm1b_evt_blk,
	             fadt->pm1a_cnt_blk,
	             fadt->pm1b_cnt_blk,
	             fadt->pm2_cnt_blk,
	             fadt->pm_tmr_blk,
	             fadt->gpe0_blk,
	             fadt->gpe1_blk,
	             fadt->pm1_evt_len,
	             fadt->pm1_cnt_len,
	             fadt->pm2_cnt_len,
	             fadt->pm_tmr_len,
	             fadt->gpe0_blk_len,
	             fadt->gpe1_blk_len,
	             fadt->gpe1_blk_len,
	             fadt->cst_cnt,
	             fadt->p_lvl2_lat,
	             fadt->p_lvl3_lat,
	             fadt->flush_size,
	             fadt->flush_stride,
	             fadt->duty_offset,
	             fadt->duty_width,
	             fadt->day_alrm,
	             fadt->month_alrm,
	             fadt->century,
	             fadt->iapc_boot_arch,
	             fadt->flags,
	             GAS_VAL(fadt->reset_reg),
	             fadt->reset_value,
	             fadt->arm_boot_arch,
	             fadt->minor_version,
	             fadt->x_firmware_ctrl,
	             fadt->x_dsdt,
	             GAS_VAL(fadt->x_pm1a_evt_blk),
	             GAS_VAL(fadt->x_pm1b_evt_blk),
	             GAS_VAL(fadt->x_pm1a_cnt_blk),
	             GAS_VAL(fadt->x_pm1b_cnt_blk),
	             GAS_VAL(fadt->x_pm2_cnt_blk),
	             GAS_VAL(fadt->x_pm_tmr_blk),
	             GAS_VAL(fadt->x_gpe0_blk),
	             GAS_VAL(fadt->x_gpe1_blk),
	             GAS_VAL(fadt->sleep_control_reg),
	             GAS_VAL(fadt->sleep_status_reg),
	             fadt->hypervisor_vendor_identity);
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op fadt_fop =
{
	.read = fadt_fread,
};

static ssize_t madt_fread(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	print_acpi_hdr(uio, &madt->hdr);
	struct madt_entry *entry = (struct madt_entry*)((uint8_t*)madt + sizeof(*madt));
	do
	{
		switch (entry->type)
		{
			case MADT_LOCAL_APIC:
			{
				struct madt_local_apic *local_apic = (struct madt_local_apic*)entry;
				uprintf(uio, "local apic\n"
				             "  acpi cpu: 0x%02" PRIx8 "\n"
				             "  apic id : 0x%02" PRIx8 "\n"
				             "  flags   : 0x%08" PRIx32 "\n",
				             local_apic->acpi_cpuid,
				             local_apic->apic_id,
				             local_apic->flags);
				break;
			}
			case MADT_IO_APIC:
			{
				struct madt_io_apic *io_apic = (struct madt_io_apic*)entry;
				uprintf(uio, "io apic\n"
				             "  apic id  : 0x%02" PRIx8 "\n"
				             "  apic addr: 0x%08" PRIx32 "\n"
				             "  gsib     : 0x%08" PRIx32 "\n",
				             io_apic->apic_id,
				             io_apic->apic_addr,
				             io_apic->gsib);
				break;
			}
			case MADT_INT_SRC_OVERRIDE:
			{
				struct madt_int_src_override *src_override = (struct madt_int_src_override*)entry;
				uprintf(uio, "int src\n"
				             "  source: 0x%02" PRIx8 "\n"
				             "  gsi   : 0x%08" PRIx32 "\n"
				             "  flags : 0x%02" PRIx8 "\n",
				             src_override->source,
				             src_override->gsi,
				             src_override->flags);
				break;
			}
			case MADT_LOCAL_APIC_NMI:
			{
				struct madt_local_apic_nmi *local_apic_nmi = (struct madt_local_apic_nmi*)entry;
				uprintf(uio, "lapic nmi\n"
				             "  proc uid: 0x%02" PRIx8 "\n"
				             "  flags   : 0x%04" PRIx16 "\n"
				             "  lint    : 0x%02" PRIx8 "\n",
				             local_apic_nmi->acpi_processor_uid,
				             local_apic_nmi->flags,
				             local_apic_nmi->local_apic_lint);
				break;
			}
			case MADT_GICC:
			{
				struct madt_gicc *gicc = (struct madt_gicc*)entry;
				uprintf(uio, "gicc:\n"
				             "  cpu itf nb   : 0x%08" PRIx32 "\n"
				             "  acpi proc uid: 0x%08" PRIx32 "\n"
				             "  flags        : 0x%08" PRIx32 "\n"
				             "  park proto v.: 0x%08" PRIx32 "\n"
				             "  perf int gsiv: 0x%08" PRIx32 "\n"
				             "  parked addr  : 0x%016" PRIx64 "\n"
				             "  base addr    : 0x%016" PRIx64 "\n"
				             "  gicv         : 0x%016" PRIx64 "\n"
				             "  gich         : 0x%016" PRIx64 "\n"
				             "  vgic mgmt int: 0x%08" PRIx32 "\n"
				             "  gicr addr    : 0x%016" PRIx64 "\n"
				             "  mpidr        : 0x%016" PRIx64 "\n"
				             "  proc eff cls : 0x%02" PRIx8 "\n"
				             "  spe overf int: 0x%04" PRIx16 "\n"
				             "  trbe int     : 0x%04" PRIx16 "\n",
				             gicc->cpu_interface_number,
				             gicc->acpi_processor_uid,
				             gicc->flags,
				             gicc->parking_protocol_version,
				             gicc->performance_interrupt_gsiv,
				             gicc->parked_address,
				             gicc->physical_base_address,
				             gicc->gicv,
				             gicc->gich,
				             gicc->vgic_maintenance_interrupt,
				             gicc->gicr_base_address,
				             gicc->mpidr,
				             gicc->processor_efficiency_class,
				             gicc->spe_overflow_interrupt,
				             gicc->trbe_interrupt);
				break;
			}
			case MADT_GICD:
			{
				struct madt_gicd *gicd = (struct madt_gicd*)entry;
				uprintf(uio, "gicd:\n"
				             "  gic id   : 0x%08" PRIx32 "\n"
				             "  base addr: 0x%016" PRIx64 "\n"
				             "  vec base : 0x%08" PRIx32 "\n"
				             "  gic ver  : 0x%02" PRIx8 "\n",
				             gicd->gic_id,
				             gicd->physical_base_address,
				             gicd->system_vector_base,
				             gicd->gic_version);
				break;
			}
			case MADT_GIC_MSI_FRAME:
			{
				struct madt_gic_msi_frame *gic_msi_frame = (struct madt_gic_msi_frame*)entry;
				uprintf(uio, "gic msi frame:\n"
				             "  frame id : 0x%08" PRIx32 "\n"
				             "  base addr: 0x%016" PRIx64 "\n"
				             "  flags    : 0x%08" PRIx32 "\n"
				             "  spi count: 0x%04" PRIx16 "\n"
				             "  spi base : 0x%04" PRIx16 "\n",
				             gic_msi_frame->gic_msi_frame_id,
				             gic_msi_frame->physical_base_address,
				             gic_msi_frame->flags,
				             gic_msi_frame->spi_count,
				             gic_msi_frame->spi_base);
				break;
			}
			default:
				uprintf(uio, "acpi: unhandled madt entry type: %" PRIx8 "\n", entry->type);
		}
		entry = (struct madt_entry*)((uint8_t*)entry + entry->length);
	} while ((uint8_t*)entry < (uint8_t*)madt + madt->hdr.length);
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op madt_fop =
{
	.read = madt_fread,
};

static ssize_t hpet_fread(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	print_acpi_hdr(uio, &hpet->hdr);
	uprintf(uio, "hardware id: 0x%08" PRIx32 "\n"
	             "base addr  : 0x%08" PRIx32 "\n"
	             "number     : 0x%02" PRIx8 "\n"
	             "min ticks  : 0x%04" PRIx16 "\n"
	             "prot       : 0x%02" PRIx8 "\n",
	             hpet->hw_id,
	             (uint32_t)hpet->base_addr.address,
	             hpet->number,
	             hpet->min_clock_ticks,
	             hpet->page_prot);
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op hpet_fop =
{
	.read = hpet_fread,
};

static ssize_t mcfg_fread(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	print_acpi_hdr(uio, &mcfg->hdr);
	for (size_t i = 0; i < (mcfg->hdr.length - sizeof(*mcfg)) / sizeof(*mcfg->entries); ++i)
	{
		const struct mcfg_entry *entry = &mcfg->entries[i];
		uprintf(uio, "addr       : 0x%08" PRIx32 "%08" PRIx32 "\n"
		             "segment    : %" PRIu16 "\n"
		             "bus start  : %" PRIu8 "\n"
		             "bus end    : %" PRIu8 "\n",
		             entry->base_addr[1],
		             entry->base_addr[0],
		             entry->pci_seg_group,
		             entry->pci_bus_start,
		             entry->pci_bus_end);
	}
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op mcfg_fop =
{
	.read = mcfg_fread,
};

static ssize_t dsdt_fread(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	print_acpi_hdr(uio, &dsdt->hdr);
#if 0
	size_t n = dsdt->hdr.length - sizeof(*dsdt);
	size_t i;
	for (i = 0; n > 16; i += 16, n -= 16)
	{
		uprintf(uio, "[%04zx] %02x %02x %02x %02x "
		                     "%02x %02x %02x %02x "
		                     "%02x %02x %02x %02x "
		                     "%02x %02x %02x %02x\n",
		        i,
		        dsdt->aml[i + 0x0],
		        dsdt->aml[i + 0x1],
		        dsdt->aml[i + 0x2],
		        dsdt->aml[i + 0x3],
		        dsdt->aml[i + 0x4],
		        dsdt->aml[i + 0x5],
		        dsdt->aml[i + 0x6],
		        dsdt->aml[i + 0x7],
		        dsdt->aml[i + 0x8],
		        dsdt->aml[i + 0x9],
		        dsdt->aml[i + 0xA],
		        dsdt->aml[i + 0xB],
		        dsdt->aml[i + 0xC],
		        dsdt->aml[i + 0xD],
		        dsdt->aml[i + 0xE],
		        dsdt->aml[i + 0xF]);
	}
	if (n)
	{
		uprintf(uio, "[%04zx] ", i);
		for (; n; ++i, --n)
			uprintf(uio, "%02x ", dsdt->aml[i]);
		uprintf(uio, "\n");
	}
#endif
	aml_print_asl(uio, aml_state, dsdt->aml, dsdt->hdr.length - sizeof(*dsdt));
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op dsdt_fop =
{
	.read = dsdt_fread,
};

static ssize_t tpm2_fread(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	print_acpi_hdr(uio, &tpm2->hdr);
	uprintf(uio, "platform   : 0x%04" PRIx16 "\n", tpm2->platform_class);
	uprintf(uio, "addr       : 0x%016" PRIx64 "\n", tpm2->base_address);
	uprintf(uio, "start meth : 0x%08" PRIx32 "\n", tpm2->start_method);
	uprintf(uio, "laml       : 0x%08" PRIx32 "\n", tpm2->laml);
	uprintf(uio, "lasa       : 0x%016" PRIx64 "\n", tpm2->lasa);
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op tpm2_fop =
{
	.read = tpm2_fread,
};

static ssize_t facs_fread(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	uprintf(uio, "signature: %c%c%c%c\n", facs->signature[0],
	        facs->signature[1], facs->signature[2], facs->signature[3]);
	uprintf(uio, "length   : 0x%" PRIx32 "\n", facs->length);
	uprintf(uio, "hw_sig   : 0x%" PRIx32 "\n", facs->hardware_signature);
	uprintf(uio, "wake_v   : 0x%" PRIx32 "\n", facs->waking_vector);
	uprintf(uio, "gl_lock  : 0x%" PRIx32 "\n", facs->global_lock);
	uprintf(uio, "flags    : 0x%" PRIx32 "\n", facs->flags);
	uprintf(uio, "x_wake_v : 0x%" PRIx64 "\n", facs->x_waking_vector);
	uprintf(uio, "version  : 0x%" PRIx8 "\n", facs->version);
	uprintf(uio, "ospm_fl  : 0x%" PRIx32 "\n", facs->ospm_flags);
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op facs_fop =
{
	.read = facs_fread,
};

static ssize_t acpi_fread(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	aml_print(uio, aml_state);
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op acpi_fop =
{
	.read = acpi_fread,
};

#if defined(__i386__) || defined(__x86_64__)

#define GAS_PIO_OUT(name) \
do \
{ \
	if (gas->address_space_id == ACPI_ADDR_SYSTEM_IO) \
	{ \
		out##name(gas->address + off, v); \
		return 0; \
	} \
} while (0)

#define GAS_PIO_IN(name) \
do \
{ \
	if (gas->address_space_id == ACPI_ADDR_SYSTEM_IO) \
		return in##name(gas->address + off); \
} while (0)

#else

#define GAS_PIO_OUT(name)
#define GAS_IO_IN(name)

#endif

#define GAS_OUT(name, size) \
static int gas_out##name(const struct acpi_gas *gas, size_t off, uint##size##_t v) \
{ \
	GAS_PIO_OUT(name); \
	if (gas->address_space_id == ACPI_ADDR_SYSTEM_MEM) \
	{ \
		uintptr_t paddr = gas->address + off; \
		struct page page; \
		pm_init_page(&page, paddr / PAGE_SIZE); \
		uint8_t *map = vm_map(&page, PAGE_SIZE, VM_PROT_RW); \
		if (!map) \
			return -ENOMEM; \
		*(uint##size##_t*)&map[paddr % PAGE_SIZE] = v; \
		__atomic_thread_fence(__ATOMIC_RELEASE); \
		vm_unmap(map, PAGE_SIZE); \
		return 0; \
	} \
	return 0; \
}

#define GAS_IN(name, size) \

GAS_OUT(b, 8);
GAS_OUT(w, 16);

GAS_IN(w, 16);

static uint8_t acpi_checksum(const void *data, size_t size)
{
	uint8_t checksum = 0;
	for (size_t i = 0; i < size; ++i)
		checksum += ((const uint8_t*)data)[i];
	return checksum;
}

static uint8_t acpi_table_checksum(const struct acpi_hdr *hdr)
{
	return acpi_checksum(hdr, hdr->length);
}

static const void *map_table(uint32_t addr)
{
	struct page page;
	pm_init_page(&page, addr / PAGE_SIZE);
	uint8_t *map_base = vm_map(&page, PAGE_SIZE, VM_PROT_R);
	if (!map_base)
		return NULL;
	struct acpi_hdr *hdr = (struct acpi_hdr*)(map_base + (addr & PAGE_MASK));
	size_t len = (hdr->length + (addr & PAGE_MASK) + PAGE_MASK) & ~PAGE_MASK;
	if (len <= PAGE_SIZE)
		return hdr;
	uint8_t *map_base2 = vm_map(&page, len, VM_PROT_R);
	vm_unmap(map_base, PAGE_SIZE);
	if (!map_base2)
		return NULL;
	return map_base2 + (addr & PAGE_MASK);
}

static void unmap_table(const struct acpi_hdr *hdr)
{
	size_t len = (hdr->length + ((uintptr_t)hdr & PAGE_MASK) + PAGE_MASK) & ~PAGE_MASK;
	vm_unmap((void*)((uintptr_t)hdr & ~PAGE_MASK), len);
}

static const void *map_verify_table(size_t addr)
{
	const struct acpi_hdr *hdr = map_table(addr);
	if (!hdr)
		return NULL;
	uint8_t checksum = acpi_table_checksum(hdr);
	if (checksum)
	{
		printf("acpi: invalid checksum: %02" PRIx8 "\n", checksum);
		unmap_table(hdr);
		return NULL;
	}
	return hdr;
}

const void *rsdt_find_table(const char *name)
{
	for (uint32_t i = 0; i < (rsdt->hdr.length - sizeof(*rsdt)) / 4; i++)
	{
		if (!rsdt->data[i])
			continue;
		const struct acpi_hdr *hdr = map_table(rsdt->data[i]);
		if (!hdr)
			continue;
		if (!memcmp(hdr->signature, name, 4))
		{
			uint8_t checksum = acpi_table_checksum(hdr);
			if (!checksum)
				return hdr;
			printf("acpi: invalid %s checksum: %02" PRIx8 "\n",
			       name, checksum);
		}
		unmap_table(hdr);
	}
	return NULL;
}

const void *xsdt_find_table(const char *name)
{
	for (uint32_t i = 0; i < (xsdt->hdr.length - sizeof(*xsdt)) / 8; i++)
	{
		if (!xsdt->data[i])
			continue;
		const struct acpi_hdr *hdr = map_table(xsdt->data[i]);
		if (!hdr)
			continue;
		if (!memcmp(hdr->signature, name, 4))
		{
			uint8_t checksum = acpi_table_checksum(hdr);
			if (!checksum)
				return hdr;
			printf("acpi: invalid %s checksum: %02" PRIx8 "\n",
			       name, checksum);
		}
		unmap_table(hdr);
	}
	return NULL;
}

static void handle_dsdt(void)
{
	int ret = sysfs_mknode("acpi/dsdt", 0, 0, 0400, &dsdt_fop, &dsdt_node);
	if (ret)
		printf("acpi: failed to create dsdt sysnode: %s\n", strerror(ret));
	ret = aml_parse(aml_state, dsdt->aml, dsdt->hdr.length - sizeof(*dsdt));
	if (ret)
		printf("acpi: failed to parse dsdt: %s\n", strerror(ret));
}

static void handle_facs(void)
{
	int ret = sysfs_mknode("acpi/facs", 0, 0, 0400, &facs_fop, &facs_node);
	if (ret)
		printf("acpi: failed to create facs sysnode: %s\n", strerror(ret));
}

static int map_facs(size_t addr)
{
	struct page page;
	pm_init_page(&page, addr / PAGE_SIZE);
	uint8_t *map_base = vm_map(&page, PAGE_SIZE * 2, VM_PROT_RW);
	if (!map_base)
		return -ENOMEM;
	facs = (struct facs*)&map_base[addr % PAGE_SIZE];
	return 0;
}

static void handle_fadt(void)
{
	int ret = sysfs_mknode("acpi/fadt", 0, 0, 0400, &fadt_fop, &fadt_node);
	if (ret)
		printf("acpi: failed to create fadt sysnode: %s\n", strerror(ret));
	if (fadt->x_dsdt)
	{
#if __SIZE_WIDTH__ == 32
		if (fadt->x_dsdt >> 32)
			panic("acpi: x_dsdt out of range\n");
#endif
		dsdt = map_verify_table(fadt->x_dsdt);
		handle_dsdt();
	}
	else if (fadt->dsdt)
	{
		dsdt = map_verify_table(fadt->dsdt);
		handle_dsdt();
	}
	if (fadt->x_firmware_ctrl)
	{
#if __SIZE_WIDTH__ == 32
		if (fadt->x_firmware_ctrl >> 32)
			panic("acpi: x_firmware_ctrl out of range\n");
#endif
		if (!map_facs(fadt->x_firmware_ctrl))
			handle_facs();
		else
			printf("acpi: failed to map facs\n");
	}
	else if (fadt->firmware_ctrl)
	{
		if (!map_facs(fadt->firmware_ctrl))
			handle_facs();
		else
			printf("acpi: failed to map facs\n");
	}
#if defined(__aarch64__)
	if (fadt->arm_boot_arch & FADT_ARM_PSCI)
		psci_init(!!(fadt->arm_boot_arch & FADT_ARM_HVC));
#endif
}

static void handle_madt(void)
{
	int ret = sysfs_mknode("acpi/madt", 0, 0, 0400, &madt_fop, &madt_node);
	if (ret)
		printf("acpi: failed to create madt sysnode: %s\n", strerror(ret));
	struct madt_entry *entry = (struct madt_entry*)((uint8_t*)madt + sizeof(*madt));
	do
	{
		switch (entry->type)
		{
#if defined(__i386__) || defined(__x86_64__)
			case MADT_LOCAL_APIC:
			{
				struct madt_local_apic *local_apic = (struct madt_local_apic*)entry;
				if (local_apic->flags == MADT_LOCAL_APIC_F_ENABLED)
					g_lapics[g_lapics_count++] = local_apic->apic_id;
				break;
			}
			case MADT_IO_APIC:
			{
				struct madt_io_apic *io_apic = (struct madt_io_apic*)entry;
				ioapic_init(io_apic->apic_id, io_apic->apic_addr, io_apic->gsib);
				break;
			}
			case MADT_INT_SRC_OVERRIDE:
			{
				struct madt_int_src_override *src_override = (struct madt_int_src_override*)entry;
				g_isa_irq[src_override->source] = src_override->gsi;
				break;
			}
			case MADT_LOCAL_APIC_NMI:
			{
				struct madt_local_apic_nmi *local_apic_nmi = (struct madt_local_apic_nmi*)entry;
				(void)local_apic_nmi;
				break;
			}
#elif defined(__aarch64__)
			case MADT_GICC:
			{
				struct madt_gicc *gicc = (struct madt_gicc*)entry;
				if (!gicc->acpi_processor_uid)
					gicv2_init_gicc(gicc->physical_base_address);
				psci_add_cpu(gicc->mpidr, gicc->cpu_interface_number);
				break;
			}
			case MADT_GICD:
			{
				struct madt_gicd *gicd = (struct madt_gicd*)entry;
				gicv2_init_gicd(gicd->physical_base_address);
				break;
			}
			case MADT_GIC_MSI_FRAME:
			{
				struct madt_gic_msi_frame *gic_msi_frame = (struct madt_gic_msi_frame*)entry;
				gicv2_init_gicm(gic_msi_frame->physical_base_address);
				break;
			}
#endif
			default:
				panic("acpi: unhandled madt entry type: %" PRIx8 "\n", entry->type);
		}
		entry = (struct madt_entry*)((uint8_t*)entry + entry->length);
	} while ((uint8_t*)entry < (uint8_t*)madt + madt->hdr.length);
}

static void handle_hpet(void)
{
	int ret = sysfs_mknode("acpi/hpet", 0, 0, 0400, &hpet_fop, &hpet_node);
	if (ret)
		printf("acpi: failed to create hpet sysnode: %s\n", strerror(ret));
}

void acpi_hpet_init(void)
{
#if defined(__i386__) || defined(__x86_64__)
	hpet_init(hpet->hw_id, hpet->base_addr.address, hpet->number,
	          hpet->min_clock_ticks);
#endif
}

static void handle_mcfg(void)
{
	int ret = sysfs_mknode("acpi/mcfg", 0, 0, 0400, &mcfg_fop, &mcfg_node);
	if (ret)
		printf("acpi: failed to create madt sysnode: %s\n", strerror(ret));
}

static void handle_tpm2(void)
{
	int ret = sysfs_mknode("acpi/tpm2", 0, 0, 0400, &tpm2_fop, &tpm2_node);
	if (ret)
		printf("acpi: failed to create tpm2 sysnode: %s\n", strerror(ret));
}

int acpi_get_ecam_addr(uint8_t bus, uint8_t slot, uint8_t func, uintptr_t *poffp)
{
	if (!mcfg)
		return -ENOENT;
	size_t entries_count = (mcfg->hdr.length - sizeof(*mcfg)) / sizeof(*mcfg->entries);
	for (size_t i = 0; i < entries_count; ++i)
	{
		const struct mcfg_entry *entry = &mcfg->entries[i];
		if (bus < entry->pci_bus_start
		 || bus > entry->pci_bus_end)
			continue;
		*poffp = entry->base_addr[0];
#if __SIZE_WIDTH__ == 64
		*poffp |= (size_t)entry->base_addr[1] << 32;
#endif
		*poffp /= PAGE_SIZE;
		*poffp |= ((bus - entry->pci_bus_start) << 8);
		*poffp |= (slot << 3);
		*poffp |= (func << 0);
		return 0;
	}
	return -ENOENT;
}

static void handle_rsdt(void)
{
	uint8_t checksum = acpi_table_checksum(&rsdt->hdr);
	if (checksum)
		panic("acpi: invalid rsdt checksum: %02" PRIx8 "\n", checksum);
	int ret = sysfs_mknode("acpi/rsdt", 0, 0, 0400, &rsdt_fop, &rsdt_node);
	if (ret)
		printf("acpi: failed to create rsdt sysnode: %s\n", strerror(ret));
	fadt = rsdt_find_table("FACP");
	if (!fadt)
		panic("acpi: FACP not found\n");
	handle_fadt();
	madt = rsdt_find_table("APIC");
	if (!madt)
		panic("acpi: APIC not found\n");
	handle_madt();
	hpet = rsdt_find_table("HPET");
	if (hpet)
		handle_hpet();
	mcfg = rsdt_find_table("MCFG");
	if (mcfg)
		handle_mcfg();
	tpm2 = rsdt_find_table("TPM2");
	if (tpm2)
		handle_tpm2();
}

static void handle_xsdt(void)
{
	uint8_t checksum = acpi_table_checksum(&xsdt->hdr);
	if (checksum)
		panic("acpi: invalid xsdt checksum: %02" PRIx8 "\n", checksum);
	int ret = sysfs_mknode("acpi/xsdt", 0, 0, 0400, &xsdt_fop, &xsdt_node);
	if (ret)
		printf("acpi: failed to create xsdt sysnode: %s\n", strerror(ret));
	fadt = xsdt_find_table("FACP");
	if (!fadt)
		panic("acpi: FACP not found\n");
	handle_fadt();
	madt = xsdt_find_table("APIC");
	if (!madt)
		panic("acpi: APIC not found\n");
	handle_madt();
	hpet = xsdt_find_table("HPET");
	if (hpet)
		handle_hpet();
	mcfg = xsdt_find_table("MCFG");
	if (mcfg)
		handle_mcfg();
	tpm2 = xsdt_find_table("TPM2");
	if (tpm2)
		handle_tpm2();
}

static void handle_rsdp(void)
{
	uint8_t checksum = acpi_checksum(rsdp, sizeof(*rsdp));
	if (checksum)
		panic("acpi: invalid rsdp checksum: %02" PRIx8 "\n", checksum);
	int ret = sysfs_mknode("acpi/rsdp", 0, 0, 0400, &rsdp_fop, &rsdp_node);
	if (ret)
		printf("acpi: failed to create rsdp sysnode: %s\n", strerror(ret));
	if (rsdp->revision >= 2)
	{
		const struct rsdp2 *rsdp2 = (const struct rsdp2*)rsdp;
		checksum = acpi_checksum(&rsdp2->length, sizeof(*rsdp2) - sizeof(*rsdp));
		if (checksum)
			panic("acpi: invalid rsdp2 checksum\n");
		if (rsdp2->xsdt >= SIZE_MAX)
			panic("acpi: xsdt out of bounds\n");
		xsdt = map_table(rsdp2->xsdt);
		if (!xsdt)
			panic("acpi: failed to vmap XSDT\n");
		handle_xsdt();
	}
	else
	{
		rsdt = map_table(rsdp->rsdt);
		if (!rsdt)
			panic("acpi: failed to vmap RSDT\n");
		handle_rsdt();
	}
}

#if defined(__i386__) || defined(__x86_64__)
static struct irq_handle acpi_irq_handle;

static void handle_acpi_irq(void *userdata)
{
	(void)userdata;
	acpi_shutdown();
}
#endif

static void acpi_enable(void)
{
#if defined(__i386__) || defined(__x86_64__)
	if (!fadt->smi_cmd || (!fadt->acpi_enable && !fadt->acpi_disable))
		return;
	outb(fadt->smi_cmd, fadt->acpi_enable);
	while (!(inw(fadt->pm1a_cnt_blk) & 1));
	gas_outw(&fadt->x_pm1a_evt_blk, fadt->pm1_evt_len / 2, PM1_PWRBTN_EN);
	register_isa_irq(fadt->sci_int, handle_acpi_irq, NULL, &acpi_irq_handle);
#endif
}

static void facs_lock(void)
{
	while (1)
	{
		uint32_t v = __atomic_load_n(&facs->global_lock, __ATOMIC_SEQ_CST);
		uint32_t next = v;
		if (v & 0x2)
			next |= 0x1;
		else
			next = 0x2;
		if (!__atomic_compare_exchange_n(&facs->global_lock, &v, next, 0,
		                                 __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
			continue;
		if (!(v & 0x2))
			break;
	}
}

static void facs_unlock(void)
{
	while (1)
	{
		uint32_t v = __atomic_load_n(&facs->global_lock, __ATOMIC_SEQ_CST);
		uint32_t next = v & ~0x3;
		if (!__atomic_compare_exchange_n(&facs->global_lock, &v, next, 0,
		                                 __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
			continue;
		if (v & 0x1)
		{
			/* XXX GBL_RLS or BIOS_RLS */
		}
		break;
	}
}

static int enter_sleep_state(int state)
{
	assert(state >= 0 && state <= 5, "invalid sleep state");
	char name[5];
	snprintf(name, sizeof(name), "_S%d_", state);
	struct acpi_obj *obj = aml_get_obj(aml_state, name);
	if (!obj)
	{
		TRACE("%s not found", name);
		return -ENOEXEC;
	}
	if (obj->type != ACPI_OBJ_NAME)
	{
		TRACE("invalid %s type", name);
		return -ENOEXEC;
	}
	if (!obj->namedef.data)
	{
		TRACE("%s has no data", name);
		return -ENOEXEC;
	}
	struct acpi_data *pkg = obj->namedef.data;
	if (pkg->type != ACPI_DATA_PACKAGE)
	{
		TRACE("%s isn't a package", name);
		return -ENOEXEC;
	}
	struct acpi_data *typa = TAILQ_FIRST(&pkg->package.elements);
	if (!typa)
	{
		TRACE("empty %s elements", name);
		return -ENOEXEC;
	}
	uint16_t v;
	switch (typa->type)
	{
		case ACPI_DATA_ZERO:
			v = 0;
			break;
		case ACPI_DATA_ONE:
			v = 1;
			break;
		case ACPI_DATA_ONES:
			v = 0xFFFF;
			break;
		case ACPI_DATA_BYTE:
			v = typa->byte.value;
			break;
		case ACPI_DATA_WORD:
			v = typa->word.value;
			break;
		default:
			return -ENOEXEC;
	}
	/* XXX
	 * sleep drivers
	 * shutdown all SMP
	 * disable interrupts
	 */
	facs_lock();
	facs->waking_vector = (uintptr_t)wakeup_trampoline;
	facs_unlock();
	/* XXX save CPU ctx */
	struct acpi_obj *_tts = aml_get_obj(aml_state, "_TTS");
	if (_tts)
	{
		if (aml_exec(aml_state, _tts))
			TRACE("failed to execute _TTS");
	}
	struct acpi_obj *_pts = aml_get_obj(aml_state, "_PTS");
	if (_pts)
	{
		if (aml_exec(aml_state, _pts))
			TRACE("failed to execute _PTS");
	}
	struct acpi_obj *_gts = aml_get_obj(aml_state, "_GTS");
	if (_gts)
	{
		if (aml_exec(aml_state, _gts))
			TRACE("failed to execute _GTS");
	}
#if defined(__i386__) || defined(__x86_64__)
	wbinvd(); /* XXX arch_cache_flush() */
#endif
	gas_outw(&fadt->x_pm1a_cnt_blk, 0, v * PM1_SLP_TYP | PM1_SLP_EN);
	while (1);
	return 0;
}

int acpi_reboot(void)
{
	return gas_outb(&fadt->reset_reg, 0, fadt->reset_value);
}

int acpi_shutdown(void)
{
	return enter_sleep_state(5);
}

int acpi_suspend(void)
{
	return enter_sleep_state(3);
}

int acpi_hibernate(void)
{
	return enter_sleep_state(4);
}

int acpi_init(void)
{
	const struct multiboot_tag *tag = multiboot_find_tag(MULTIBOOT_TAG_TYPE_ACPI_NEW);
	if (tag)
	{
		rsdp = (struct rsdp*)&((struct multiboot_tag_old_acpi*)tag)->rsdp[0];
	}
	else
	{
		tag = multiboot_find_tag(MULTIBOOT_TAG_TYPE_ACPI_OLD);
		if (!tag)
			return -ENOENT;
		rsdp = (struct rsdp*)&((struct multiboot_tag_new_acpi*)tag)->rsdp[0];
	}
	if (aml_alloc(&aml_state))
		panic("acpi: failed to allocate aml state\n");
	handle_rsdp();
	acpi_enable();
	int ret = sysfs_mknode("acpi/acpi", 0, 0, 0400, &acpi_fop, &acpi_node);
	if (ret)
		printf("acpi: failed to create acpi sysnode: %s\n", strerror(ret));
	return 0;
}

void acpi_probe_devices(acpi_probe_t probe, void *userdata)
{
	if (!aml_state)
		return;
	struct acpi_obj *sb = aml_get_obj(aml_state, "_SB_");
	if (!sb)
	{
		TRACE("_SB_ not found");
		return;
	}
	if (sb->type != ACPI_OBJ_SCOPE)
	{
		TRACE("_SB_ isn't a scope");
		return;
	}
	struct acpi_obj *child;
	TAILQ_FOREACH(child, &sb->scope.ns.obj_list, chain)
	{
		if (child->type != ACPI_OBJ_DEVICE)
			continue;
		probe(child, userdata);
	}
}

static int acpi_resource_get_entry(struct acpi_data *data, uint8_t searched,
                                   size_t *off, size_t *lengthp)
{
	if (data->type != ACPI_DATA_BUFFER)
	{
		TRACE("acpi resource isn't a buffer");
		return -EINVAL;
	}
	size_t i = 0;
	while (i < data->buffer.size)
	{
		uint8_t type = data->buffer.data[i];
		if (type & 0x80)
		{
			if (i + 3 > data->buffer.size)
			{
				TRACE("acpi long header out of range");
				return -EILSEQ;
			}
			uint16_t length = (data->buffer.data[i + 1] << 0)
			                | (data->buffer.data[i + 2] << 8);
			if (i + 3 + length > data->buffer.size)
			{
				TRACE("acpi long resource out of range");
				return -EILSEQ;
			}
			if (type == searched)
			{
				*off = i + 3;
				*lengthp = length;
				return 0;
			}
			i += length + 3;
		}
		else
		{
			uint8_t length = type & 0x7;
			if (i + 1 + length > data->buffer.size)
			{
				TRACE("acpi short resource out of range");
				return -EILSEQ;
			}
			if (((type >> 3) & 0xF) == searched)
			{
				*off = i + 1;
				*lengthp = length;
				return 0;
			}
			i += length + 1;
		}
	}
	return -ENOENT;
}

int acpi_resource_get_fixed_memory_range_32(struct acpi_data *data,
                                            uint8_t *info,
                                            uint32_t *base,
                                            uint32_t *size)
{
	size_t length;
	size_t off;
	int ret = acpi_resource_get_entry(data, 0x86, &off, &length);
	if (ret)
		return ret;
	if (length != 0x9)
	{
		TRACE("memory range 32 length isn't 9");
		return -EINVAL;
	}
	*info = data->buffer.data[off];
	*base = (data->buffer.data[off + 1] << 0)
	      | (data->buffer.data[off + 2] << 8)
	      | (data->buffer.data[off + 3] << 16)
	      | (data->buffer.data[off + 4] << 24);
	*size = (data->buffer.data[off + 5] << 0)
	      | (data->buffer.data[off + 6] << 8)
	      | (data->buffer.data[off + 7] << 16)
	      | (data->buffer.data[off + 8] << 24);
	return 0;
}

int acpi_resource_get_ext_interrupt(struct acpi_data *data,
                                    uint8_t *flags,
                                    uint32_t *interrupts,
                                    uint8_t *interrupts_count)
{
	size_t length;
	size_t off;
	int ret = acpi_resource_get_entry(data, 0x89, &off, &length);
	if (ret)
		return ret;
	if (length < 6)
	{
		TRACE("extended interrupt length < 6");
		return -EINVAL;
	}
	*flags = data->buffer.data[off];
	uint8_t int_count = data->buffer.data[off + 1];
	if (length < 2u + int_count * 4u)
	{
		TRACE("extended interrupts out of range");
		return -EINVAL;
	}
	for (size_t i = 0; i < int_count && i < *interrupts_count; ++i)
	{
		size_t n = off + 2 + i * 4;
		interrupts[i] = (data->buffer.data[n + 0] << 0)
		              | (data->buffer.data[n + 1] << 8)
		              | (data->buffer.data[n + 2] << 16)
		              | (data->buffer.data[n + 3] << 24);
	}
	*interrupts_count = int_count;
	return 0;
}
