#include <sys/syscall.h>

#include <bits/clone.h>

.global vfork
.type vfork, %function
vfork:
	mov (%rsp), %rdx
	mov $SYS_clone, %rax
	mov $(CLONE_VFORK | CLONE_VM), %rdi
	int $0x80
	mov %rdx, (%rsp)
	test %rax, %rax
	js .Lerrno
	ret

.Lerrno:
	neg %rax
	mov %rax, %rdi
	call __set_errno@PLT
	mov $-1, %rax
	ret

.section .note.GNU-stack, "", %progbits
