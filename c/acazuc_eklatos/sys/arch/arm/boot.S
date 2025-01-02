#define ASM_FILE
#include <multiboot2.h>

.section .multiboot.data, "aw"

.align 8
multiboot_header_start:

.long MULTIBOOT2_HEADER_MAGIC
.long MULTIBOOT_ARCHITECTURE_ARM32
.long multiboot_header_end - multiboot_header_start
.long -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_ARM32 + (multiboot_header_end - multiboot_header_start))
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
.long 0x100000
.long MULTIBOOT_LOAD_PREFERENCE_LOW
.short MULTIBOOT_HEADER_TAG_END
.short 0
.long 8

multiboot_header_end:

.section .multiboot.bss, "aw", %nobits

.p2align 14
.global l1t
l1t:
.skip 4096 * 4

.p2align 12
l2t:
.skip 1024 * 128
l2t_end:

.p2align 10
l2t_l1t: /* l2t referencing l1t pages */
.skip 1024

.p2align 12
l2t_l2t: /* self-referencing l2t */
.skip 1024 * 4

.p2align 10
l2t_uart:
.skip 1024

multiboot_buf:
.skip 4096 * 8
multiboot_buf_end:

.macro store_dir dst, src
	ldr r8, =\src
	ldr r9, =\dst
	add r8, r4
	add r9, r4
	add r8, #0x1 /* present */
	str r8, [r9]
.endm

.macro store_tbl dst, src
	ldr r8, =\src
	ldr r9, =\dst
	add r8, r4
	add r9, r4
	add r8, #0x400 /* shareable */
	add r8, #0x01E /* AP(1), cacheable, buffereable, present */
	str r8, [r9]
.endm

.macro store_tbl_direct dst, src
	ldr r8, =\src
	ldr r9, =\dst
	add r9, r4
	add r8, #0x400 /* shareable */
	add r8, #0x01E /* AP(1), cacheable, buffereable, present */
	str r8, [r9]
.endm

.section .multiboot.text, "ax"
.global _start
.type _start, %function
_start:
	/* disable IRQ */
	cpsid aif

.Lref:	mov r8, pc
	ldr r7, =.Lref
	ldr r6, =_elf_start
	sub r7, r6
	sub r8, r7
	mov r7, #8
	sub r4, r8, r7

	/* copy multiboot struct */
	mov r8, #0
	mov r8, r1
	mov r9, #0
	ldr r9, =multiboot_buf
	add r9, r4
	mov r10, #0
	mov r13, #0
	ldr r13, [r8]
1:	ldrb r11, [r8, r10]
	strb r11, [r9, r10]
	add r10, #1
	cmp r10, r13
	bne 1b

	mrc p15, 0, r8, c1, c0, 0
	orr r8, #0x0004 /* cache enable */
	orr r8, #0x0800 /* branch prediction enable */
	orr r8, #0x1000 /* instruction cache */
	and r8, #~0x1 /* disable mmu */
	mcr p15, 0, r8, c1, c0, 0

	/* kernel address into l2t */
	ldr r8, =l2t
	add r8, r4
	mov r9, r4
	ldr r10, =_kernel_end
	sub r10, #0xC0000000
	add r10, r4
	mov r11, #0x41E /* shareable, AP(1), cacheable, bufferable, present */
2:	orr r12, r9, r11
	str r12, [r8]
	add r8, #4
	add r9, #4096
	cmp r9, r10
	blt 2b

	/* save the last used p2t not to feed l1t more than necessary */
	mov r10, r8

	/* l2t into l1t */
	ldr r8, =l1t
	ldr r9, =l2t
	add r8, r4
	add r9, r4
	add r11, r8, #0xC00 * 4
	mov r7, r4, LSR #20
	add r8, r7, LSL #2
3:	orr r12, r9, #0x1 /* present */
	str r12, [r8]
	str r12, [r11]
	add r8, #4
	add r11, #4
	add r9, #1024
	cmp r9, r10
	blt 3b

	store_tbl l2t_l1t + 0x3F0, l1t + 4096 * 0
	store_tbl l2t_l1t + 0x3F4, l1t + 4096 * 1
	store_tbl l2t_l1t + 0x3F8, l1t + 4096 * 2
	store_tbl l2t_l1t + 0x3FC, l1t + 4096 * 3
	store_dir l1t + 0xFFB * 4, l2t_l1t

	store_tbl l2t_l2t + 0xFFC, l2t_l2t
	store_dir l1t + 0xFFC * 4, l2t_l2t + 1024 * 0
	store_dir l1t + 0xFFD * 4, l2t_l2t + 1024 * 1
	store_dir l1t + 0xFFE * 4, l2t_l2t + 1024 * 2
	store_dir l1t + 0xFFF * 4, l2t_l2t + 1024 * 3

	store_tbl_direct l2t_uart, 0x09000000
	store_dir l1t + 0x090 * 4, l2t_uart

	/* set translation control */
	mov r8, #0 /* enable translation walk, ttbr0 used for full address space */
	mcr p15, 0, r8, c2, c0, 2

	/* dsb */
	mov r8, #0
	mcr p15, 0, r8, c7, c10, 5

	/* set ttbr0 */
	ldr r8, =l1t
	add r8, r4
	orr r8, r8, #0x3 /* shareable, inner cacheable */
	mcr p15, 0, r8, c2, c0, 0

	/* enable paging */
	mrc p15, 0, r8, c1, c0, 0
	orr r8, r8, #0x1
	mcr p15, 0, r8, c1, c0, 0

	/* dsb */
	mov r8, #0
	mcr p15, 0, r8, c7, c10, 5

	ldr r9, =kern_paddr
	str r4, [r9]

	ldr r8, =multiboot_buf
	add r8, r4
	ldr r9, =multiboot_ptr
	str r8, [r9]

	ldr sp, =stack_top
	ldr r8, =cpu_boot
	bx r8
	b .

.section .bss, "aw", %nobits
.p2align 12
stack_bottom:
.skip KSTACK_SIZE
stack_top:
