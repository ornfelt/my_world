.global smp_trampoline
.type smp_trampoline, %function
smp_trampoline:
	/* disable IRQ */
	msr DAIFSet, #0x2

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
	ldr x8, =ttbr0_page
	add x8, x8, x0
	msr ttbr0_el1, x8

	/* set ttbr1 */
	ldr x8, =ttbr1_page
	add x8, x8, x0
	msr ttbr1_el1, x8

	/* enable paging */
	mrs x8, sctlr_el1
	orr x8, x8, #1
	msr sctlr_el1, x8
	isb

	ldr x29, =g_ncpus
	ldr x29, [x29]
	lsl x29, x29, #3
	ldr x30, =smp_stacks
	add x30, x30, x29
	ldr x30, [x30]
	mov sp, x30
	ldr x9, =cpu_boot
	br x9
