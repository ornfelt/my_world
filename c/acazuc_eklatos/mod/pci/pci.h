#ifndef PCI_H
#define PCI_H

#include <queue.h>
#include <types.h>
#include <irq.h>
#include <mem.h>

#define PCI_CLASS_UNCLASSIFIED 0x0
#define PCI_CLASS_MASS_STORAGE 0x1
#define PCI_CLASS_NETWORK      0x2
#define PCI_CLASS_DISPLAY      0x3
#define PCI_CLASS_MULTIMEDIA   0x4
#define PCI_CLASS_MEMORY       0x5
#define PCI_CLASS_BRIDGE       0x6
#define PCI_CLASS_SIMPLE_COM   0x7
#define PCI_CLASS_BASE_SYSTEM  0x8
#define PCI_CLASS_INPUT_DEVICE 0x9
#define PCI_CLASS_DOCKING      0xA
#define PCI_CLASS_PROCESSOR    0xB
#define PCI_CLASS_SERIAL_BUS   0xC
#define PCI_CLASS_WIRELESS     0xD
#define PCI_CLASS_INTELLIGENT  0xE
#define PCI_CLASS_SATELLITE    0xF
#define PCI_CLASS_ENCRYPTION   0x10
#define PCI_CLASS_SIGNAL       0x11
#define PCI_CLASS_ACCELERATOR  0x12

#define PCI_SUBCLASS_UNCLASSIFIED_NONVGA 0x0
#define PCI_SUBCLASS_UNCLASSIFIED_VGA    0x1

#define PCI_SUBCLASS_MASS_STORAGE_SCSI   0x0
#define PCI_SUBCLASS_MASS_STORAGE_IDE    0x1
#define PCI_SUBCLASS_MASS_STORAGE_FLOPPY 0x2
#define PCI_SUBCLASS_MASS_STORAGE_IPI    0x3
#define PCI_SUBCLASS_MASS_STORAGE_RAID   0x4
#define PCI_SUBCLASS_MASS_STORAGE_ATA    0x5
#define PCI_SUBCLASS_MASS_STORAGE_SATA   0x6
#define PCI_SUBCLASS_MASS_STORAGE_SAS    0x7
#define PCI_SUBCLASS_MASS_STORAGE_NVM    0x8

#define PCI_SUBCLASS_NETWORK_ETHERNET   0x0
#define PCI_SUBCLASS_NETWORK_TOKENRING  0x1
#define PCI_SUBCLASS_NETWORK_FDDI       0x2
#define PCI_SUBCLASS_NETWORK_ATM        0x3
#define PCI_SUBCLASS_NETWORK_ISDN       0x4
#define PCI_SUBCLASS_NETWORK_WORLDFIP   0x5
#define PCI_SUBCLASS_NETWORK_PICMG      0x6
#define PCI_SUBCLASS_NETWORK_INFINIBAND 0x7
#define PCI_SUBCLASS_NETWORK_FABRIC     0x8

#define PCI_SUBCLASS_DISPLAY_VGA 0x0
#define PCI_SUBCLASS_DISPLAY_XGA 0x1
#define PCI_SUBCLASS_DISPLAY_3D  0x2

#define PCI_SUBCLASS_MULTIMEDIA_VIDEO     0x0
#define PCI_SUBCLASS_MULTIMEDIA_AUDIO     0x1
#define PCI_SUBCLASS_MULTIMEDIA_TELEPHONE 0x2
#define PCI_SUBCLASS_MULTIMEDIA_AUDIODEV  0x3

#define PCI_SUBCLASS_MEMORY_RAM   0x0
#define PCI_SUBCLASS_MEMORY_FLASH 0x1

#define PCI_SUBCLASS_BRIDGE_HOST       0x0
#define PCI_SUBCLASS_BRIDGE_ISA        0x1
#define PCI_SUBCLASS_BRIDGE_EISA       0x2
#define PCI_SUBCLASS_BRIDGE_MCA        0x3
#define PCI_SUBCLASS_BRIDGE_PCI        0x4
#define PCI_SUBCLASS_BRIDGE_PCMCIA     0x5
#define PCI_SUBCLASS_BRIDGE_NUBUS      0x6
#define PCI_SUBCLASS_BRIDGE_CARDBUS    0x7
#define PCI_SUBCLASS_BRIDGE_RACEWAY    0x8
#define PCI_SUBCLASS_BRIDGE_PCI2       0x9
#define PCI_SUBCLASS_BRIDGE_INFINIBAND 0xA

#define PCI_SUBCLASS_SERIAL_BUS_FIREWIRE   0x0
#define PCI_SUBCLASS_SERIAL_BUS_ACCESS     0x1
#define PCI_SUBCLASS_SERIAL_BUS_SSA        0x2
#define PCI_SUBCLASS_SERIAL_BUS_USB        0x3
#define PCI_SUBCLASS_SERIAL_BUS_FIBRE      0x4
#define PCI_SUBCLASS_SERIAL_BUS_SMBUS      0x5
#define PCI_SUBCLASS_SERIAL_BUS_INFINIBAND 0x6
#define PCI_SUBCLASS_SERIAL_BUS_IPMI       0x7
#define PCI_SUBCLASS_SERIAL_BUS_SERCOS     0x8
#define PCI_SUBCLASS_SERIAL_BUS_CANBUS     0x9

#define PCI_PROGIF_SERIAL_BUS_USB_UHCI 0x00
#define PCI_PROGIF_SERIAL_BUS_USB_OHCI 0x10
#define PCI_PROGIF_SERIAL_BUS_USB_EHCI 0x20
#define PCI_PROGIF_SERIAL_BUS_USB_XHCI 0x30

#define PCI_CAP_NULL   0x00
#define PCI_CAP_PM     0x01
#define PCI_CAP_AGP    0x02
#define PCI_CAP_VPD    0x03
#define PCI_CAP_SLOTID 0x04
#define PCI_CAP_MSI    0x05
#define PCI_CAP_CHSWP  0x06
#define PCI_CAP_PCIX   0x07
#define PCI_CAP_HT     0x08
#define PCI_CAP_VNDR   0x09
#define PCI_CAP_DBG    0x0A
#define PCI_CAP_CCRC   0x0B
#define PCI_CAP_SHPC   0x0C
#define PCI_CAP_SSVID  0x0D
#define PCI_CAP_AGP3   0x0E
#define PCI_CAP_SECDEV 0x0F
#define PCI_CAP_EXP    0x10
#define PCI_CAP_MSIX   0x11
#define PCI_CAP_SATA   0x12
#define PCI_CAP_AF     0x13
#define PCI_CAP_EA     0x14
#define PCI_CAP_FBP    0x15

struct pci_msix;
struct pci_map;
struct node;

union pci_header
{
	struct
	{
		uint16_t vendor;
		uint16_t device;
		uint16_t command;
		uint16_t status;
		uint8_t revision;
		uint8_t progif;
		uint8_t subclass;
		uint8_t class;
		uint8_t cacheline;
		uint8_t latency;
		uint8_t headertype;
		uint8_t bist;
	};
	struct
	{
		uint32_t v0;
		uint32_t v4;
		uint32_t v8;
		uint32_t vC;
	};
};

struct pci_header0
{
	union
	{
		struct
		{
			uint32_t bar0;
			uint32_t bar1;
			uint32_t bar2;
			uint32_t bar3;
			uint32_t bar4;
			uint32_t bar5;
			uint32_t cis_pointer;
			uint16_t subsystem_vendor_id;
			uint16_t subsystem_id;
			uint32_t rom_bar;
			uint8_t capabilities;
			uint8_t reserved1;
			uint16_t reserved2;
			uint32_t reserved3;
			uint8_t int_line;
			uint8_t int_pin;
			uint8_t min_grant;
			uint8_t max_latency;
		};
		struct
		{
			uint32_t v0;
			uint32_t v1;
			uint32_t v2;
			uint32_t v3;
			uint32_t v4;
			uint32_t v5;
			uint32_t v6;
			uint32_t v7;
			uint32_t v8;
			uint32_t v9;
			uint32_t vA;
			uint32_t vB;
		};
	};
};

struct pci_header1
{
	union
	{
		struct
		{
			uint32_t bar0;
			uint32_t bar1;
			uint8_t primary_bus;
			uint8_t secondary_bus;
			uint8_t subordinate_bus;
			uint8_t secondary_latency;
			uint8_t io_base;
			uint8_t io_limit;
			uint16_t secondary_status;
			uint16_t memory_base;
			uint16_t memory_limit;
			uint16_t prefetchable_base;
			uint16_t prefetchable_limit;
			uint32_t prefetchable_base_upper;
			uint32_t prefetchable_limit_upper;
			uint16_t io_base_upper;
			uint16_t io_limit_upper;
			uint8_t capability_pointer;
			uint8_t reserved1;
			uint16_t reserved2;
			uint32_t expansion_rom_addr;
			uint8_t int_line;
			uint8_t int_pin;
			uint16_t bridge_control;
		};
		struct
		{
			uint32_t v0;
			uint32_t v1;
			uint32_t v2;
			uint32_t v3;
			uint32_t v4;
			uint32_t v5;
			uint32_t v6;
			uint32_t v7;
			uint32_t v8;
			uint32_t v9;
			uint32_t vA;
			uint32_t vB;
			uint32_t vC;
			uint32_t vD;
			uint32_t vE;
			uint32_t vF;
		};
	};
};

union pci_cap_msi
{
	struct
	{
		uint8_t id;
		uint8_t next_pointer;
		uint16_t control;
		uint32_t addr_low;
		uint32_t addr_high;
		uint16_t data;
		uint16_t reserved;
	};
	struct
	{
		uint32_t v0;
		uint32_t v4;
		uint32_t v8;
		uint32_t vC;
	};
};

union pci_cap_msix
{
	struct
	{
		uint8_t id;
		uint8_t next_pointer;
		uint16_t control;
		uint32_t table_offset;
		uint32_t pending_offset;
	};
	struct
	{
		uint32_t v0;
		uint32_t v4;
		uint32_t v8;
	};
};

union pci_cap_virtio
{
	struct
	{
		uint8_t id;
		uint8_t next_pointer;
		uint8_t cap_len;
		uint8_t cfg_type;
		uint8_t bar;
		uint8_t padding[3];
		uint32_t offset;
		uint32_t length;
	};
	struct
	{
		uint32_t v0;
		uint32_t v4;
		uint32_t v8;
		uint32_t vC;
	};
};

union pci_cap_pm
{
	struct
	{
		uint8_t id;
		uint8_t next_point;
		uint16_t capabilities;
		uint16_t status_control;
		uint16_t unimplement;
	};
	struct
	{
		uint32_t v0;
		uint32_t v4;
	};
};

struct pci_device
{
	union pci_header header;
	union
	{
		struct pci_header0 header0;
		struct pci_header1 header1;
	};
	uint32_t base;
	uint16_t group;
	uint8_t bus;
	uint8_t slot;
	uint8_t func;
	struct page ecam_page;
	uint8_t *ecam;
	struct node *sysfs_node;
	struct pci_msix *msix;
	TAILQ_ENTRY(pci_device) chain;
};

typedef int (*pci_probe_t)(struct pci_device *device, void *userdata);

int pci_find_capability(struct pci_device *device, uint8_t id, uint8_t *ptr);
int pci_register_irq(struct pci_device *device, irq_fn_t fn, void *userdata,
                     struct irq_handle *handler);
void pci_unregister_irq(struct pci_device *device, struct irq_handle *handle);
int pci_map(size_t addr, size_t size, size_t offset, struct pci_map **mapp);
int pci_map_bar(struct pci_device *device, size_t bar, size_t size,
                size_t offset, struct pci_map **mapp);
void pci_unmap(struct pci_device *device, struct pci_map *map);
void pci_probe(uint16_t vendor, uint16_t device, pci_probe_t probe,
               void *userdata);
void pci_probe_progif(uint8_t class, uint8_t subclass, uint8_t progif,
                      pci_probe_t probe, void *userdata);

uint8_t pci_r8(struct pci_map *map, uint32_t off);
uint16_t pci_r16(struct pci_map *map, uint32_t off);
uint32_t pci_r32(struct pci_map *map, uint32_t off);
uint64_t pci_r64(struct pci_map *map, uint64_t off);
void pci_r8v(struct pci_map *map, uint32_t off, uint8_t *data, size_t count);
void pci_r16v(struct pci_map *map, uint32_t off, uint16_t *data, size_t count);
void pci_r32v(struct pci_map *map, uint32_t off, uint32_t *data, size_t count);
void pci_r64v(struct pci_map *map, uint32_t off, uint64_t *data, size_t count);
void pci_w8(struct pci_map *map, uint32_t off, uint8_t v);
void pci_w16(struct pci_map *map, uint32_t off, uint16_t v);
void pci_w32(struct pci_map *map, uint32_t off, uint32_t v);
void pci_w64(struct pci_map *map, uint32_t off, uint64_t v);
void pci_w8v(struct pci_map *map, uint32_t off, const uint8_t *data, size_t count);
void pci_w16v(struct pci_map *map, uint32_t off, const uint16_t *data, size_t count);
void pci_w32v(struct pci_map *map, uint32_t off, const uint32_t *data, size_t count);
void pci_w64v(struct pci_map *map, uint32_t off, const uint64_t *data, size_t count);
uint32_t pci_dev_read(struct pci_device *device, uint32_t off);
void pci_dev_write(struct pci_device *device, uint32_t off, uint32_t val);

#endif
