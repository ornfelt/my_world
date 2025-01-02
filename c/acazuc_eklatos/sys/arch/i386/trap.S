/* stack frame:
 * ss (if ring cross)
 * esp (if ring cross)
 * eflags
 * cs
 * eip
 * err
 * id
 */

#define TF_FPU (0)
#define TF_REGS (TF_FPU + 3072)
#define TF_EAX (TF_REGS + 4 * 0)
#define TF_EBX (TF_REGS + 4 * 1)
#define TF_ECX (TF_REGS + 4 * 2)
#define TF_EDX (TF_REGS + 4 * 3)
#define TF_ESI (TF_REGS + 4 * 4)
#define TF_EDI (TF_REGS + 4 * 5)
#define TF_ESP (TF_REGS + 4 * 6)
#define TF_EBP (TF_REGS + 4 * 7)
#define TF_EIP (TF_REGS + 4 * 8)
#define TF_CS  (TF_REGS + 4 * 9)
#define TF_DS  (TF_REGS + 4 * 10)
#define TF_ES  (TF_REGS + 4 * 11)
#define TF_FS  (TF_REGS + 4 * 12)
#define TF_GS  (TF_REGS + 4 * 13)
#define TF_SS  (TF_REGS + 4 * 14)
#define TF_EF  (TF_REGS + 4 * 15)

isr_common:
	cld

	/* save ctx */
	push %ss
	push %gs
	push %fs
	push %es
	push %ds
	push %eax
	push %ebx

	call reload_data_segments

	/* set IOPL 0 */
	pushf
	pop %eax
	and $0xFFFFCFFF, %eax
	push %eax
	popf

	/* if ring has been crossed, load ss & esp from int stack
	 * otherwise, keep already-present values
	 */
	mov 10 * 4(%esp), %eax
	test $0x3, %eax
	mov %gs:4, %eax
	jz .Lno_ring_cross_pre

	/* ring crossed, take ss:esp from stack */
	mov 12 * 4(%esp), %ebx
	mov %ebx, TF_ESP(%eax)
	mov 13 * 4(%esp), %ebx
	mov %ebx, TF_SS(%eax)
	jmp .Lcommon_store

.Lno_ring_cross_pre:
	/* no ring cross, load ss:esp from current esp & saved ss */
	mov %esp, %ebx
	add $12 * 4, %ebx
	mov %ebx, TF_ESP(%eax)
	mov 6 * 4(%esp), %ebx
	mov %ebx, TF_SS(%eax)

.Lcommon_store:
	mov 1 * 4(%esp), %ebx
	mov %ebx, TF_EAX(%eax)
	mov 0 * 4(%esp), %ebx
	mov %ebx, TF_EBX(%eax)
	mov %ecx, TF_ECX(%eax)
	mov %edx, TF_EDX(%eax)
	mov %esi, TF_ESI(%eax)
	mov %edi, TF_EDI(%eax)
	mov %ebp, TF_EBP(%eax)
	mov 9 * 4(%esp), %ebx
	mov %ebx, TF_EIP(%eax)
	mov 10 * 4(%esp), %ebx
	mov %ebx, TF_CS(%eax)
	mov 2 * 4(%esp), %ebx
	mov %ebx, TF_DS(%eax)
	mov 3 * 4(%esp), %ebx
	mov %ebx, TF_ES(%eax)
	mov 4 * 4(%esp), %ebx
	mov %ebx, TF_FS(%eax)
	mov 5 * 4(%esp), %ebx
	mov %ebx, TF_GS(%eax)
	mov 11 * 4(%esp), %ebx
	mov %ebx, TF_EF(%eax)

	/* cleanup stack & generate interrupt handle stack */
	mov 8 * 4(%esp), %esi /* err */
	mov 7 * 4(%esp), %edi /* id */
	add $4 * 12, %esp
	mov TF_CS(%eax), %ebx
	test $0x3, %ebx
	jz .Lno_ring_cross_stack
	add $4 * 2, %esp
.Lno_ring_cross_stack:
	push %esi
	push %eax
	push %esp
	push %edi
	call trap_handle

.global context_switch
.type context_switch, %function
context_switch:
	mov 4(%esp), %eax

	mov TF_CS(%eax), %ebx
	test $0x3, %ebx
	jz .Lno_ring_cross_post
	sub $2 * 4, %esp
	mov TF_SS(%eax), %ebx
	mov %ebx, 1 * 4(%esp)
	mov TF_ESP(%eax), %ebx
	mov %ebx, 0 * 4(%esp)
	jmp .Lcommon_cross_post

.Lno_ring_cross_post:
	mov TF_ESP(%eax), %esp

.Lcommon_cross_post:
	sub $3 * 4, %esp
	mov TF_EF(%eax), %ebx
	mov %ebx, 2 * 4(%esp)
	mov TF_CS(%eax), %ebx
	mov %ebx, 1 * 4(%esp)
	mov TF_EIP(%eax), %ebx
	mov %ebx, 0 * 4(%esp)

	push TF_EAX(%eax)
	mov TF_EBP(%eax), %ebp
	mov TF_EDI(%eax), %edi
	mov TF_ESI(%eax), %esi
	mov TF_EDX(%eax), %edx
	mov TF_ECX(%eax), %ecx
	mov TF_EBX(%eax), %ebx
	mov TF_ES(%eax), %es
	mov TF_FS(%eax), %fs
	mov TF_GS(%eax), %gs
	mov TF_DS(%eax), %ds
	pop %eax /* ds isn't available anymore, use ss */
	iret

.macro isr_err id
isr_\id:
	push $\id
	jmp isr_common
.endm

.macro isr_no_err id
isr_\id:
	push $0
	push $\id
	jmp isr_common
.endm

isr_no_err 0
isr_no_err 1
isr_no_err 2
isr_no_err 3
isr_no_err 4
isr_no_err 5
isr_no_err 6
isr_no_err 7
isr_err    8
isr_no_err 9
isr_err    10
isr_err    11
isr_err    12
isr_err    13
isr_err    14
isr_no_err 15
isr_no_err 16
isr_err    17
isr_no_err 18
isr_no_err 19
isr_no_err 20
isr_err    21
isr_no_err 22
isr_no_err 23
isr_no_err 24
isr_no_err 25
isr_no_err 26
isr_no_err 27
isr_no_err 28
isr_err    29
isr_err    30
isr_no_err 31

.altmacro

.set i, 32
.rept 224
	isr_no_err %i
	.set i, i + 1
.endr

.global g_isr_table
.type g_isr_table, %object
g_isr_table:

.macro isr_entry id
	.long isr_\id
.endm

.set i, 0
.rept 256
	isr_entry %i
	.set i, i + 1
.endr
