#define ASM_FILE
#include <multiboot2.h>

.section .multiboot.data, "aw"

.align 8
multiboot_header_start:

.long MULTIBOOT2_HEADER_MAGIC
.long MULTIBOOT_ARCHITECTURE_RISCV32
.long multiboot_header_end - multiboot_header_start
.long -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_RISCV32 + (multiboot_header_end - multiboot_header_start))
.short MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS
.short MULTIBOOT_HEADER_TAG_OPTIONAL
.long 12
.long 0
.long 0
.long 0
.short MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST
.short MULTIBOOT_HEADER_TAG_OPTIONAL
.long 32
.long MULTIBOOT_TAG_TYPE_CMDLINE
.long MULTIBOOT_TAG_TYPE_BASIC_MEMINFO
.long MULTIBOOT_TAG_TYPE_MMAP
.long MULTIBOOT_TAG_TYPE_ACPI_NEW

.short MULTIBOOT_HEADER_TAG_END
.short 0
.long 8

multiboot_header_end:

.section .multiboot.bss, "aw", %nobits
.p2align 12
.global dir_page
dir_page:
.skip 4096
tbl_page:
.skip 4096 * 128 /* this has to be handled better, somehow */
tbl_page_last:

tbl_tbl:
.skip 4096

dir_tbl:
.skip 4096

tbl_uart:
.skip 4096

multiboot_buf:
.skip 4096 * 8
multiboot_buf_end:

.macro store_dir dst, src
	la t0, \dst
	la t1, \src
	srli t1, t1, 2
	addi t1, t1, 0x1 /* valid */
	sw t1, (t0)
.endm

.macro store_tbl dst, src
	la t0, \dst
	la t1, \src
	srli t1, t1, 2
	addi t1, t1, 0xF /* executable, writable, readable, valid */
	sw t1, (t0)
.endm

.section .multiboot.text, "ax"
.global _start
.type _start, %function
_start:
	/* disable IRQ */
	csrci sstatus, (1 << 1)

	/* disable paging */
	csrw satp, 0
	sfence.vma

	/* copy multiboot struct */
	mv t0, a2
	la t1, multiboot_buf
	lw t2, (t0)
1:	lb t3, (t0)
	sb t3, (t1)
	add t0, t0, 1
	add t1, t1, 1
	add t2, t2, -1
	bnez t2, 1b

	/* move address into tbl */
	la t0, tbl_page
	li t1, 0x80000000
	la t2, _kernel_end - 0x40000000
	srli t1, t1, 2
	srli t2, t2, 2
2:	ori t3, t1, 0xF /* executable, writable, readable, valid */
	sw t3, (t0)
	addi t0, t0, 4
	addi t1, t1, 1024
	blt t1, t2, 2b

	/* save the last used p2t not to feed l1t more than necessary */
	mv t4, t0

	/* move tbl into dir */
	la t0, dir_page + 0x800
	la t1, dir_page + 0xC00
	la t2, tbl_page
	li t5, 4096
3:	srli t3, t2, 2
	ori t3, t3, 0x1 /* valid */
	sw t3, (t0)
	sw t3, (t1)
	addi t0, t0, 4
	addi t1, t1, 4
	add t2, t2, t5
	blt t2, t4, 3b

	store_tbl tbl_uart,         0x10000000
	store_dir dir_page + 0x100, tbl_uart

	store_tbl tbl_tbl + 0xFFC,  tbl_tbl
	store_dir dir_page + 0xFFC, tbl_tbl

	store_tbl dir_tbl + 0xFFC,  dir_page
	store_dir dir_page + 0xFF8, dir_tbl

	/* set satp & enable paging */
	la t0, dir_page
	srli t0, t0, 12
	li t1, 1 << 31
	or t0, t0, t1
	csrw satp, t0
	sfence.vma

	la t0, multiboot_buf
	li t1, 0x40000000
	add t0, t0, t1
	la t1, multiboot_ptr
	sw t0, (t1)

	la sp, stack_top
	la t0, cpu_boot
	jalr t0

.section .bss, "aw", %nobits
.p2align 12
stack_bottom:
.skip KSTACK_SIZE
stack_top:
