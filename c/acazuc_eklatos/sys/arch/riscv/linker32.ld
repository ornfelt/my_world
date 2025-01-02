ENTRY(_start)

SECTIONS
{
	. = 0x80100000;

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
	. += 0x40000000;

	_kernel_begin = .;

	_text_begin = .;
	.text : AT(ADDR(.text) - 0x40000000)
	{
		*(.text)
		*(.text.*)
	}
	. = ALIGN(4K);
	_text_end = .;

	_rodata_begin = .;
	.rodata : AT(ADDR(.rodata) - 0x40000000)
	{
		*(.rodata*)
	}
	. = ALIGN(4K);
	_rodata_end = .;

	_data_begin = .;
	.data : AT(ADDR(.data) - 0x40000000)
	{
		*.o(.data)
	}
	. = ALIGN(4K);
	_data_end = .;

	_bss_begin = .;
	.bss : AT(ADDR(.bss) - 0x40000000)
	{
		*(COMMON)
		*(.bss)
	}
	. = ALIGN(4K);
	_bss_end = .;

	/* XXX use a symbol here to avoid excessive pages in boot.S */

	.data_rdinit : AT(ADDR(.data_rdinit) - 0x40000000)
	{
		*.tar(.data)
	}

	. = ALIGN(4K);
	_kernel_end = .;
}
