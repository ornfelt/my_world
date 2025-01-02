#include <sys/syscall.h>

#include <bits/clone.h>

.type _pthread_fork, %function
.global _pthread_fork
_pthread_fork:
	mv t0, a0
	li a7, SYS_clone
	li a0, CLONE_THREAD | CLONE_VM
	ecall
	bltz a0, .Lerrno
	beqz a0, .Lchild
	ret

.Lerrno:
	neg a0, a0
	add sp, sp, -8
	sd ra, (sp)
	la t0, __set_errno
	jalr t0
	ld ra, (sp)
	add sp, sp, 8
	li a0, -1
	ret

.Lchild:
	mv a0, t0
	ld t0, 0(a0)
	ld t1, 8(a0)
	add sp, t0, t1
	la t0, _pthread_start
	jalr t0

.section .note.GNU-stack, "", %progbits
