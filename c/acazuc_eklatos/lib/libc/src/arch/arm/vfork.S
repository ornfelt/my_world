#include <sys/syscall.h>

#include <bits/clone.h>

.global vfork
.type vfork, %function
vfork:
	mov r2, r7
	mov r7, #SYS_clone
	mov r0, #(CLONE_VFORK | CLONE_VM)
	swi #0
	mov r7, r2
	cmn r0, #0x1000
	bhi .Lerrno
	mov pc, lr

.Lerrno:
	neg r0, r0
	push {lr}
	bl __set_errno
	mov r0, #-1
	pop {pc}

.section .note.GNU-stack, "", %progbits
