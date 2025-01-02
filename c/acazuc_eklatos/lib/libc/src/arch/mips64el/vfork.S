#include <sys/syscall.h>

#iclude <bits/clone.h>

.global vfork
.type vfork, %function
vfork:
	li v0, SYS_clone
	li a0, CLONE_VFORK | CLONE_VM
	syscall
	/* XXX errno */
	ret

.section .note.GNU-stack, "", %progbits
