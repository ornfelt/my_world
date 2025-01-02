ENTRY(_start)

KERNEL_BASE = 0xC0000000;

SECTIONS
{
	_elf_start = .;

	. += 0x1000; /* space for PHDR */

	.multiboot.data BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot.data)
	}

	.multiboot.bss BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot.bss)
	}

	.multiboot.text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot.text)
		*(.multiboot.text.*)
	}

	. = ALIGN(4K);
	. += KERNEL_BASE;

	_kernel_begin = .;

	_text_begin = .;
	.text : AT(ADDR(.text) - KERNEL_BASE)
	{
		*(.text)
		*(.text.*)
	}
	. = ALIGN(4K);
	_text_end = .;

	_rodata_begin = .;
	.rodata : AT(ADDR(.rodata) - KERNEL_BASE)
	{
		*(.rodata*)
	}
	. = ALIGN(4K);
	_rodata_end = .;

	_data_begin = .;
	.data : AT(ADDR(.data) - KERNEL_BASE)
	{
		*.o(.data)
	}
	. = ALIGN(4K);
	_data_end = .;

	_bss_begin = .;
	.bss : AT(ADDR(.bss) - KERNEL_BASE)
	{
		*(COMMON)
		*(.bss)
	}
	. = ALIGN(4K);
	_bss_end = .;

	/* XXX use a symbol here to avoid excessive pages in boot.S */

	.data_rdinit : AT(ADDR(.data_rdinit) - KERNEL_BASE)
	{
		*.tar(.data)
	}

	. = ALIGN(4K);
	_kernel_end = .;
	_kernel_size = _kernel_end - KERNEL_BASE;

	_elf_end = .;
	_elf_size = _elf_end - _elf_start - KERNEL_BASE;
}
