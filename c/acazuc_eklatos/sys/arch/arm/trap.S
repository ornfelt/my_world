#include <arch/csr.h>

.global context_switch
.type context_switch, %function
context_switch:
	add r0, #264
	ldr r8, [r0, #0x40]
	msr spsr, r8
	and r9, r8, #0x1F
	cmp r9, #0x10
	beq .Lswitch_usr
	ldm r0, {r0-pc}

.Lswitch_usr:
	mov sp, r1
	ldr lr, [r0, #0x3C]
	ldm r0, {r0-lr}^
	movs pc, lr

.type setup_trap_stack, %function
.global setup_trap_stack
setup_trap_stack:
	cpsid aif, #PSR_M_FIQ
	mov sp, r0
	cpsid aif, #PSR_M_IRQ
	mov sp, r0
	cpsid aif, #PSR_M_ABT
	mov sp, r0
	cpsid aif, #PSR_M_UND
	mov sp, r0
	cpsid aif, #PSR_M_SVC
	mov pc, lr

.p2align 6
.global trap_vector
trap_vector:
b vec_rst
b vec_und
b vec_svc
b vec_pabt
b vec_dabt
.long 0
b vec_irq
b vec_fiq

vec_rst:
	/* XXX panic */

vec_und:
	stmdb sp, {r0-r3}
	sub r3, sp, #16
	/* subtract pc 4 if arm, 2 if thumb */
	mrs r1, spsr
	tst r1, #PSR_T
	subeq lr, #4
	subne lr, #2
	mov r1, #1
	b save_ctx

vec_svc:
	stmdb sp, {r0-r3}
	sub r3, sp, #16
	mov r1, #2
	b save_ctx

vec_pabt:
	stmdb sp, {r0-r3}
	sub r3, sp, #16
	sub lr, #4
	mov r1, #3
	b save_ctx

vec_dabt:
	stmdb sp, {r0-r3}
	sub r3, sp, #16
	sub lr, #8
	mov r1, #4
	b save_ctx

vec_irq:
	stmdb sp, {r0-r3}
	sub r3, sp, #16
	sub lr, #4
	mov r1, #5
	b save_ctx

vec_fiq:
	stmdb sp, {r0-r3}
	sub r3, sp, #16
	sub lr, #4
	mov r1, #6
	b save_ctx

save_ctx:
	mrc p15, #0, r0, c13, c0, #4
	ldr r0, [r0, #4]

	/* store r4-r7 */
	add r0, #280
	stm r0, {r4-r7}

	/* store r0-r3 */
	sub r0, #16
	ldm r3, {r4-r7}
	stm r0, {r4-r7}

	/* keep irq type around */
	mov r5, r1

	/* store spsr + pc */
	mrs r3, spsr
	mov r2, lr
	add r0, #0x3C
	strd r2, r3, [r0]

	mrs r1, cpsr
	and r2, r1, #0x1F
	and r4, r3, #0x1F

	cmp r2, r4
	beq .Lsave_r8_r12 /* not fiq switch involved */
	cmp r2, #PSR_M_FIQ
	beq .Lto_fiq
	cmp r4, #PSR_M_FIQ
	beq .Lfrom_fiq
	b .Lsave_r8_r12

.Lto_fiq:
	mrs r8, r8_usr
	mrs r9, r9_usr
	mrs r10, r10_usr
	mrs r11, r11_usr
	mrs r12, r12_usr
	b .Lsave_r8_r12

.Lfrom_fiq:
	mrs r8, r8_fiq
	mrs r9, r9_fiq
	mrs r10, r10_fiq
	mrs r11, r11_fiq
	mrs r12, r12_fiq
	b .Lsave_r8_r12

.Lsave_r8_r12:
	sub r0, #0x1C
	stmia r0, {r8-r12}
	add r0, #0x14

	cmp r4, #PSR_M_USR
	beq .Lsave_usr
	cmp r4, #PSR_M_FIQ
	beq .Lsave_fiq
	cmp r4, #PSR_M_IRQ
	beq .Lsave_irq
	cmp r4, #PSR_M_SVC
	beq .Lsave_svc
	cmp r4, #PSR_M_MON
	beq .Lsave_mon
	cmp r4, #PSR_M_ABT
	beq .Lsave_abt
	cmp r4, #PSR_M_UND
	beq .Lsave_und
	b . /* XXX panic */

.Lsave_usr:
	cpsid aif, #PSR_M_SVC
	stm r0, {sp,lr}^
	mov r0, r5
	b arm_trap_handle

.Lsave_fiq:
	cpsid aif, #PSR_M_FIQ
	mov r6, sp
	mov r7, lr
	cpsid aif, #PSR_M_SVC
	stm r0, {r6, r7}
	mov r0, r5
	b arm_trap_handle

.Lsave_irq:
	cpsid aif, #PSR_M_IRQ
	mov r6, sp
	mov r7, lr
	cpsid aif, #PSR_M_SVC
	stm r0, {r6, r7}
	mov r0, r5
	b arm_trap_handle

.Lsave_svc:
	cpsid aif, #PSR_M_SVC
	mov r6, sp
	mov r7, lr
	stm r0, {r6, r7}
	mov r0, r5
	b arm_trap_handle

.Lsave_mon:
	cpsid aif, #PSR_M_MON
	mov r6, sp
	mov r7, lr
	cpsid aif, #PSR_M_SVC
	stm r0, {r6, r7}
	mov r0, r5
	b arm_trap_handle

.Lsave_abt:
	cpsid aif, #PSR_M_ABT
	mov r6, sp
	mov r7, lr
	cpsid aif, #PSR_M_SVC
	stm r0, {r6, r7}
	mov r0, r5
	b arm_trap_handle

.Lsave_und:
	cpsid aif, #PSR_M_UND
	mov r6, sp
	mov r7, lr
	cpsid aif, #PSR_M_SVC
	stm r0, {r6, r7}
	mov r0, r5
	b arm_trap_handle
