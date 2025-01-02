#include "arch/x86/cr.h"

.section .boot.data, "aw"

.align 16
gdt_table:
	.long 0, 0
	.long 0x0000FFFF, 0x00CF9A00
	.long 0x0000FFFF, 0x00CF9200

.align 16
gdt_value:
	.word 23
	.long gdt_table

.section .boot.text, "ax"

.p2align 12

.code16
.global wakeup_trampoline
.type wakeup_trampoline, @function
wakeup_trampoline:
	cli
	cld

	lgdt gdt_value

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
	mov %ax, %ss

	mov $dir_page, %eax
	mov %eax, %cr3

	/* enable paging (with write-protect) */
	mov %cr0, %eax
	or $(CR0_PG | CR0_WP), %eax
	mov %eax, %cr0

	/* XXX restore CPU state */
1:	jmp 1b
