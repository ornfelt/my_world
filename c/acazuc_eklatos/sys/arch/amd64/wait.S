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

.global arch_waitq_sleep
.type arch_waitq_sleep, %function
arch_waitq_sleep:
	pop %rcx
	mov %gs:8, %rax
	mov %rbx, TF_RBX(%rax)
	mov %rsp, TF_RSP(%rax)
	mov %rbp, TF_RBP(%rax)
	mov %r12, TF_R12(%rax)
	mov %r13, TF_R13(%rax)
	mov %r14, TF_R14(%rax)
	mov %r15, TF_R15(%rax)
	pushf
	pop %rbx
	mov %rbx, TF_RF(%rax)
	mov %rcx, TF_RIP(%rax)
	mov $0, %rax
	push %rcx
	ret

.global arch_waitq_leave
.type arch_waitq_leave, %function
arch_waitq_leave:
	mov %gs:8, %rax
	mov TF_RBX(%rax), %rbx
	mov TF_RSP(%rax), %rsp
	mov TF_RBP(%rax), %rbp
	mov TF_R12(%rax), %r12
	mov TF_R13(%rax), %r13
	mov TF_R14(%rax), %r14
	mov TF_R15(%rax), %r15
	mov TF_RF(%rax), %rdx
	mov TF_RIP(%rax), %rcx
	/* XXX should we write the segments values ? */
	mov $1, %rax
	push %rdx
	popf
	push %rcx
	ret
