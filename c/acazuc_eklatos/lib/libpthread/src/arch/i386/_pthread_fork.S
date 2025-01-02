#include <sys/syscall.h>

#include <bits/clone.h>

.type _pthread_fork, %function
.global _pthread_fork
_pthread_fork:
	mov %ebx, %edx
	mov 4(%esp), %ecx
	mov $SYS_clone, %eax
	mov $(CLONE_THREAD | CLONE_VM), %ebx
	int $0x80
	test %eax, %eax
	js .Lerrno
	jz .Lchild
	mov %edx, %ebx
	ret

.Lerrno:
	push %edx
	neg %eax
	call __x86.get_pc_thunk.bx
.Lref:
	add $(_GLOBAL_OFFSET_TABLE_ - .Lref), %ebx
	push %eax
	call *__set_errno@GOT(%ebx)
	add $4, %esp
	mov $-1, %eax
	pop %ebx
	ret

.Lchild:
	/* setup stack */
	mov 0(%ecx), %esp
	add 4(%ecx), %esp
	push $0
	mov %esp, %ebp

	call __x86.get_pc_thunk.bx
.Lref2:
	add $(_GLOBAL_OFFSET_TABLE_ - .Lref2), %ebx
	push %ecx
	call *_pthread_start@GOT(%ebx)

.section .note.GNU-stack, "", %progbits
