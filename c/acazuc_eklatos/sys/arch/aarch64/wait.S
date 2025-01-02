.global arch_waitq_sleep
.type arch_waitq_sleep, %function
arch_waitq_sleep:
	mrs x0, tpidr_el1
	ldr x0, [x0, #8]
	add x0, x0, #528
	str x19, [x0, #0x98]
	stp x20, x21, [x0, #0xA0]
	stp x22, x23, [x0, #0xB0]
	stp x24, x25, [x0, #0xC0]
	stp x26, x27, [x0, #0xD0]
	stp x28, x29, [x0, #0xE0]
	mov x1, sp
	str x1, [x0, #0xF8]
	str lr, [x0, #0x100]
	mrs x1, nzcv
	add x1, x1, 0x3C5 /* DAIF + EL1 */
	str x1, [x0, #0x108]
	mov x0, 0
	ret

.global arch_waitq_leave
.type arch_waitq_leave, %function
arch_waitq_leave:
	mrs x0, tpidr_el1
	ldr x0, [x0, #8]
	add x0, x0, #528
	ldr x19, [x0, #0x98]
	ldp x20, x21, [x0, #0xA0]
	ldp x22, x23, [x0, #0xB0]
	ldp x24, x25, [x0, #0xC0]
	ldp x26, x27, [x0, #0xD0]
	ldp x28, x29, [x0, #0xE0]
	ldr x1, [x0, #0xF8]
	mov sp, x1
	ldr lr, [x0, #0x100]
	add x1, x1, 0x3C5 /* DAIF + EL1 */
	ldr x1, [x0, #0x108]
	msr nzcv, x1
	mov x0, 1
	ret
