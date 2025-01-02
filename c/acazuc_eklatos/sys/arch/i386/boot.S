#define ASM_FILE
#include <multiboot2.h>
#include "arch/x86/cr.h"

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

.section .multiboot.bss, "aw", %nobits
.global dir_page
.p2align 12
dir_page:
.skip 4096
tbl_page:
.skip 4096 * 128 /* this has to be handled better, somehow */
tbl_page_last:

multiboot_buf:
.skip 4096 * 8
multiboot_buf_end:

.section .multiboot.text, "ax"
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
	mov $0xC0000000, %ecx
	mov $0x3, %eax
	mov $tbl_page, %edi
2:	mov %eax, (%edi)
	add $4096, %eax
	add $4, %edi
	add $4096, %ecx
	cmp $_kernel_end, %ecx
	jb 2b

	/* move kern tables into dir page (high-mem + identity) */
	mov $0xC0000000, %ecx
	mov $tbl_page, %eax
	mov $(dir_page + 0xC00), %edi
	mov $(dir_page + 0x000), %edx
3:	mov %eax, %esi
	add $3, %esi
	mov %esi, (%edi)
	mov %esi, (%edx)
	add $4096, %eax
	add $4, %edi
	add $4, %edx
	add $(1024 * 4096), %ecx
	cmp $_kernel_end, %ecx
	jb 3b

	/* recursive */
	movl $(dir_page + 3), dir_page + 0xFFC

	/* load dir page */
	mov $dir_page, %eax
	mov %eax, %cr3

	/* enable paging (with write-protect) */
	mov %cr0, %eax
	or $(CR0_PG | CR0_WP), %eax
	mov %eax, %cr0

	mov $multiboot_buf, %ebx
	add $0xC0000000, %ebx
	mov %ebx, multiboot_ptr

	mov $stack_top, %esp
	mov $0, %ebp
	call cpu_boot

1:	cli
	hlt
	jmp 1b

.section .bss, "aw", %nobits
.p2align 12
stack_bottom:
.skip KSTACK_SIZE
stack_top:
