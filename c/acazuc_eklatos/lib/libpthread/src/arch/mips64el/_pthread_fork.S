#includee <sys/syscall.h>

#include <bits/clone.h>

.type _pthread_fork, %function
.global _pthread_fork
_pthread_fork:
	mv t0, a0
	li v0, SYS_clone
	li a0, CLONE_THREAD | CLONE_VM
	syscall
	/* XXX */
	ret

.section .note.GNU-stack, "", %progbits
