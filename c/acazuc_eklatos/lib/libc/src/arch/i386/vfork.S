#include <sys/syscall.h>

#include <bits/clone.h>

.global vfork
.type vfork, %function
vfork:
	mov (%esp), %edx
	mov %ebx, %ecx
	mov $SYS_clone, %eax
	mov $(CLONE_VFORK | CLONE_VM), %ebx
	int $0x80
	mov %edx, (%esp)
	test %eax, %eax
	js .Lerrno
	mov %ecx, %ebx
	ret
.Lerrno:
	push %ecx
	neg %eax
	call __x86.get_pc_thunk.bx
.Lref:
	add $(_GLOBAL_OFFSET_TABLE_ - .Lref), %ebx
	push %eax
	call __set_errno@PLT
	add $4, %esp
	mov $-1, %eax
	pop %ebx
	ret

.section .note.GNU-stack, "", %progbits
