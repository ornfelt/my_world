#include <sys/syscall.h>

#include <bits/clone.h>

.type _pthread_fork, %function
.global _pthread_fork
_pthread_fork:
	mov %rdi, %rsi
	mov $SYS_clone, %rax
	mov $(CLONE_THREAD | CLONE_VM), %rdi
	int $0x80
	test %rax, %rax
	js .Lerrno
	jz .Lchild
	ret

.Lerrno:
	neg %rax
	mov %rax, %rdi
	call *__set_errno@GOTPCREL(%rip)
	mov $-1, %rax
	ret

.Lchild:
	mov 0(%rsi), %rsp
	add 8(%rsi), %rsp
	push $0 /* two times for 16-bytes stack alignment */
	push $0
	mov %rsp, %rbp
	mov %rsi, %rdi
	call *_pthread_start@GOTPCREL(%rip)

.section .note.GNU-stack, "", %progbits
