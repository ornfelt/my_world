.macro save_ctx eln
	sub sp, sp, #16
	stp x0, x1, [sp]
	mrs x0, tpidr_el1
	ldr x0, [x0, #8]
	add x0, x0, #528
	stp x2 , x3 , [x0, #0x10]
	stp x4 , x5 , [x0, #0x20]
	stp x6 , x7 , [x0, #0x30]
	stp x8 , x9 , [x0, #0x40]
	stp x10, x11, [x0, #0x50]
	stp x12, x13, [x0, #0x60]
	stp x14, x15, [x0, #0x70]
	stp x16, x17, [x0, #0x80]
	stp x18, x19, [x0, #0x90]
	stp x20, x21, [x0, #0xA0]
	stp x22, x23, [x0, #0xB0]
	stp x24, x25, [x0, #0xC0]
	stp x26, x27, [x0, #0xD0]
	stp x28, x29, [x0, #0xE0]
.if \eln == 1
	add x1, sp, #16
.else
	mrs x1, sp_el\eln
.endif
	stp x30, x1, [x0, #0xF0]
	mrs x1, elr_el1
	mrs x2, spsr_el1
	stp x1, x2, [x0, #0x100]
	/* saved x0 / x1 */
	ldp x2 , x3 , [sp]
	stp x2 , x3 , [x0, #0x00]
.endm

.global context_switch
.type context_switch, %function
context_switch:
	add x0, x0, #528
	ldp x2, x3, [x0, #0x100]
	ldr x5, [x0, #0xF8]
	ands x4, x3, #0xF
	b.ne .Lsp_el1
	msr sp_el0, x5
	mov sp, x1
	b .Lafter_sp
.Lsp_el1:
	mov sp, x5
.Lafter_sp:
	msr elr_el1, x2
	msr spsr_el1, x3
	ldr x30, [x0, #0xF0]
	ldp x28, x29, [x0, #0xE0]
	ldp x26, x27, [x0, #0xD0]
	ldp x24, x25, [x0, #0xC0]
	ldp x22, x23, [x0, #0xB0]
	ldp x20, x21, [x0, #0xA0]
	ldp x18, x19, [x0, #0x90]
	ldp x16, x17, [x0, #0x80]
	ldp x14, x15, [x0, #0x70]
	ldp x12, x13, [x0, #0x60]
	ldp x10, x11, [x0, #0x50]
	ldp x8 , x9 , [x0, #0x40]
	ldp x6 , x7 , [x0, #0x30]
	ldp x4 , x5 , [x0, #0x20]
	ldp x2 , x3 , [x0, #0x10]
	ldp x0 , x1 , [x0, #0x00]
	eret

.p2align 11
.global trap_vector
trap_vector:

.p2align 7
	/* synchronous exception sp_el0 */
	b .

.p2align 7
	/* irq sp_el0 */
	b .

.p2align 7
	/* fiq sp_el0 */
	b .

.p2align 7
	/* serror sp_el0 */
	b .

.p2align 7
	/* synchronous exception el1 */
	save_ctx 1
	mov x0, 1
	mrs x1, esr_el1
	b aarch64_trap_handle

.p2align 7
	/* irq el1 */
	save_ctx 1
	mov x0, 2
	mrs x1, esr_el1
	b aarch64_trap_handle

.p2align 7
	/* fiq el1 */
	save_ctx 1
	mov x0, 3
	mrs x1, esr_el1
	b aarch64_trap_handle

.p2align 7
	/* serror el1 */
	save_ctx 1
	mov x0, 4
	mrs x1, esr_el1
	b aarch64_trap_handle

.p2align 7
	/* synchronous exception el0 */
	save_ctx 0
	mov x0, 5
	mrs x1, esr_el1
	b aarch64_trap_handle

.p2align 7
	/* irq el0 */
	save_ctx 0
	mov x0, 6
	mrs x1, esr_el1
	b aarch64_trap_handle

.p2align 7
	/* fiq el0 */
	save_ctx 0
	mov x0, 7
	mrs x1, esr_el1
	b aarch64_trap_handle

.p2align 7
	/* serror el0 */
	save_ctx 0
	mov x0, 8
	mrs x1, esr_el1
	b aarch64_trap_handle

.p2align 7
	/* sychronous exception el0 aarch32 */
	b .

.p2align 7
	/* irq el0 aarch32 */
	b .

.p2align 7
	/* fiq el0 aarch32 */
	b .

.p2align 7
	/* serror el0 aarch32 */
	b .
