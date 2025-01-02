.section .init
	pop %ebp
	ret

.section .fini
	pop %ebp
	ret

.section .note.GNU-stack, "", %progbits
