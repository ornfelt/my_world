#define ASM_FILE
#include <multiboot2.h>

.section .multiboot.data, "aw"

.align 8
multiboot_header_start:

.long MULTIBOOT2_HEADER_MAGIC
.long MULTIBOOT_ARCHITECTURE_RISCV64
.long multiboot_header_end - multiboot_header_start
.long -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_RISCV64 + (multiboot_header_end - multiboot_header_start))
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

.global satp_page
satp_page:
.skip 4096
ident_dir2_page:
.skip 4096
dir2_page:
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

dir0_page_early:
.skip 4096

multiboot_buf:
.skip 4096 * 8
multiboot_buf_end:

.macro store_dir dst, src
	la t0, \dst
	la t1, \src
	srli t1, t1, 2
	addi t1, t1, 0x1 /* valid */
	sd t1, (t0)
.endm

.macro store_tbl dst, src
	la t0, \dst
	la t1, \src
	srli t1, t1, 2
	addi t1, t1, 0xF /* executable, writable, readable, valid */
	sd t1, (t0)
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

	/* move address into dir0 */
	la t0, dir0_page
	li t1, 0x80000000
	srli t1, t1, 2
	ori t1, t1, 0xF /* executable, writable, readable, valid */
	la t2, dir0_page_end
2:	sd t1, (t0)
	addi t0, t0, 8
	addi t1, t1, 1024
	bne t0, t2, 2b

	/* move dir0 into dir1 */
	la t0, dir1_page
	la t1, dir0_page
	srli t1, t1, 2
	ori t1, t1, 0x1 /* valid */
	la t2, dir0_page_end
	srli t2, t2, 2
3:	sd t1, (t0)
	addi t0, t0, 8
	add t1, t1, 1024
	blt t1, t2, 3b

	store_dir ident_dir2_page + 0x10,  dir1_page
	store_tbl uart_dir0_page,          0x10000000
	store_dir uart_dir1_page + 0x400,  uart_dir0_page
	store_dir ident_dir2_page,         uart_dir1_page
	store_dir satp_page,               ident_dir2_page
	store_dir dir2_page + 0xFF8,       dir1_page
	store_tbl dir0_page_early + 0xFF8, dir0_page_early
	store_dir dir1_page + 0xFF8,       dir0_page_early
	store_dir satp_page + 0xFF8,       dir2_page

	/* set satp & enable paging */
	la t0, satp_page
	srli t0, t0, 12
	li t1, 9 << 60
	or t0, t0, t1
	csrw satp, t0
	sfence.vma

	la t0, multiboot_buf
	li t1, 0xFFFFFFFF40000000
	add t0, t0, t1
	la t1, multiboot_ptr
	sd t0, (t1)

	la sp, stack_top
	la t0, cpu_boot
	jalr t0

.section .bss, "aw", %nobits
.p2align 12
stack_bottom:
.skip KSTACK_SIZE
stack_top:
