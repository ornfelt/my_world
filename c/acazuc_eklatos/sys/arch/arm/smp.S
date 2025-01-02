.global smp_trampoline
.type smp_trampoline, %function
smp_trampoline:
	/* disable IRQ */
	cpsid aif

	mrc p15, 0, r8, c1, c0, 0
	orr r8, #0x0004 /* cache enable */
	orr r8, #0x0800 /* branch prediction enable */
	orr r8, #0x1000 /* instruction cache */
	and r8, #~0x1 /* disable mmu */
	mcr p15, 0, r8, c1, c0, 0

	/* set translation control */
	mov r8, #0 /* enable translation walk, ttbr0 used for full address space */
	mcr p15, 0, r8, c2, c0, 2

	/* set dacr */
	mov r8, #0x55
	orr r8, #0x5500
	orr r8, #0x550000
	orr r8, #0x55000000
	mcr p15, 0, r8, c3, c0, 0

	/* dsb */
	mov r8, #0
	mcr p15, 0, r8, c7, c10, 5

	/* set ttbr0 */
	ldr r8, =l1t
	add r8, r0
	orr r8, r8, #0x3 /* shareable, inner cacheable */
	mcr p15, 0, r8, c2, c0, 0

	/* enable paging */
	mrc p15, 0, r8, c1, c0, 0
	orr r8, r8, #0x1
	mcr p15, 0, r8, c1, c0, 0

	/* dsb */
	mov r8, #0
	mcr p15, 0, r8, c7, c10, 5

	ldr r8, =g_ncpus
	ldr r8, [r8]
	lsl r8, r8, #2
	ldr r9, =smp_stacks
	add r9, r9, r8
	ldr sp, [r9]
	ldr r8, =cpu_boot
	bx r8
	b .
