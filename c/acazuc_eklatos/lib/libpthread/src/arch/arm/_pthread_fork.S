#include <sys/syscall.h>

#include <bits/clone.h>

.type _pthread_fork, %function
.global _pthread_fork
_pthread_fork:
	mov r2, r7
	mov r1, r0
	mov r7, #SYS_clone
	mov r0, #(CLONE_THREAD | CLONE_VM)
	swi #0
	mov r7, r2
	cmp r0, #0
	blt .Lerrno
	beq .Lchild
	mov pc, lr

.Lerrno:
	neg r0, r0
	push {lr}
	bl __set_errno
	mov r0, #-1
	pop {pc}

.Lchild:
	mov r0, r1
	ldr r1, [r0, #0]
	ldr r2, [r0, #4]
	add sp, r1, r2
	bl _pthread_start

.section .note.GNU-stack, "", %progbits
