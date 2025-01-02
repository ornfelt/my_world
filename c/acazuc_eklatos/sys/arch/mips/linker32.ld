ENTRY(_start)

SECTIONS
{
	. = 0x80000000;

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

	_kernel_begin = .;

	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.text)
		*(.text.*)
	}

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata*)
	}

	.data BLOCK(4K) : ALIGN(4K)
	{
		*.o(.data)
	}

	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}

	/* XXX use a symbol here to avoid excessive pages in boot.S */

	.data_rdinit BLOCK(4K) : ALIGN(4K)
	{
		*.tar(.data)
	}

	. = ALIGN(4K);
	_kernel_end = .;
}
