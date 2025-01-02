#include <sys/syscall.h>

#include <bits/clone.h>

.global vfork
.type vfork, %function
vfork:
	mov x8, #SYS_clone
	mov x0, #(CLONE_VFORK | CLONE_VM)
	svc #0
	add x0, x0, #0xFFF
	cmp x0, #0xFFE
	b.ls .Lerrno
	sub x0, x0, #0xFFF
	ret

.Lerrno:
	neg x0, x0
	mov x19, lr
	bl __set_errno
	mov x0, #-1
	mov lr, x19
	ret

.section .note.GNU-stack, "", %progbits
