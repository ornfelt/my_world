#if __riscv_xlen == 64
#define ll ld
#define sl sd
#else
#define ll lw
#define sl sw
#endif

#define TF_PC  (0 * __SIZEOF_POINTER__)
#define TF_RA  (1 * __SIZEOF_POINTER__)
#define TF_SP  (2 * __SIZEOF_POINTER__)
#define TF_GP  (3 * __SIZEOF_POINTER__)
#define TF_TP  (4 * __SIZEOF_POINTER__)
#define TF_T0  (5 * __SIZEOF_POINTER__)
#define TF_T1  (6 * __SIZEOF_POINTER__)
#define TF_T2  (7 * __SIZEOF_POINTER__)
#define TF_FP  (8 * __SIZEOF_POINTER__)
#define TF_S1  (9 * __SIZEOF_POINTER__)
#define TF_A0  (10 * __SIZEOF_POINTER__)
#define TF_A1  (11 * __SIZEOF_POINTER__)
#define TF_A2  (12 * __SIZEOF_POINTER__)
#define TF_A3  (13 * __SIZEOF_POINTER__)
#define TF_A4  (14 * __SIZEOF_POINTER__)
#define TF_A5  (15 * __SIZEOF_POINTER__)
#define TF_A6  (16 * __SIZEOF_POINTER__)
#define TF_A7  (17 * __SIZEOF_POINTER__)
#define TF_S2  (18 * __SIZEOF_POINTER__)
#define TF_S3  (19 * __SIZEOF_POINTER__)
#define TF_S4  (20 * __SIZEOF_POINTER__)
#define TF_S5  (21 * __SIZEOF_POINTER__)
#define TF_S6  (22 * __SIZEOF_POINTER__)
#define TF_S7  (23 * __SIZEOF_POINTER__)
#define TF_S8  (24 * __SIZEOF_POINTER__)
#define TF_S9  (25 * __SIZEOF_POINTER__)
#define TF_S10 (26 * __SIZEOF_POINTER__)
#define TF_S11 (27 * __SIZEOF_POINTER__)
#define TF_T3  (28 * __SIZEOF_POINTER__)
#define TF_T4  (29 * __SIZEOF_POINTER__)
#define TF_T5  (30 * __SIZEOF_POINTER__)
#define TF_T6  (31 * __SIZEOF_POINTER__)

.global context_switch
.type context_switch, %function
context_switch:
	addi a0, a0, 520
	ll t0, TF_PC(a0)
	csrw sepc, t0
	ll ra, TF_RA(a0)
	ll sp, TF_SP(a0)
	ll gp, TF_GP(a0)
	ll tp, TF_TP(a0)
	ll t0, TF_T0(a0)
	ll t1, TF_T1(a0)
	ll t2, TF_T2(a0)
	ll fp, TF_FP(a0)
	ll s1, TF_S1(a0)
	ll a1, TF_A1(a0)
	ll a2, TF_A2(a0)
	ll a3, TF_A3(a0)
	ll a4, TF_A4(a0)
	ll a5, TF_A5(a0)
	ll a6, TF_A6(a0)
	ll a7, TF_A7(a0)
	ll s2, TF_S2(a0)
	ll s3, TF_S3(a0)
	ll s4, TF_S4(a0)
	ll s5, TF_S5(a0)
	ll s6, TF_S6(a0)
	ll s7, TF_S7(a0)
	ll s8, TF_S8(a0)
	ll s9, TF_S9(a0)
	ll s10, TF_S10(a0)
	ll s11, TF_S11(a0)
	ll t3, TF_T3(a0)
	ll t4, TF_T4(a0)
	ll t5, TF_T5(a0)
	ll t6, TF_T6(a0)
	ll a0, TF_A0(a0)
	sret

.p2align 2
.global trap_vector
trap_vector:
	csrrw tp, sscratch, tp
	sl tp, 2 * __SIZEOF_POINTER__(tp)
	sl t0, 3 * __SIZEOF_POINTER__(tp)
	ll t0, 1 * __SIZEOF_POINTER__(tp)
	addi t0, t0, 520
	sl ra, TF_RA(t0)
	sl sp, TF_SP(t0)
	sl gp, TF_GP(t0)
	sl t1, TF_T1(t0)
	sl t2, TF_T2(t0)
	mv t1, tp
	csrrw t1, sscratch, t1 /* conviently restore sscratch en passant */
	sl t1, TF_TP(t0)
	ll t1, 3 * __SIZEOF_POINTER__(tp)
	sl t1, TF_T0(t0)
	sl fp, TF_FP(t0)
	sl s1, TF_S1(t0)
	sl a0, TF_A0(t0)
	sl a1, TF_A1(t0)
	sl a2, TF_A2(t0)
	sl a3, TF_A3(t0)
	sl a4, TF_A4(t0)
	sl a5, TF_A5(t0)
	sl a6, TF_A6(t0)
	sl a7, TF_A7(t0)
	sl s2, TF_S2(t0)
	sl s3, TF_S3(t0)
	sl s4, TF_S4(t0)
	sl s5, TF_S5(t0)
	sl s6, TF_S6(t0)
	sl s7, TF_S7(t0)
	sl s8, TF_S8(t0)
	sl s9, TF_S9(t0)
	sl s10, TF_S10(t0)
	sl s11, TF_S11(t0)
	sl t3, TF_T3(t0)
	sl t4, TF_T4(t0)
	sl t5, TF_T5(t0)
	sl t6, TF_T6(t0)
	csrr t1, sepc
	sl t1, TF_PC(t0)
	csrr t0, sstatus
	li t1, (1 << 8) /* SPP bit */
	and t0, t0, t1
	bnez t0, .Lpost_sp
	ll sp, 4 * __SIZEOF_POINTER__(tp)
.Lpost_sp:
	csrr a0, scause
	la t0, riscv_trap_handle
	jalr t0
