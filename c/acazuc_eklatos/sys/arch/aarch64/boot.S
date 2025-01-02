#define ASM_FILE
#include <multiboot2.h>

.section .multiboot.data, "aw"

.align 8
multiboot_header_start:

.long MULTIBOOT2_HEADER_MAGIC
.long MULTIBOOT_ARCHITECTURE_AARCH64
.long multiboot_header_end - multiboot_header_start
.long -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_AARCH64 + (multiboot_header_end - multiboot_header_start))
.short MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS
.short MULTIBOOT_HEADER_TAG_OPTIONAL
.long 24
.long 0
.long 0
.long 0
.long 0
.short MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST
.short MULTIBOOT_HEADER_TAG_OPTIONAL
.long 24
.long MULTIBOOT_TAG_TYPE_CMDLINE
.long MULTIBOOT_TAG_TYPE_BASIC_MEMINFO
.long MULTIBOOT_TAG_TYPE_MMAP
.long MULTIBOOT_TAG_TYPE_ACPI_NEW
.short MULTIBOOT_HEADER_TAG_RELOCATABLE
.short MULTIBOOT_HEADER_TAG_OPTIONAL
.long 16
.long 0x10000
.long -1
.long 0x10000
.long MULTIBOOT_LOAD_PREFERENCE_LOW
.short MULTIBOOT_HEADER_TAG_END
.short 0
.long 8

multiboot_header_end:

.section .multiboot.bss, "aw", %nobits
.p2align 12

.global ttbr0_page
ttbr0_page:
.skip 4096
ident_dir2_page:
.skip 4096
dir1_page:
.skip 4096
dir0_page:
.skip 4096 * 128
dir0_page_end:

/* XXX should be removed to map from C ? */
uart_dir1_page:
.skip 4096
uart_dir0_page:
.skip 4096

.global ttbr1_page
ttbr1_page:
.skip 4096
dir2_page:
.skip 4096

dir0_page_early:
.skip 4096

multiboot_buf:
.skip 4096 * 8
multiboot_buf_end:

.macro store_pte dst, src, off
	adr x8, \dst
	adr x9, \src
	add x9, x9, #0x603 /* AF | outer sharable | table */
	add x8, x8, #\off
	str x9, [x8]
.endm

.section .multiboot.text, "ax"
.global _start
.type _start, %function
_start:
	/* disable IRQ */
	msr DAIFSet, #0x2

	/* copy multiboot struct */
	mov x8, #0
	mov w8, w1
	mov x9, #0
	adr x9, multiboot_buf
	mov x10, #0
	mov x13, #0
	ldr w13, [x8]
1:	ldrb w11, [x8, x10]
	strb w11, [x9, x10]
	add x10, x10, #1
	cmp x10, x13
	bne 1b

	/* disable paging, in case it was already enabled */
	/* XXX we should put some stable state here (and even before) */
	mrs x8, sctlr_el1
	and x8, x8, #~(1 << 0)
	msr sctlr_el1, x8
	isb

	mrs x8, sctlr_el1
	and x8, x8, #~(1 << 1) /* disable alignment check */
	orr x8, x8, # (1 << 2) /* enable cache */
	and x8, x8, #~(1 << 3) /* disable stack alignment check */
	and x8, x8, #~(1 << 4) /* disable el0 stack alignment check */
	orr x8, x8, # (1 << 12) /* enable instruction cache */
	msr sctlr_el1, x8
	isb

	/* move addresses into dir0 */
	adr x8, dir0_page
	adr x9, _elf_start
	add x9, x9, #0x603 /* AF | outer shareable | table */
	adr x10, dir0_page_end
2:	str x9, [x8]
	add x8, x8, #8
	add x9, x9, #4096
	cmp x8, x10
	bne 2b

	/* move dir0 into dir1 */
	adr x8, dir1_page
	adr x9, dir0_page
	add x9, x9, #0x603 /* AF | outer shareable | table */
	adr x10, dir0_page_end
3:	str x9, [x8]
	add x8, x8, #8
	add x9, x9, #4096
	cmp x9, x10
	blt 3b

	/* move uart address into uart dir0 */
	adr x8, uart_dir0_page
	mov x9, 0x09000000
	add x9, x9, #0x603 /* AF | outer sharable | table */
	str x9, [x8]

	store_pte ident_dir2_page, dir1_page,       0x008 /* XXX offset based on paddr */
	store_pte uart_dir1_page,  uart_dir0_page,  0x240
	store_pte ident_dir2_page, uart_dir1_page,  0x000
	store_pte ttbr0_page,      ident_dir2_page, 0x000 /* XXX offset based on paddr */
	store_pte dir2_page,       dir1_page,       0xFF8
	store_pte dir0_page_early, dir0_page_early, 0xFF8
	store_pte dir1_page,       dir0_page_early, 0xFF8
	store_pte ttbr1_page,      dir2_page,       0xFF8

	/* set memory attributes */
	/* outer write-back non-transient, rw, inner write-back non-transient */
	/* outer write-through non-transient, rw, inner write-through non-transient */
	/* outer uncacheable, rw, inner uncacheable */
	/* unused */
	/* device-nGnRnE */
	/* device-GRE */
	movz x8, #0xBBFF
	movk x8, #0x0044, LSL 16
	movk x8, #0x0C00, LSL 32
	msr mair_el1, x8

	/* set translation control */
	movz x8, #0x1510         /* 4kb granule, outer shareable, outer WC RA WA cacheable, inner WB RA WA cacheable, enable translation walk */
	movk x8, #0xA510, LSL 16 /* 4kb granule, outer shareable, outer WC RA WA cacheable, inner WB RA WA cacheable, enable translation walk */
	movk x8, #0x0005, LSL 32 /* 48 bits virtual, 40 bits physical */
	msr tcr_el1, x8
	isb

	/* set ttbr0 */
	adr x8, ttbr0_page
	msr ttbr0_el1, x8

	/* set ttbr1 */
	adr x8, ttbr1_page
	msr ttbr1_el1, x8

	/* enable paging */
	mrs x8, sctlr_el1
	orr x8, x8, #1
	msr sctlr_el1, x8
	isb

.Lref:	adr x8, 0
	ldr x9, =kern_paddr
	ldr x6, =.Lref
	ldr x7, =_elf_start
	sub x7, x6, x7
	sub x8, x8, x7
	str x8, [x9]
	mov x10, x8

	adr x8, multiboot_buf
	mov x7, #0xFFFFFFFFC0000000
	sub x7, x7, x10
	add x8, x8, x7
	ldr x9, =multiboot_ptr
	str x8, [x9]

	ldr x30, =stack_top
	mov sp, x30
	ldr x9, =cpu_boot
	br x9

.section .bss, "aw", %nobits
.p2align 12
stack_bottom:
.skip KSTACK_SIZE
stack_top:
