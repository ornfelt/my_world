#define ASM_FILE
#include <multiboot2.h>
#include "arch/x86/cr.h"
#include "arch/x86/msr.h"

.section .multiboot.data, "aw"

.align 8
multiboot_header_start:

.long MULTIBOOT2_HEADER_MAGIC
.long MULTIBOOT_ARCHITECTURE_I386
.long multiboot_header_end - multiboot_header_start
.long -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + (multiboot_header_end - multiboot_header_start))

#if 1
.word MULTIBOOT_HEADER_TAG_FRAMEBUFFER
.word MULTIBOOT_HEADER_TAG_OPTIONAL
.long 20
.long 1280
.long 1024
.long 32
.long 0
.long 0
#else
.word MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS
.word MULTIBOOT_HEADER_TAG_OPTIONAL
.long 12
.long 0
.long 0
.long 0
#endif

.word MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST
.word MULTIBOOT_HEADER_TAG_OPTIONAL
.long 32
.long MULTIBOOT_TAG_TYPE_CMDLINE
.long MULTIBOOT_TAG_TYPE_BASIC_MEMINFO
.long MULTIBOOT_TAG_TYPE_MMAP
.long MULTIBOOT_TAG_TYPE_FRAMEBUFFER
.long MULTIBOOT_TAG_TYPE_ACPI_NEW

.word MULTIBOOT_HEADER_TAG_END
.word 0
.long 8

multiboot_header_end:

.p2align 4
gdt_table:
	.long 0, 0
	.long 0x0000FFFF, 0x00AF9A00
	.long 0x0000FFFF, 0x00AF9200

.p2align 4
gdt_value:
	.word 23
	.quad gdt_table

.section .multiboot.bss, "aw", %nobits
.p2align 12
.global pml_page
pml_page:
.skip 4096
pdp_page:
.skip 4096
.global pdp_page_ident
pdp_page_ident:
.skip 4096
dir_page:
.skip 4096
tbl_page:
.skip 4096 * 512
tbl_page_last:
dir_page_early:
.skip 4096
tbl_page_early:
.skip 4096

multiboot_buf:
.skip 4096 * 8
multiboot_buf_end:

.section .multiboot.text, "ax"
.code32
.global _start
.type _start, %function

_start:
	cli

	/* copy multiboot struct */
	mov %ebx, %esi
	mov $multiboot_buf, %edi
	mov (%ebx), %ecx
	rep movsb

	/* move address into kern tables */
	mov $0x103, %eax
	mov $tbl_page, %edi
2:	mov %eax, (%edi)
	add $8, %edi
	add $4096, %eax
	cmp $tbl_page_last, %edi
	jb 2b

	/* move kern tables into dir page */
	mov $tbl_page, %eax
	mov $dir_page, %edi
3:	mov %eax, %esi
	add $3, %esi
	mov %esi, (%edi)
	add $8, %edi
	add $4096, %eax
	cmp $tbl_page_last, %eax
	jb 3b

	movl $(tbl_page_early + 3), tbl_page_early + 0xFF8
	movl $(tbl_page_early + 3), dir_page_early + 0xFF8
	movl $(dir_page_early + 3), pdp_page + 0xFF8
	movl $(dir_page + 3), pdp_page + 0xFF0
	movl $(pdp_page + 3), pml_page + 0xFF8
	movl $(dir_page + 3), pdp_page_ident + 0x000
	movl $(pdp_page_ident + 3), pml_page + 0x000
	movl $(pdp_page_ident + 3), pml_page + 0x800

	/* disable paging */
	mov %cr0, %eax
	and $~CR0_PG, %eax
	mov %eax, %cr0

	/* enable PAE */
	mov %cr4, %eax
	or $CR4_PAE, %eax
	mov %eax, %cr4

	/* load PML page */
	mov $pml_page, %eax
	mov %eax, %cr3

	/* enable long mode & no-exec */
	mov $MSR_EFER, %ecx
	rdmsr
	or $(EFER_LME | EFER_NXE), %eax
	wrmsr

	/* enable paging (with write-protect) */
	mov %cr0, %eax
	or $(CR0_PG | CR0_WP), %eax
	mov %eax, %cr0

	lgdt gdt_value

	ljmp $0x8, $reload_cs

.code64
reload_cs:
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss

	movabs $multiboot_buf, %rbx
	add $0xFFFFFFFF80000000, %rbx
	mov %rbx, multiboot_ptr

	movabs $stack_top, %rsp
	mov $0, %rbp
	call cpu_boot

.section .bss, "aw", %nobits
.p2align 12
stack_bottom:
.skip KSTACK_SIZE
stack_top:
