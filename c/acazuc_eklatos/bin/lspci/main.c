#include <inttypes.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static const struct
{
	uint16_t vendor;
	uint16_t device;
	const char *name;
} devices_ref[] =
{
	{0x10DE, 0x0BEE, "NVidia GF116 HD Audio Controller"},
	{0x10DE, 0x1251, "NVidia GF116M"},
	{0x10EC, 0x8029, "RTL8029(AS)"},
	{0x10EC, 0x8139, "RTL8139 Gigabit Ethernet Controller"},
	{0x10EC, 0x8168, "RTL8168 Gigabit Ethernet Controller"},
	{0x1234, 0x1111, "QEMU Virtual Video Controller"},
	{0x168C, 0x002B, "AR9285 Wireless Network Adapter"},
	{0x1AF4, 0x1000, "Virtio network device"},
	{0x1AF4, 0x1001, "Virtio block device"},
	{0x1AF4, 0x1002, "Virtio memory balloon"},
	{0x1AF4, 0x1003, "Virtio console"},
	{0x1AF4, 0x1004, "Virtio SCSI"},
	{0x1AF4, 0x1005, "Virtio RNG"},
	{0x1AF4, 0x1009, "Virtio filesystem"},
	{0x1AF4, 0x1001, "Virtio 1.0 network device"},
	{0x1AF4, 0x1042, "Virtio 1.0 block device"},
	{0x1AF4, 0x1043, "Virtio 1.0 console"},
	{0x1AF4, 0x1044, "Virtio 1.0 RNG"},
	{0x1AF4, 0x1045, "Virtio 1.0 memory balloon"},
	{0x1AF4, 0x1048, "Virtio 1.0 SCSI"},
	{0x1AF4, 0x1049, "Virtio 1.0 filesystem"},
	{0x1AF4, 0x1050, "Virtio 1.0 GPU"},
	{0x1AF4, 0x1052, "Virtio 1.0 input"},
	{0x1AF4, 0x1053, "Virtio 1.0 socket"},
	{0x1AF4, 0x105A, "Virtio filesystem"},
	{0x1B36, 0x0008, "QEMU PCIe Host bridge"},
	{0x1B36, 0x000D, "QEMU XHCI Host Controller"},
	{0x1B73, 0x1000, "FL1000G USB 3.0 Host Controller"},
	{0x8086, 0x0101, "Intel 2nd Gen PCIe Root Port"},
	{0x8086, 0x0104, "Intel 2nd Gen DRAM Controller"},
	{0x8086, 0x0697, "Comet Lake ISA Bridge"},
	{0x8086, 0x06A3, "Comet Lake PCH SMBus Controller"},
	{0x8086, 0x06A4, "Comet Lake PCH SPI Controller"},
	{0x8086, 0x06AC, "Comet Lake PCI Express Root Port #21"},
	{0x8086, 0x06B0, "Comet Lake PCI Express Root Port #9"},
	{0x8086, 0x06BD, "Comet Lake PCIe Port #6"},
	{0x8086, 0x06BF, "Comet Lake PCIe Port #8"},
	{0x8086, 0x06C0, "Comet Lake PCI Express Root Port #17"},
	{0x8086, 0x06D2, "Comet Lake SATA AHCI Controller"},
	{0x8086, 0x06E0, "Comet Lake HECI Controller"},
	{0x8086, 0x06ED, "Comet Lake USB 3.1 xHCI Host Controller"},
	{0x8086, 0x06EF, "Comet Lake PCH Shared SRAM"},
	{0x8086, 0x06F9, "Comet Lake PCH Thermal Controller"},
	{0x8086, 0x100E, "82540EM Gigabit Ethernet Controller"},
	{0x8086, 0x10D3, "82574L Gigabit Ethernet Controller"},
	{0x8086, 0x1237, "440FX - 82441FX PMC"},
	{0x8086, 0x1536, "I210 Gigabit Fiber Network Connection"},
	{0x8086, 0x1572, "Ethernet Controller X710 for 10GbE SFP+"},
	{0x8086, 0x15F2, "Ethernet Controller I225-LM"},
	{0x8086, 0x1901, "6th-10th Gen Core Processor PCIe Controller (x16)"},
	{0x8086, 0x1905, "Xeon E3-1200 v5/E3-1500 v5/6th Gen Core Processor PCIe Controller (x8)"},
	{0x8086, 0x1909, "Xeon E3-1200 v5/E3-1500 v5/6th Gen Core Processor PCIe Controller (x4)"},
	{0x8086, 0x1911, "Xeon E3-1200 v5/v6 / E3-1500 v5 / 6th/7th/8th Gen Core Processor Gaussian Mixture Model"},
	{0x8086, 0x1C03, "6 Series/C200 Mobile SATA AHCI Controller"},
	{0x8086, 0x1C10, "6 Series/C200 PCIe Root Port 1"},
	{0x8086, 0x1C12, "6 Series/C200 PCIe Root Port 2"},
	{0x8086, 0x1C16, "6 Series/C200 PCIe Root Port 4"},
	{0x8086, 0x1C1A, "6 Series/C200 PCIe Root Port 6"},
	{0x8086, 0x1C20, "6 Series/C200 HD Audio Controller"},
	{0x8086, 0x1C22, "6 Series/C200 SMBus Controller"},
	{0x8086, 0x1C26, "6 Series/C200 USB EHC #1"},
	{0x8086, 0x1C2D, "6 Series/C200 USB EHC #2"},
	{0x8086, 0x1C3A, "6 Series/C200 MEI Controller #1"},
	{0x8086, 0x1C49, "HM65 Express Chipset LPC Controller"},
	{0x8086, 0x2918, "82801IB (ICH9) LPC Interface Controller"},
	{0x8086, 0x2922, "82801IR/IO/IH (ICH9R/DO/DH) 6 port SATA Controller"},
	{0x8086, 0x2930, "82801I (ICH9 Family) SMBus Controller"},
	{0x8086, 0x2932, "82801I (ICH9 Family) Thermal Subsystem"},
	{0x8086, 0x2934, "82801I (ICH9 Family) USB UHCI Controller #1"},
	{0x8086, 0x2935, "82801I (ICH9 Family) USB UHCI Controller #2"},
	{0x8086, 0x2936, "82801I (ICH9 Family) USB UHCI Controller #3"},
	{0x8086, 0x2937, "82801I (ICH9 Family) USB UHCI Controller #4"},
	{0x8086, 0x2938, "82801I (ICH9 Family) USB UHCI Controller #5"},
	{0x8086, 0x2939, "82801I (ICH9 Family) USB UHCI Controller #6"},
	{0x8086, 0x293A, "82801I (ICH9 Family) USB2 EHCI Controller #1"},
	{0x8086, 0x293C, "82801I (ICH9 Family) USB2 EHCI Controller #2"},
	{0x8086, 0x293E, "82801I (ICH9 Family) HD Audio Controller"},
	{0x8086, 0x29C0, "82G33/G31/P35/P31 Express DRAM Controller"},
	{0x8086, 0x2415, "82801AA AC'97 Audio Controller"},
	{0x8086, 0x24CD, "82801DB/DBM USB2 EHCI Controller"},
	{0x8086, 0x2668, "82801FB HDA Controller"},
	{0x8086, 0x4511, "Elkhart Lake Gaussian and Neural Accelerator"},
	{0x8086, 0x4571, "Elkhart Lake [UHD Graphics Gen11 32EU]"},
	{0x8086, 0x452A, "Elkhart Lake IBECC"},
	{0x8086, 0x4B00, "Elkhart Lake eSPI Controller"},
	{0x8086, 0x4B23, "Elkhart Lake High Density Audio bus interface"},
	{0x8086, 0x4B24, "Elkhart Lake SPI (Flash) Controller"},
	{0x8086, 0x4B38, "Elkhart Lake PCH PCI Express Root Port #0"},
	{0x8086, 0x4B3C, "Elkhart Lake PCIe Root Port #4"},
	{0x8086, 0x4B3E, "Elkhart Lake PCH PCI Express Root Port #6"},
	{0x8086, 0x4B47, "Elkhart Lake Atom SD Controller"},
	{0x8086, 0x4B58, "Elkhart Lake High Density Audio bus interface"},
	{0x8086, 0x4B63, "Elkhart Lake SATA AHCI"},
	{0x8086, 0x4B70, "Elkhart Lake Management Engine Interface"},
	{0x8086, 0x4B7D, "Elkhart Lake Gaussian and Neural Accelerator"},
	{0x8086, 0x4B7F, "Elkhart Lake PMC SRAM"},
	{0x8086, 0x7000, "82371SB PIIX3 ISA"},
	{0x8086, 0x7010, "82371SB PIIX3 IDE"},
	{0x8086, 0x7110, "82371AB/EB/MB PIIX4 ISA"},
	{0x8086, 0x7111, "82371AB/EB/MB PIIX4 IDE"},
	{0x8086, 0x7112, "82371AB/EB/MB PIIX4 USB"},
	{0x8086, 0x7113, "82371AB/EB/MB PIIX4 ACPI"},
	{0x8086, 0x7020, "82371SB PIIX3 USB"},
	{0x8086, 0x9B63, "10th Gen Core Processor Host Bridge/DRAM Registers"},
	{0x8086, 0x9BC8, "CometLake-S GT2 [UHD Graphics 630]"},
};

static const char *dev_name(uint16_t vendor, uint16_t device)
{
	for (size_t i = 0; i < sizeof(devices_ref) / sizeof(*devices_ref); ++i)
	{
		if (devices_ref[i].vendor == vendor
		 && devices_ref[i].device == device)
			return devices_ref[i].name;
	}
	return "";
}

static void usage(const char *progname)
{
	printf("%s [-h]\n", progname);
	printf("-h: show this help\n");
}

int main(int argc, char **argv)
{
	char *line = NULL;
	size_t size = 0;
	FILE *fp = NULL;
	int c;

	while ((c = getopt(argc, argv, "h")) != -1)
	{
		switch (c)
		{
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	fp = fopen("/sys/pci/list", "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: open(/sys/pci/list): %s\n", argv[0],
		        strerror(errno));
		return EXIT_FAILURE;
	}
	while ((getline(&line, &size, fp)) > 0)
	{
		uint16_t group;
		uint8_t bus;
		uint8_t slot;
		uint8_t func;
		uint16_t vendor;
		uint16_t device;
		if (sscanf(line, "%" SCNx16 ":%" SCNx8 ":%" SCNx8 ".%" SCNx8
		                 " %" SCNx16 ":%" SCNx16,
		                 &group, &bus, &slot, &func,
		                 &vendor, &device) != 6)
		{
			fprintf(stderr, "%s: invalid line\n", argv[0]);
			return EXIT_FAILURE;
		}
		printf("%04" PRIx16 ":%02" PRIx8 ":%02" PRIx8 ".%01" PRIx8
		       " %04" PRIx16 ":%04" PRIx16 " %s\n",
		       group, bus, slot, func, vendor, device,
		       dev_name(vendor, device));
	}
	if (ferror(fp))
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
