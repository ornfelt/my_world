#include <sys/syscall.h>

#include <bits/clone.h>

.type _pthread_fork, %function
.global _pthread_fork
_pthread_fork:
	move $t0, $a0
	li $v0, SYS_clone
	li $a0, CLONE_THREAD | CLONE_VM
	syscall
	beqz $v0, .Lchild
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
	jalr $t0
	negu $s0, $s0
	sw $s0, ($v0)
	lw $ra, ($sp)
	add $sp, $sp, 4
	li $v0, -1
	jr $ra

.Lchild:
	move $a0, $t0
	lw $t0, 0($a0)
	lw $t1, 4($a0)
	add $sp, $t0, $t1
	lw $t0, %call16(_pthread_start)($gp)
	jalr $t0

.section .note.GNU-stack, "", %progbits
.abicalls
