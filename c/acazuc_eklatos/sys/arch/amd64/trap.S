/* stack frame:
 * ss
 * rsp
 * rflags
 * cs
 * rip
 * err
 * id
 */

#define TF_FPU (0)
#define TF_REGS (TF_FPU + 3072)
#define TF_RAX (TF_REGS + 8 * 0)
#define TF_RBX (TF_REGS + 8 * 1)
#define TF_RCX (TF_REGS + 8 * 2)
#define TF_RDX (TF_REGS + 8 * 3)
#define TF_RSI (TF_REGS + 8 * 4)
#define TF_RDI (TF_REGS + 8 * 5)
#define TF_RSP (TF_REGS + 8 * 6)
#define TF_RBP (TF_REGS + 8 * 7)
#define TF_R8  (TF_REGS + 8 * 8)
#define TF_R9  (TF_REGS + 8 * 9)
#define TF_R10 (TF_REGS + 8 * 10)
#define TF_R11 (TF_REGS + 8 * 11)
#define TF_R12 (TF_REGS + 8 * 12)
#define TF_R13 (TF_REGS + 8 * 13)
#define TF_R14 (TF_REGS + 8 * 14)
#define TF_R15 (TF_REGS + 8 * 15)
#define TF_RIP (TF_REGS + 8 * 16)
#define TF_CS  (TF_REGS + 8 * 17)
#define TF_DS  (TF_REGS + 8 * 18)
#define TF_ES  (TF_REGS + 8 * 19)
#define TF_FS  (TF_REGS + 8 * 20)
#define TF_GS  (TF_REGS + 8 * 21)
#define TF_SS  (TF_REGS + 8 * 22)
#define TF_RF  (TF_REGS + 8 * 23)

.global syscall_enter
.type syscall_enter, %function
syscall_enter:
	swapgs

	mov %rsp, %gs:16
	mov %gs:24, %rsp
	push %rax

	mov %ss, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %gs:8, %rax
	mov %rbx, TF_RBX(%rax)
	mov 0 * 8(%rsp), %rbx
	mov %rbx, TF_RAX(%rax)
	mov %rdx, TF_RDX(%rax)
	mov %rsi, TF_RSI(%rax)
	mov %rdi, TF_RDI(%rax)
	mov %gs:16, %rbx
	mov %rbx, TF_RSP(%rax)
	mov %rbp, TF_RBP(%rax)
	mov %r8,  TF_R8(%rax)
	mov %r9,  TF_R9(%rax)
	mov %r10, TF_R10(%rax)
	mov %r12, TF_R12(%rax)
	mov %r13, TF_R13(%rax)
	mov %r14, TF_R14(%rax)
	mov %r15, TF_R15(%rax)
	mov %rcx, TF_RIP(%rax)
	mov %r11, TF_RF(%rax)

	/* call handler */
	mov $0x80, %rdi
	push $0
	push %rax
	mov %rsp, %rsi

	/* enforce 16-bytes stack alignment */
	and $0xFFFFFFFFFFFFFFF0, %rsp
	call trap_handle

.global syscall_leave
.type syscall_leave, %function
syscall_leave:
	mov %rdi, %rax

	push TF_RAX(%rax)
	mov TF_RSP(%rax), %r15
	mov %r15, %gs:16
	mov TF_R15(%rax), %r15
	mov TF_R14(%rax), %r14
	mov TF_R13(%rax), %r13
	mov TF_R12(%rax), %r12
	mov TF_RF (%rax), %r11
	mov TF_R10(%rax), %r10
	mov TF_R9 (%rax), %r9
	mov TF_R8 (%rax), %r8
	mov TF_RBP(%rax), %rbp
	mov TF_RDI(%rax), %rdi
	mov TF_RSI(%rax), %rsi
	mov TF_RDX(%rax), %rdx
	mov TF_RIP(%rax), %rcx
	mov TF_RBX(%rax), %rbx
	mov TF_ES(%rax), %es
	mov TF_DS(%rax), %ds
	pop %rax /* ds isn't available anymore at this point, but ss is */
	mov %gs:16, %rsp
	sysretq

isr_common:
	cld

	/* save ctx */
	push %rax /* trapframe */
	push %rbx /* tmp value */

	call reload_data_segments

	/* swapgs only if we come from userland */
	mov 5 * 8(%rsp), %rax
	test $0x3, %rax
	jz .Lno_swapgs
	swapgs
.Lno_swapgs:

	/* set IOPL 0 */
	pushf
	pop %rax
	and $0xFFFFFFFFFFFFCFFF, %rax
	push %rax
	popf

	mov %gs:8, %rax
	mov 1 * 8(%rsp), %rbx
	mov %rbx, TF_RAX(%rax)
	mov 0 * 8(%rsp), %rbx
	mov %rbx, TF_RBX(%rax)
	mov %rcx, TF_RCX(%rax)
	mov %rdx, TF_RDX(%rax)
	mov %rsi, TF_RSI(%rax)
	mov %rdi, TF_RDI(%rax)
	mov 7 * 8(%rsp), %rbx
	mov %rbx, TF_RSP(%rax)
	mov %rbp, TF_RBP(%rax)
	mov %r8,  TF_R8(%rax)
	mov %r9,  TF_R9(%rax)
	mov %r10, TF_R10(%rax)
	mov %r11, TF_R11(%rax)
	mov %r12, TF_R12(%rax)
	mov %r13, TF_R13(%rax)
	mov %r14, TF_R14(%rax)
	mov %r15, TF_R15(%rax)
	mov 4 * 8(%rsp), %rbx
	mov %rbx, TF_RIP(%rax)
	mov 5 * 8(%rsp), %rbx
	mov %rbx, TF_CS(%rax)
	mov 8 * 8(%rsp), %rbx
	movq %rbx, TF_DS(%rax)
	movq %rbx, TF_ES(%rax)
	movq %rbx, TF_FS(%rax)
	movq %rbx, TF_GS(%rax)
	movq %rbx, TF_SS(%rax)
	mov 6 * 8(%rsp), %rbx
	mov %rbx, TF_RF(%rax)

	/* call handler */
	mov 2 * 8(%rsp), %rdi
	push 3 * 8(%rsp)
	push %rax
	mov %rsp, %rsi

	/* enforce 16-bytes stack alignment */
	and $0xFFFFFFFFFFFFFFF0, %rsp
	call trap_handle

.global context_switch
.type context_switch, %function
context_switch:
	mov %rdi, %rax

	sub $5 * 8, %rsp
	mov TF_SS(%rax), %rbx
	mov %rbx, 4 * 8(%rsp)
	mov TF_RSP(%rax), %rbx
	mov %rbx, 3 * 8(%rsp)
	mov TF_RF(%rax), %rbx
	mov %rbx, 2 * 8(%rsp)
	mov TF_CS(%rax), %rbx
	mov %rbx, 1 * 8(%rsp)
	mov TF_RIP(%rax), %rbx
	mov %rbx, 0 * 8(%rsp)

	push TF_RAX(%rax)
	mov TF_R15(%rax), %r15
	mov TF_R14(%rax), %r14
	mov TF_R13(%rax), %r13
	mov TF_R12(%rax), %r12
	mov TF_R11(%rax), %r11
	mov TF_R10(%rax), %r10
	mov TF_R9 (%rax), %r9
	mov TF_R8 (%rax), %r8
	mov TF_RBP(%rax), %rbp
	mov TF_RDI(%rax), %rdi
	mov TF_RSI(%rax), %rsi
	mov TF_RDX(%rax), %rdx
	mov TF_RCX(%rax), %rcx
	mov TF_RBX(%rax), %rbx
	mov TF_ES(%rax), %es
	mov TF_DS(%rax), %ds
	pop %rax /* ds isn't available anymore at this point, but ss is */
	iretq

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
	.quad isr_\id
.endm

.set i, 0
.rept 256
	isr_entry %i
	.set i, i + 1
.endr
