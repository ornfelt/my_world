.section .init
.global _init
_init:
	push %ebp
	movl %esp, %ebp

.section .fini
.global _fini
_fini:
	push %ebp
	movl %esp, %ebp

.section .text
.global __stack_chk_fail_local
__stack_chk_fail_local:
	call __x86.get_pc_thunk.cx
.Lref:
	add $(_GLOBAL_OFFSET_TABLE_ - .Lref), %ecx
	call *__stack_chk_fail@GOT(%ecx)
	ret

.section .note.GNU-stack, "", %progbits
