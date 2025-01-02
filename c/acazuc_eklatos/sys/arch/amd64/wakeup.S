#include "arch/x86/msr.h"
#include "arch/x86/cr.h"

.section .boot.bss, "aw", @nobits

.align 4096
wakeup_pml_page:
.skip 4096

.section .boot.data, "aw"

.align 16
gdt32_table:
	.long 0, 0
	.long 0x0000FFFF, 0x00CF9A00
	.long 0x0000FFFF, 0x00CF9200

.align 16
gdt32_value:
	.word 23
	.long gdt32_table

.align 16
gdt64_table:
	.long 0, 0
	.long 0x0000FFFF, 0x00AF9A00
	.long 0x0000FFFF, 0x00AF9200

.align 16
gdt64_value:
	.word 23
	.quad gdt64_table

.section .boot.text, "ax"

.p2align 12

.code16
.global wakeup_trampoline
.type wakeup_trampoline, @function
wakeup_trampoline:
	cli
	cld

	lgdt gdt32_value

	/* enable protected mode */
	mov %cr0, %eax
	or $CR0_PE, %eax
	mov %eax, %cr0
	jmp $0x8, $trampoline32

.code32
trampoline32:
	/* reload segments */
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss

	/* disable paging */
	mov %cr0, %eax
	and $~CR0_PG, %eax
	mov %eax, %cr0

	/* copy pml to wakeup pml */
	mov $pml_page, %esi
	mov $wakeup_pml_page, %edi
	mov $4096, %ecx
	rep movsb

	/* move pdp page into identity pml page */
	movl $(pdp_page_ident + 3), wakeup_pml_page + 0x000

	/* enable PAE */
	mov %cr4, %eax
	or $CR4_PAE, %eax
	mov %eax, %cr4

	/* load PML page */
	mov $wakeup_pml_page, %eax
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

	lgdt gdt64_value

	ljmp $0x8, $reload_cs

reload_cs:
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss

	/* let's go */
	jmp trampoline64

.code64
trampoline64:
	/* XXX restore CPU state */
	jmp trampoline64
