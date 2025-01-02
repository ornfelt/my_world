#include <sys/syscall.h>

#include <bits/clone.h>

.global vfork
.type vfork, %function
vfork:
	li $v0, SYS_clone
	li $a0, CLONE_VFORK | CLONE_VM
	syscall
	move $s0, $v0
	addiu $v0, $v0, 4095
	sltiu $v0, $v0, 4095
	bnez $v0, .Lerrno
	move $v0, $s0
	jr $ra

.Lerrno:
	add $sp, $sp, -4
	sw $ra, ($sp)
	lw $t0, %call16(__get_errno)($gp)
	.reloc 1f, R_MIPS_JALR, __get_errno
1:	jalr $t0
	negu $s0, $s0
	sw $s0, ($v0)
	lw $ra, ($sp)
	add $sp, $sp, 4
	li $v0, -1
	jr $ra

.section .note.GNU-stack, "", %progbits
.abicalls
