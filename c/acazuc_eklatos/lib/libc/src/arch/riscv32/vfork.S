#include <sys/syscall.h>

#include <bits/clone.h>

.global vfork
.type vfork, %function
vfork:
	li a7, SYS_clone
	li a0, CLONE_VFORK | CLONE_VM
	ecall
	blt a0, zero, .Lerrno
	ret

.Lerrno:
	neg a0, a0
	add sp, sp, -4
	sw ra, (sp)
	la t0, __set_errno
	jalr t0
	lw ra, (sp)
	add sp, sp, 4
	li a0, -1
	ret

.section .note.GNU-stack, "", %progbits
