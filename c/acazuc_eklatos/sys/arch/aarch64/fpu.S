.type arch_save_fpu, %function
.global arch_save_fpu
arch_save_fpu:
	stp q0 , q1 , [x0, #0x000]
	stp q2 , q3 , [x0, #0x020]
	stp q4 , q5 , [x0, #0x040]
	stp q6 , q7 , [x0, #0x060]
	stp q8 , q9 , [x0, #0x080]
	stp q10, q11, [x0, #0x0A0]
	stp q12, q13, [x0, #0x0C0]
	stp q14, q15, [x0, #0x0E0]
	stp q16, q17, [x0, #0x100]
	stp q18, q19, [x0, #0x120]
	stp q20, q21, [x0, #0x140]
	stp q22, q23, [x0, #0x160]
	stp q24, q25, [x0, #0x180]
	stp q26, q27, [x0, #0x1A0]
	stp q28, q29, [x0, #0x1C0]
	stp q30, q31, [x0, #0x1E0]
	add x0, x0, #0x200
	mrs x1, fpcr
	mrs x2, fpsr
	stp x1, x2, [x0]
	ret

.type arch_load_fpu, %function
.global arch_load_fpu
arch_load_fpu:
	ldp q0 , q1 , [x0, #0x000]
	ldp q2 , q3 , [x0, #0x020]
	ldp q4 , q5 , [x0, #0x040]
	ldp q6 , q7 , [x0, #0x060]
	ldp q8 , q9 , [x0, #0x080]
	ldp q10, q11, [x0, #0x0A0]
	ldp q12, q13, [x0, #0x0C0]
	ldp q14, q15, [x0, #0x0E0]
	ldp q16, q17, [x0, #0x100]
	ldp q18, q19, [x0, #0x120]
	ldp q20, q21, [x0, #0x140]
	ldp q22, q23, [x0, #0x160]
	ldp q24, q25, [x0, #0x180]
	ldp q26, q27, [x0, #0x1A0]
	ldp q28, q29, [x0, #0x1C0]
	ldp q30, q31, [x0, #0x1E0]
	add x0, x0, #0x200
	ldp x1, x2, [x0]
	msr fpcr, x1
	msr fpsr, x2
	ret
