#ifndef EKLAT_SYSCALL_H
#define EKLAT_SYSCALL_H

#include <sys/syscall.h>
#include <sys/types.h>

#if defined(__i386__)

#define SYSCALL_INSTR "int $0x80"
#define SYSCALL_REG_ID "eax"
#define SYSCALL_REG_RET "eax"
#define SYSCALL_REG_ARG1 "ebx"
#define SYSCALL_REG_ARG2 "ecx"
#define SYSCALL_REG_ARG3 "edx"
#define SYSCALL_REG_ARG4 "esi"
#define SYSCALL_REG_ARG5 "edi"
#define SYSCALL_REG_ARG6 "ebp"
#define SYSCALL_CLOBBER

#elif defined(__x86_64__)

#define SYSCALL_INSTR "syscall"
#define SYSCALL_REG_ID "rax"
#define SYSCALL_REG_RET "rax"
#define SYSCALL_REG_ARG1 "rdi"
#define SYSCALL_REG_ARG2 "rsi"
#define SYSCALL_REG_ARG3 "rdx"
#define SYSCALL_REG_ARG4 "r10"
#define SYSCALL_REG_ARG5 "r8"
#define SYSCALL_REG_ARG6 "r9"
#define SYSCALL_CLOBBER , "ecx", "r11"

#elif defined(__arm__)

#define SYSCALL_INSTR "swi #0x0"
#define SYSCALL_REG_ID "r7"
#define SYSCALL_REG_RET "r0"
#define SYSCALL_REG_ARG1 "r0"
#define SYSCALL_REG_ARG2 "r1"
#define SYSCALL_REG_ARG3 "r2"
#define SYSCALL_REG_ARG4 "r3"
#define SYSCALL_REG_ARG5 "r4"
#define SYSCALL_REG_ARG6 "r5"
#define SYSCALL_CLOBBER

#elif defined(__aarch64__)

#define SYSCALL_INSTR "svc #0"
#define SYSCALL_REG_ID "x8"
#define SYSCALL_REG_RET "x0"
#define SYSCALL_REG_ARG1 "x0"
#define SYSCALL_REG_ARG2 "x1"
#define SYSCALL_REG_ARG3 "x2"
#define SYSCALL_REG_ARG4 "x3"
#define SYSCALL_REG_ARG5 "x4"
#define SYSCALL_REG_ARG6 "x5"
#define SYSCALL_CLOBBER

#elif defined(__riscv)

#define SYSCALL_INSTR "ecall"
#define SYSCALL_REG_ID "a7"
#define SYSCALL_REG_RET "a0"
#define SYSCALL_REG_ARG1 "a0"
#define SYSCALL_REG_ARG2 "a1"
#define SYSCALL_REG_ARG3 "a2"
#define SYSCALL_REG_ARG4 "a3"
#define SYSCALL_REG_ARG5 "a4"
#define SYSCALL_REG_ARG6 "a5"
#define SYSCALL_CLOBBER

#elif defined(__mips__)

/* NB: registers a4 and a5 don't exist in o32
 * we're using them anyway because the syscall ABI is not standardised
 * $4-$9 register names are used instead of a4-a5 because of this
 */
#define SYSCALL_INSTR "syscall"
#define SYSCALL_REG_ID "v0"
#define SYSCALL_REG_RET "v0"
#define SYSCALL_REG_ARG1 "$4"
#define SYSCALL_REG_ARG2 "$5"
#define SYSCALL_REG_ARG3 "$6"
#define SYSCALL_REG_ARG4 "$7"
#define SYSCALL_REG_ARG5 "$8"
#define SYSCALL_REG_ARG6 "$9"
#define SYSCALL_CLOBBER

#else

#error "unknown arch"

#endif

#ifdef __cplusplus
extern "C" {
#endif

static inline ssize_t syscall0_raw(uintptr_t id)
{
	register long r_id __asm__ (SYSCALL_REG_ID) = id;
	register long r_ret __asm__ (SYSCALL_REG_RET);
	__asm__ volatile (SYSCALL_INSTR:
	                  "=r"(r_ret):
	                  "r"(r_id):
	                  "memory" SYSCALL_CLOBBER);
	return r_ret;
}

static inline ssize_t syscall1_raw(uintptr_t id, uintptr_t arg1)
{
	register long r_id __asm__ (SYSCALL_REG_ID) = id;
	register long r_ret __asm__ (SYSCALL_REG_RET);
	register long r_arg1 __asm__ (SYSCALL_REG_ARG1) = arg1;
	__asm__ volatile (SYSCALL_INSTR:
	                  "=r"(r_ret):
	                  "r"(r_id),
	                  "r"(r_arg1):
	                  "memory" SYSCALL_CLOBBER);
	return r_ret;
}

static inline ssize_t syscall2_raw(uintptr_t id, uintptr_t arg1,
                                   uintptr_t arg2)
{
	register long r_id __asm__ (SYSCALL_REG_ID) = id;
	register long r_ret __asm__ (SYSCALL_REG_RET);
	register long r_arg1 __asm__ (SYSCALL_REG_ARG1) = arg1;
	register long r_arg2 __asm__ (SYSCALL_REG_ARG2) = arg2;
	__asm__ volatile (SYSCALL_INSTR:
	                  "=r"(r_ret):
	                  "r"(r_id),
	                  "r"(r_arg1),
	                  "r"(r_arg2):
	                  "memory" SYSCALL_CLOBBER);
	return r_ret;
}

static inline ssize_t syscall3_raw(uintptr_t id, uintptr_t arg1,
                                   uintptr_t arg2, uintptr_t arg3)
{
	register long r_id __asm__ (SYSCALL_REG_ID) = id;
	register long r_ret __asm__ (SYSCALL_REG_RET);
	register long r_arg1 __asm__ (SYSCALL_REG_ARG1) = arg1;
	register long r_arg2 __asm__ (SYSCALL_REG_ARG2) = arg2;
	register long r_arg3 __asm__ (SYSCALL_REG_ARG3) = arg3;
	__asm__ volatile (SYSCALL_INSTR:
	                  "=r"(r_ret):
	                  "r"(r_id),
	                  "r"(r_arg1),
	                  "r"(r_arg2),
	                  "r"(r_arg3):
	                  "memory" SYSCALL_CLOBBER);
	return r_ret;
}

static inline ssize_t syscall4_raw(uintptr_t id, uintptr_t arg1,
                                   uintptr_t arg2, uintptr_t arg3,
                                   uintptr_t arg4)
{
	register long r_id __asm__ (SYSCALL_REG_ID) = id;
	register long r_ret __asm__ (SYSCALL_REG_RET);
	register long r_arg1 __asm__ (SYSCALL_REG_ARG1) = arg1;
	register long r_arg2 __asm__ (SYSCALL_REG_ARG2) = arg2;
	register long r_arg3 __asm__ (SYSCALL_REG_ARG3) = arg3;
	register long r_arg4 __asm__ (SYSCALL_REG_ARG4) = arg4;
	__asm__ volatile (SYSCALL_INSTR:
	                  "=r"(r_ret):
	                  "r"(r_id),
	                  "r"(r_arg1),
	                  "r"(r_arg2),
	                  "r"(r_arg3),
	                  "r"(r_arg4):
	                  "memory" SYSCALL_CLOBBER);
	return r_ret;
}

static inline ssize_t syscall5_raw(uintptr_t id, uintptr_t arg1,
                                   uintptr_t arg2, uintptr_t arg3,
                                   uintptr_t arg4, uintptr_t arg5)
{
	register long r_id __asm__ (SYSCALL_REG_ID) = id;
	register long r_ret __asm__ (SYSCALL_REG_RET);
	register long r_arg1 __asm__ (SYSCALL_REG_ARG1) = arg1;
	register long r_arg2 __asm__ (SYSCALL_REG_ARG2) = arg2;
	register long r_arg3 __asm__ (SYSCALL_REG_ARG3) = arg3;
	register long r_arg4 __asm__ (SYSCALL_REG_ARG4) = arg4;
	register long r_arg5 __asm__ (SYSCALL_REG_ARG5) = arg5;
	__asm__ volatile (SYSCALL_INSTR:
	                  "=r"(r_ret):
	                  "r"(r_id),
	                  "r"(r_arg1),
	                  "r"(r_arg2),
	                  "r"(r_arg3),
	                  "r"(r_arg4),
	                  "r"(r_arg5):
	                  "memory" SYSCALL_CLOBBER);
	return r_ret;
}

static inline ssize_t syscall6_raw(uintptr_t id, uintptr_t arg1,
                                   uintptr_t arg2, uintptr_t arg3,
                                   uintptr_t arg4, uintptr_t arg5,
                                   uintptr_t arg6)
{
#if defined(__i386__)
	uintptr_t args[2] = {arg5, arg6};
	/* gcc is really bothering us here
	 * we can't fill directly the 6 registers because of reg contraints
	 * we can't directly access args because it generates invalid assembly
	 * instead, we just push the two last args on the stack
	 * (because ebp isn't allowed inline input)
	 */
	__asm__ volatile ("push %%ebp;"
	                  "push %6;"
	                  "mov 0(%%esp), %%edi;"
	                  "add $4, %%esp;"
	                  "mov 4(%%edi), %%ebp;"
	                  "mov 0(%%edi), %%edi;"
	                  "int $0x80;"
	                  "pop %%ebp":
	                  "=a"(id):
	                  "a"(id),
	                  "b"(arg1),
	                  "c"(arg2),
	                  "d"(arg3),
	                  "S"(arg4),
	                  "g"(&args[0]):
	                  "edi",
	                  "memory" SYSCALL_CLOBBER);
	return id;
#else
	register long r_id __asm__ (SYSCALL_REG_ID) = id;
	register long r_ret __asm__ (SYSCALL_REG_RET);
	register long r_arg1 __asm__ (SYSCALL_REG_ARG1) = arg1;
	register long r_arg2 __asm__ (SYSCALL_REG_ARG2) = arg2;
	register long r_arg3 __asm__ (SYSCALL_REG_ARG3) = arg3;
	register long r_arg4 __asm__ (SYSCALL_REG_ARG4) = arg4;
	register long r_arg5 __asm__ (SYSCALL_REG_ARG5) = arg5;
	register long r_arg6 __asm__ (SYSCALL_REG_ARG6) = arg6;
	__asm__ volatile (SYSCALL_INSTR:
	                  "=r"(r_ret):
	                  "r"(r_id),
	                  "r"(r_arg1),
	                  "r"(r_arg2),
	                  "r"(r_arg3),
	                  "r"(r_arg4),
	                  "r"(r_arg5),
	                  "r"(r_arg6):
	                  "memory" SYSCALL_CLOBBER);
	return r_ret;
#endif
}

#ifdef __cplusplus
}
#endif

#endif
