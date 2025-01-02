#include <sys/syscall.h>

#include <bits/clone.h>

.type _pthread_fork, %function
.global _pthread_fork
_pthread_fork:
	mov x1, x0
	mov x8, #SYS_clone
	mov x0, #(CLONE_THREAD | CLONE_VM)
	svc #0
	cmp x0, #0
	b.lt .Lerrno
	b.eq .Lchild
	ret

.Lerrno:
	neg x0, x0
	sub sp, sp, #0x10
	str lr, [sp]
	bl __set_errno
	ldr lr, [sp]
	add sp, sp, #0x10
	mov x0, #-1
	ret lr

.Lchild:
	mov x0, x1
	ldr x7, [x0, #0x0]
	ldr x8, [x0, #0x8]
	add sp, x7, x8
	bl _pthread_start

.section .note.GNU-stack, "", %progbits
