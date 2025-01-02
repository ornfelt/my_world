#include "instr.h"
#include "../cpu.h"
#include "../mem.h"

#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define THUMB_INSTR(name, exec_fn, print_fn) \
static void exec_##name(struct cpu *cpu) \
exec_fn \
static void print_##name(struct cpu *cpu, char *data, size_t size) \
{ \
	(void)cpu; \
	print_fn \
} \
static const struct cpu_instr thumb_##name = \
{ \
	.exec = exec_##name, \
	.print = print_##name, \
}

#define THUMB_LSL(v, s) (((s) >= 32) ? 0 : ((v) << (s)))
#define THUMB_LSR(v, s) (((s) >= 32) ? 0 : ((v) >> (s)))
#define THUMB_ASR(v, s) (((s) >= 32) ? (v & 0x80000000UL) : (uint32_t)((int32_t)(v) >> (s)))
#define THUMB_ROR(v, s) (((v) >> (s)) | ((v) << (32 - (s))))

static void update_flags_logical(struct cpu *cpu, uint32_t v)
{
	CPU_SET_FLAG_N(cpu, v & 0x80000000UL);
	CPU_SET_FLAG_Z(cpu, !v);
}

static void update_flags_add(struct cpu *cpu, uint32_t v, uint32_t op1, uint32_t op2)
{
	CPU_SET_FLAG_V(cpu, (~(op1 ^ op2) & (v ^ op2)) & 0x80000000UL);
	update_flags_logical(cpu, v);
}

static void update_flags_sub(struct cpu *cpu, uint32_t v, uint32_t op1, uint32_t op2)
{
	CPU_SET_FLAG_V(cpu, ((op1 ^ op2) & (v ^ op1)) & 0x80000000UL);
	update_flags_logical(cpu, v);
}

#define THUMB_SHIFTED(n, shiftop) \
THUMB_INSTR(n##_imm, \
{ \
	uint32_t shift = (cpu->instr_opcode >> 6) & 0x1F; \
	uint32_t rsr = (cpu->instr_opcode >> 3) & 0x7; \
	uint32_t rdr = (cpu->instr_opcode >> 0) & 0x7; \
	uint32_t rss = cpu_get_reg(cpu, rsr); \
	shiftop; \
	cpu_set_reg(cpu, rdr, rs); \
	update_flags_logical(cpu, rs); \
	cpu_inc_pc(cpu, 2); \
}, \
{ \
	uint32_t shift = (cpu->instr_opcode >> 6) & 0x1F; \
	uint32_t rsr = (cpu->instr_opcode >> 3) & 0x7; \
	uint32_t rdr = (cpu->instr_opcode >> 0) & 0x7; \
	snprintf(data, size, #n " r%" PRIu32 ", r%" PRIu32 ", #0x%" PRIx32, rdr, rsr, shift); \
})

#define THUMB_SHIFTED_LSL uint32_t rs = THUMB_LSL(rss, shift); if (shift) { CPU_SET_FLAG_C(cpu, rss & (1 << (32 - shift))); }
#define THUMB_SHIFTED_LSR shift = shift ? shift : 32; uint32_t rs = THUMB_LSR(rss, shift); CPU_SET_FLAG_C(cpu, rss & (1 << (shift - 1)));
#define THUMB_SHIFTED_ASR \
	uint32_t rs; \
	if (!shift) \
	{ \
		rs = rss & 0x80000000UL; \
		if (rs) \
		{ \
			rs = 0xFFFFFFFFUL; \
			CPU_SET_FLAG_C(cpu, 1); \
		} \
		else \
		{ \
			CPU_SET_FLAG_C(cpu, 0); \
		} \
	} \
	else \
	{ \
		rs = THUMB_ASR(rss, shift); \
		CPU_SET_FLAG_C(cpu, rss & (1 << (shift - 1)));  \
	} \

THUMB_SHIFTED(lsl, THUMB_SHIFTED_LSL);
THUMB_SHIFTED(lsr, THUMB_SHIFTED_LSR);
THUMB_SHIFTED(asr, THUMB_SHIFTED_ASR);

#define THUMB_ADDSUBR(n, v, add_sub, reg_imm) \
THUMB_INSTR(n##_##v, \
{ \
	uint32_t rsr = (cpu->instr_opcode >> 3) & 0x7; \
	uint32_t rdr = (cpu->instr_opcode >> 0) & 0x7; \
	uint32_t rs = cpu_get_reg(cpu, rsr); \
	uint32_t val; \
	if (reg_imm) \
		val = (cpu->instr_opcode >> 6) & 0x7; \
	else \
		val = cpu_get_reg(cpu, (cpu->instr_opcode >> 6) & 0x7); \
	if (add_sub) \
	{ \
		uint32_t res = rs - val; \
		cpu_set_reg(cpu, rdr, res); \
		update_flags_sub(cpu, res, rs, val); \
		CPU_SET_FLAG_C(cpu, val <= rs); \
	} \
	else \
	{ \
		uint32_t res = rs + val; \
		cpu_set_reg(cpu, rdr, res); \
		update_flags_add(cpu, res, rs, val); \
		CPU_SET_FLAG_C(cpu, res < rs); \
	} \
	cpu_inc_pc(cpu, 2); \
}, \
{ \
	uint32_t rsr = (cpu->instr_opcode >> 3) & 0x7; \
	uint32_t rdr = (cpu->instr_opcode >> 0) & 0x7; \
	uint32_t val = (cpu->instr_opcode >> 6) & 0x7; \
	if (reg_imm) \
		snprintf(data, size, #n " r%" PRIu32 ", r%" PRIu32 ", #0x%" PRIx32, rdr, rsr, val); \
	else \
		snprintf(data, size, #n " r%" PRIu32 ", r%" PRIu32 ", r%" PRIu32, rdr, rsr, val); \
})

THUMB_ADDSUBR(add, reg, 0, 0);
THUMB_ADDSUBR(sub, reg, 1, 0);
THUMB_ADDSUBR(add, imm, 0, 1);
THUMB_ADDSUBR(sub, imm, 1, 1);

static void mcas_mov(struct cpu *cpu, uint32_t r, uint32_t nn)
{
	cpu_set_reg(cpu, r, nn);
	update_flags_logical(cpu, nn);
}

static void mcas_cmp(struct cpu *cpu, uint32_t r, uint32_t nn)
{
	uint32_t rv = cpu_get_reg(cpu, r);
	uint32_t res = rv - nn;
	update_flags_sub(cpu, res, rv, nn);
	CPU_SET_FLAG_C(cpu, nn <= rv);
}

static void mcas_add(struct cpu *cpu, uint32_t r, uint32_t nn)
{
	uint32_t rv = cpu_get_reg(cpu, r);
	uint32_t res = rv + nn;
	cpu_set_reg(cpu, r, res);
	update_flags_add(cpu, res, rv, nn);
	CPU_SET_FLAG_C(cpu, res < nn);
}

static void mcas_sub(struct cpu *cpu, uint32_t r, uint32_t nn)
{
	uint32_t rv = cpu_get_reg(cpu, r);
	uint32_t res = rv - nn;
	cpu_set_reg(cpu, r, res);
	update_flags_sub(cpu, res, rv, nn);
	CPU_SET_FLAG_C(cpu, nn <= rv);
}

#define THUMB_MCAS_I8R(n, r) \
THUMB_INSTR(n##_i8r##r, \
{ \
	uint32_t nn = cpu->instr_opcode & 0xFF; \
	mcas_##n(cpu, r, nn); \
	cpu_inc_pc(cpu, 2); \
}, \
{ \
	uint32_t nn = cpu->instr_opcode & 0xFF; \
	snprintf(data, size, #n " r%d, #0x%" PRIx32, r, nn); \
})

THUMB_MCAS_I8R(mov, 0);
THUMB_MCAS_I8R(mov, 1);
THUMB_MCAS_I8R(mov, 2);
THUMB_MCAS_I8R(mov, 3);
THUMB_MCAS_I8R(mov, 4);
THUMB_MCAS_I8R(mov, 5);
THUMB_MCAS_I8R(mov, 6);
THUMB_MCAS_I8R(mov, 7);
THUMB_MCAS_I8R(cmp, 0);
THUMB_MCAS_I8R(cmp, 1);
THUMB_MCAS_I8R(cmp, 2);
THUMB_MCAS_I8R(cmp, 3);
THUMB_MCAS_I8R(cmp, 4);
THUMB_MCAS_I8R(cmp, 5);
THUMB_MCAS_I8R(cmp, 6);
THUMB_MCAS_I8R(cmp, 7);
THUMB_MCAS_I8R(add, 0);
THUMB_MCAS_I8R(add, 1);
THUMB_MCAS_I8R(add, 2);
THUMB_MCAS_I8R(add, 3);
THUMB_MCAS_I8R(add, 4);
THUMB_MCAS_I8R(add, 5);
THUMB_MCAS_I8R(add, 6);
THUMB_MCAS_I8R(add, 7);
THUMB_MCAS_I8R(sub, 0);
THUMB_MCAS_I8R(sub, 1);
THUMB_MCAS_I8R(sub, 2);
THUMB_MCAS_I8R(sub, 3);
THUMB_MCAS_I8R(sub, 4);
THUMB_MCAS_I8R(sub, 5);
THUMB_MCAS_I8R(sub, 6);
THUMB_MCAS_I8R(sub, 7);

static void alu_and(struct cpu *cpu, uint32_t rd, uint32_t rdr, uint32_t rs)
{
	uint32_t v = rd & rs;
	cpu_set_reg(cpu, rdr, v);
	update_flags_logical(cpu, v);
}

static void alu_eor(struct cpu *cpu, uint32_t rd, uint32_t rdr, uint32_t rs)
{
	uint32_t v = rd ^ rs;
	cpu_set_reg(cpu, rdr, v);
	update_flags_logical(cpu, v);
}

static void alu_lsr(struct cpu *cpu, uint32_t rd, uint32_t rdr, uint32_t rs)
{
	uint32_t v = THUMB_LSR(rd, rs);
	cpu_set_reg(cpu, rdr, v);
	update_flags_logical(cpu, v);
	if (rs)
		CPU_SET_FLAG_C(cpu, rd & (1 << (rs - 1)));
}

static void alu_lsl(struct cpu *cpu, uint32_t rd, uint32_t rdr, uint32_t rs)
{
	uint32_t v = THUMB_LSL(rd, rs);
	cpu_set_reg(cpu, rdr, v);
	update_flags_logical(cpu, v);
	if (rs)
		CPU_SET_FLAG_C(cpu, rd & (1 << (32 - rs)));
}

static void alu_asr(struct cpu *cpu, uint32_t rd, uint32_t rdr, uint32_t rs)
{
	uint32_t v = THUMB_ASR(rd, rs);
	cpu_set_reg(cpu, rdr, v);
	update_flags_logical(cpu, v);
	if (rs)
		CPU_SET_FLAG_C(cpu, rd & (1 << (32 - rs)));
}

static void alu_adc(struct cpu *cpu, uint32_t rd, uint32_t rdr, uint32_t rs)
{
	uint32_t c = CPU_GET_FLAG_C(cpu);
	uint32_t v = rd + rs + c;
	cpu_set_reg(cpu, rdr, v);
	update_flags_add(cpu, v, rd, rs + c);
	CPU_SET_FLAG_C(cpu, c ? v <= rd : v < rd);
}

static void alu_sbc(struct cpu *cpu, uint32_t rd, uint32_t rdr, uint32_t rs)
{
	uint32_t c = CPU_GET_FLAG_C(cpu);
	uint32_t v2 = rs + 1 - c;
	uint32_t v = rd - v2;
	cpu_set_reg(cpu, rdr, v);
	update_flags_sub(cpu, v, rd, v2);
	CPU_SET_FLAG_C(cpu, c ? rs <= rd : v < rd);
}

static void alu_ror(struct cpu *cpu, uint32_t rd, uint32_t rdr, uint32_t rs)
{
	uint32_t v = THUMB_ROR(rd, rs);
	cpu_set_reg(cpu, rdr, v);
	update_flags_logical(cpu, v);
	if (rs)
		CPU_SET_FLAG_C(cpu, rd & (1 << (rs - 1)));
}

static void alu_tst(struct cpu *cpu, uint32_t rd, uint32_t rdr, uint32_t rs)
{
	(void)rdr;
	uint32_t v = rd & rs;
	update_flags_logical(cpu, v);
}

static void alu_neg(struct cpu *cpu, uint32_t rd, uint32_t rdr, uint32_t rs)
{
	(void)rd;
	uint32_t v = -rs;
	cpu_set_reg(cpu, rdr, v);
	update_flags_sub(cpu, v, 0, rs);
	CPU_SET_FLAG_C(cpu, rs <= 0);
}

static void alu_cmp(struct cpu *cpu, uint32_t rd, uint32_t rdr, uint32_t rs)
{
	(void)rdr;
	uint32_t v = rd - rs;
	update_flags_sub(cpu, v, rd, rs);
	CPU_SET_FLAG_C(cpu, rs <= rd);
}

static void alu_cmn(struct cpu *cpu, uint32_t rd, uint32_t rdr, uint32_t rs)
{
	(void)rdr;
	uint32_t v = rd + rs;
	update_flags_add(cpu, v, rd, rs);
	CPU_SET_FLAG_C(cpu, v < rd);
}

static void alu_orr(struct cpu *cpu, uint32_t rd, uint32_t rdr, uint32_t rs)
{
	uint32_t v = rd | rs;
	cpu_set_reg(cpu, rdr, v);
	update_flags_logical(cpu, v);
}

static void alu_mul(struct cpu *cpu, uint32_t rd, uint32_t rdr, uint32_t rs)
{
	uint32_t v = rd * rs;
	cpu_set_reg(cpu, rdr, v);
	update_flags_logical(cpu, v);
	if (!cpu->arm9)
		CPU_SET_FLAG_C(cpu, 0);
}

static void alu_bic(struct cpu *cpu, uint32_t rd, uint32_t rdr, uint32_t rs)
{
	uint32_t v = rd & ~rs;
	cpu_set_reg(cpu, rdr, v);
	update_flags_logical(cpu, v);
}

static void alu_mvn(struct cpu *cpu, uint32_t rd, uint32_t rdr, uint32_t rs)
{
	(void)rd;
	uint32_t v = ~rs;
	cpu_set_reg(cpu, rdr, v);
	update_flags_logical(cpu, v);
}

#define THUMB_ALU(n) \
THUMB_INSTR(alu_##n, \
{ \
	uint32_t rdr = (cpu->instr_opcode >> 0) & 0x7; \
	uint32_t rsr = (cpu->instr_opcode >> 3) & 0x7; \
	uint32_t rd = cpu_get_reg(cpu, rdr); \
	if (rdr == CPU_REG_PC) \
		rd += 4; \
	uint32_t rs = cpu_get_reg(cpu, rsr); \
	if (rsr == CPU_REG_PC) \
		rs += 4; \
	alu_##n(cpu, rd, rdr, rs); \
	cpu_inc_pc(cpu, 2); \
}, \
{ \
	uint32_t rd = (cpu->instr_opcode >> 0) & 0x7; \
	uint32_t rs = (cpu->instr_opcode >> 3) & 0x7; \
	snprintf(data, size, #n " r%" PRIu32 ", r%" PRIu32, rd, rs); \
})

THUMB_ALU(and);
THUMB_ALU(eor);
THUMB_ALU(lsr);
THUMB_ALU(lsl);
THUMB_ALU(asr);
THUMB_ALU(adc);
THUMB_ALU(sbc);
THUMB_ALU(ror);
THUMB_ALU(tst);
THUMB_ALU(neg);
THUMB_ALU(cmp);
THUMB_ALU(cmn);
THUMB_ALU(orr);
THUMB_ALU(mul);
THUMB_ALU(bic);
THUMB_ALU(mvn);

static void hi_addh(struct cpu *cpu, uint32_t rd, uint32_t rdr, uint32_t rs)
{
	if (rdr == CPU_REG_PC)
	{
		cpu_set_reg(cpu, rdr, (rd + rs) & ~1);
		cpu->instr_delay += 2;
	}
	else
	{
		cpu_set_reg(cpu, rdr, rd + rs);
		cpu_inc_pc(cpu, 2);
	}
}

static void hi_cmph(struct cpu *cpu, uint32_t rd, uint32_t rdr, uint32_t rs)
{
	(void)rdr;
	uint32_t res = rd - rs;
	update_flags_sub(cpu, res, rd, rs);
	CPU_SET_FLAG_C(cpu, rs <= rd);
	cpu_inc_pc(cpu, 2);
}

static void hi_movh(struct cpu *cpu, uint32_t rd, uint32_t rdr, uint32_t rs)
{
	(void)rd;
	if (rdr == CPU_REG_PC)
	{
		cpu_set_reg(cpu, rdr, rs & ~1);
	}
	else
	{
		cpu_set_reg(cpu, rdr, rs);
		cpu_inc_pc(cpu, 2);
	}
}

#define THUMB_HI(n) \
THUMB_INSTR(n, \
{ \
	uint32_t msbd = (cpu->instr_opcode >> 7) & 0x1; \
	uint32_t msbs = (cpu->instr_opcode >> 6) & 0x1; \
	uint32_t rdr = ((cpu->instr_opcode >> 0) & 0x7) | (msbd << 3); \
	uint32_t rsr = ((cpu->instr_opcode >> 3) & 0x7) | (msbs << 3); \
	uint32_t rd = cpu_get_reg(cpu, rdr); \
	rd += (rdr == CPU_REG_PC) ? 4 : 0; \
	uint32_t rs = cpu_get_reg(cpu, rsr); \
	rs += (rsr == CPU_REG_PC) ? 4 : 0; \
	hi_##n(cpu, rd, rdr, rs); \
}, \
{ \
	uint32_t msbd = (cpu->instr_opcode >> 7) & 0x1; \
	uint32_t msbs = (cpu->instr_opcode >> 6) & 0x1; \
	uint32_t rd = ((cpu->instr_opcode >> 0) & 0x7) | (msbd << 3); \
	uint32_t rs = ((cpu->instr_opcode >> 3) & 0x7) | (msbs << 3); \
	snprintf(data, size, #n " r%" PRIu32 ", r%" PRIu32, rd, rs); \
})

THUMB_HI(addh);
THUMB_HI(cmph);
THUMB_HI(movh);

THUMB_INSTR(bx,
{
	uint32_t msbd = (cpu->instr_opcode >> 7) & 0x1;
	uint32_t msbs = (cpu->instr_opcode >> 6) & 0x1;
	uint32_t rsr = ((cpu->instr_opcode >> 3) & 0x7) | (msbs << 3);
	uint32_t rs = cpu_get_reg(cpu, rsr);
	rs += (rsr == CPU_REG_PC) ? 4 : 0;
	if (!(rs & 1))
	{
		CPU_SET_FLAG_T(cpu, 0);
		rs &= ~3;
	}
	else
	{
		rs &= ~1;
	}
	if (msbd)
		cpu_set_reg(cpu, CPU_REG_LR, (cpu_get_reg(cpu, CPU_REG_PC) + 2) | 1);
	cpu_set_reg(cpu, CPU_REG_PC, rs);
},
{
	uint32_t msbs = (cpu->instr_opcode >> 6) & 0x1;
	uint32_t rs = ((cpu->instr_opcode >> 3) & 0x7) | (msbs << 3);
	snprintf(data, size, "bx r%" PRIu32, rs);
});

#define THUMB_LDRPC_R(r) \
THUMB_INSTR(ldrpc_r##r, \
{ \
	uint32_t rd = (cpu->instr_opcode >> 8) & 0x7; \
	uint32_t nn = cpu->instr_opcode & 0xFF; \
	uint32_t addr = ((cpu_get_reg(cpu, CPU_REG_PC) + 4) & ~2) + nn * 4; \
	uint32_t v = cpu->get32(cpu->mem, addr, MEM_DATA_NSEQ); \
	cpu_set_reg(cpu, rd, v); \
	cpu_inc_pc(cpu, 2); \
	cpu->instr_delay++; \
}, \
{ \
	uint32_t rd = (cpu->instr_opcode >> 8) & 0x7; \
	uint32_t nn = cpu->instr_opcode & 0xFF; \
	snprintf(data, size, "ldr r%" PRIu32 ", [pc, #0x%" PRIx32 "]", rd, nn * 4); \
})

THUMB_LDRPC_R(0);
THUMB_LDRPC_R(1);
THUMB_LDRPC_R(2);
THUMB_LDRPC_R(3);
THUMB_LDRPC_R(4);
THUMB_LDRPC_R(5);
THUMB_LDRPC_R(6);
THUMB_LDRPC_R(7);

static void stld_str(struct cpu *cpu, uint32_t rd, uint32_t rb, uint32_t ro)
{
	cpu->set32(cpu->mem, rb + ro, cpu_get_reg(cpu, rd), MEM_DATA_NSEQ);
}

static void stld_strh(struct cpu *cpu, uint32_t rd, uint32_t rb, uint32_t ro)
{
	cpu->set16(cpu->mem, rb + ro, cpu_get_reg(cpu, rd), MEM_DATA_NSEQ);
}

static void stld_strb(struct cpu *cpu, uint32_t rd, uint32_t rb, uint32_t ro)
{
	cpu->set8(cpu->mem, rb + ro, cpu_get_reg(cpu, rd), MEM_DATA_NSEQ);
}

static void stld_ldr(struct cpu *cpu, uint32_t rd, uint32_t rb, uint32_t ro)
{
	uint32_t addr = rb + ro;
	uint32_t v = cpu->get32(cpu->mem, addr, MEM_DATA_NSEQ);
	cpu_set_reg(cpu, rd, THUMB_ROR(v, (addr & 3) * 8));
}

static void stld_ldrh(struct cpu *cpu, uint32_t rd, uint32_t rb, uint32_t ro)
{
	uint32_t addr = rb + ro;
	uint32_t v = cpu->get16(cpu->mem, addr, MEM_DATA_NSEQ);
	cpu_set_reg(cpu, rd, THUMB_ROR(v, (addr & 1) * 8));
}

static void stld_ldrb(struct cpu *cpu, uint32_t rd, uint32_t rb, uint32_t ro)
{
	cpu_set_reg(cpu, rd, cpu->get8(cpu->mem, rb + ro, MEM_DATA_NSEQ));
}

static void stld_ldrsh(struct cpu *cpu, uint32_t rd, uint32_t rb, uint32_t ro)
{
	uint32_t addr = rb + ro;
	uint32_t v;
	if (addr & 1)
		v = (int8_t)cpu->get8(cpu->mem, addr, MEM_DATA_NSEQ);
	else
		v = (int16_t)cpu->get16(cpu->mem, addr, MEM_DATA_NSEQ);
	cpu_set_reg(cpu, rd, v);
}

static void stld_ldrsb(struct cpu *cpu, uint32_t rd, uint32_t rb, uint32_t ro)
{
	cpu_set_reg(cpu, rd, (int8_t)cpu->get8(cpu->mem, rb + ro, MEM_DATA_NSEQ));
}

#define THUMB_STLD_REG(n, st_ld) \
THUMB_INSTR(n##_reg, \
{ \
	uint32_t rd = (cpu->instr_opcode >> 0) & 0x7; \
	uint32_t rb = (cpu->instr_opcode >> 3) & 0x7; \
	uint32_t ro = (cpu->instr_opcode >> 6) & 0x7; \
	stld_##n(cpu, rd, cpu_get_reg(cpu, rb), cpu_get_reg(cpu, ro)); \
	cpu_inc_pc(cpu, 2); \
	if (st_ld) \
		cpu->instr_delay++; \
}, \
{ \
	uint32_t rd = (cpu->instr_opcode >> 0) & 0x7; \
	uint32_t rb = (cpu->instr_opcode >> 3) & 0x7; \
	uint32_t ro = (cpu->instr_opcode >> 6) & 0x7; \
	snprintf(data, size, #n " r%" PRIu32 ", [r%" PRIu32 ", r%" PRIu32 "]", rd, rb, ro); \
})

THUMB_STLD_REG(str  , 0);
THUMB_STLD_REG(strh , 0);
THUMB_STLD_REG(strb , 0);
THUMB_STLD_REG(ldr  , 1);
THUMB_STLD_REG(ldrh , 1);
THUMB_STLD_REG(ldrb , 1);
THUMB_STLD_REG(ldrsh, 1);
THUMB_STLD_REG(ldrsb, 1);

#define THUMB_STLD_IMM(n, st_ld, nn_mult) \
THUMB_INSTR(n##_imm, \
{ \
	uint32_t rd = (cpu->instr_opcode >> 0) & 0x7; \
	uint32_t rb = (cpu->instr_opcode >> 3) & 0x7; \
	uint32_t nn = (cpu->instr_opcode >> 6) & 0x1F; \
	nn *= nn_mult; \
	stld_##n(cpu, rd, cpu_get_reg(cpu, rb), nn); \
	cpu_inc_pc(cpu, 2); \
	if (st_ld) \
		cpu->instr_delay++; \
}, \
{ \
	uint32_t rd = (cpu->instr_opcode >> 0) & 0x7; \
	uint32_t rb = (cpu->instr_opcode >> 3) & 0x7; \
	uint32_t nn = (cpu->instr_opcode >> 6) & 0x1F; \
	snprintf(data, size, #n " r%" PRIu32 ", [r%" PRIu32 ", #0x%" PRIx32 "]", rd, rb, nn * nn_mult); \
})

THUMB_STLD_IMM(str , 0, 4);
THUMB_STLD_IMM(strh, 0, 2);
THUMB_STLD_IMM(strb, 0, 1);
THUMB_STLD_IMM(ldr , 1, 4);
THUMB_STLD_IMM(ldrh, 1, 2);
THUMB_STLD_IMM(ldrb, 1, 1);

#define THUMB_STLDSP_R(n, r, st_ld) \
THUMB_INSTR(n##sp_r##r, \
{ \
	uint32_t rd = (cpu->instr_opcode >> 8) & 0x7; \
	uint32_t nn = ((cpu->instr_opcode >> 0) & 0xFF) * 4; \
	if (st_ld) \
	{ \
		uint32_t addr = cpu_get_reg(cpu, CPU_REG_SP) + nn; \
		uint32_t v = cpu->get32(cpu->mem, addr, MEM_DATA_NSEQ); \
		cpu_set_reg(cpu, rd, THUMB_ROR(v, (addr & 3) * 8)); \
		cpu->instr_delay++; \
	} \
	else \
	{ \
		cpu->set32(cpu->mem, cpu_get_reg(cpu, CPU_REG_SP) + nn, cpu_get_reg(cpu, rd), MEM_DATA_NSEQ); \
	} \
	cpu_inc_pc(cpu, 2); \
}, \
{ \
	uint32_t rd = (cpu->instr_opcode >> 8) & 0x7; \
	uint32_t nn = (cpu->instr_opcode >> 0) & 0xFF; \
	snprintf(data, size, #n " r%" PRIu32 ", [sp, #0x%" PRIx32 "]", rd, nn * 4); \
})

THUMB_STLDSP_R(str, 0, 0);
THUMB_STLDSP_R(str, 1, 0);
THUMB_STLDSP_R(str, 2, 0);
THUMB_STLDSP_R(str, 3, 0);
THUMB_STLDSP_R(str, 4, 0);
THUMB_STLDSP_R(str, 5, 0);
THUMB_STLDSP_R(str, 6, 0);
THUMB_STLDSP_R(str, 7, 0);
THUMB_STLDSP_R(ldr, 0, 1);
THUMB_STLDSP_R(ldr, 1, 1);
THUMB_STLDSP_R(ldr, 2, 1);
THUMB_STLDSP_R(ldr, 3, 1);
THUMB_STLDSP_R(ldr, 4, 1);
THUMB_STLDSP_R(ldr, 5, 1);
THUMB_STLDSP_R(ldr, 6, 1);
THUMB_STLDSP_R(ldr, 7, 1);

#define THUMB_ADDPCSP_R(rr, r, pc_sp) \
THUMB_INSTR(add##rr##_r##r, \
{ \
	uint32_t rd = (cpu->instr_opcode >> 8) & 0x7; \
	uint32_t nn = ((cpu->instr_opcode >> 0) & 0xFF) * 4; \
	if (pc_sp) \
		cpu_set_reg(cpu, rd, cpu_get_reg(cpu, CPU_REG_SP) + nn); \
	else \
		cpu_set_reg(cpu, rd, ((cpu_get_reg(cpu, CPU_REG_PC) + 4) & ~2) + nn); \
	cpu_inc_pc(cpu, 2); \
}, \
{ \
	uint32_t rd = (cpu->instr_opcode >> 8) & 0x7; \
	uint32_t nn = (cpu->instr_opcode >> 0) & 0xFF; \
	snprintf(data, size, "add r%" PRIu32 ", " #rr ", #0x%" PRIx32, rd, nn * 4); \
})

THUMB_ADDPCSP_R(pc, 0, 0);
THUMB_ADDPCSP_R(pc, 1, 0);
THUMB_ADDPCSP_R(pc, 2, 0);
THUMB_ADDPCSP_R(pc, 3, 0);
THUMB_ADDPCSP_R(pc, 4, 0);
THUMB_ADDPCSP_R(pc, 5, 0);
THUMB_ADDPCSP_R(pc, 6, 0);
THUMB_ADDPCSP_R(pc, 7, 0);
THUMB_ADDPCSP_R(sp, 0, 1);
THUMB_ADDPCSP_R(sp, 1, 1);
THUMB_ADDPCSP_R(sp, 2, 1);
THUMB_ADDPCSP_R(sp, 3, 1);
THUMB_ADDPCSP_R(sp, 4, 1);
THUMB_ADDPCSP_R(sp, 5, 1);
THUMB_ADDPCSP_R(sp, 6, 1);
THUMB_ADDPCSP_R(sp, 7, 1);

THUMB_INSTR(addsp_imm,
{
	uint32_t add_sub = (cpu->instr_opcode >> 7) & 0x1;
	uint32_t nn = (cpu->instr_opcode >> 0) & 0x7F;
	if (add_sub)
		cpu_set_reg(cpu, CPU_REG_SP, cpu_get_reg(cpu, CPU_REG_SP) - nn * 4);
	else
		cpu_set_reg(cpu, CPU_REG_SP, cpu_get_reg(cpu, CPU_REG_SP) + nn * 4);
	cpu_inc_pc(cpu, 2);
},
{
	uint32_t add_sub = (cpu->instr_opcode >> 7) & 0x1;
	uint32_t nn = (cpu->instr_opcode >> 0) & 0x7F;
	snprintf(data, size, "%s sp, #0x%" PRIx32, add_sub ? "sub" : "add", nn * 4);
});

#define THUMB_PUSHPOP(n, next, post_pre, down_up, st_ld, ext_reg, base_reg) \
THUMB_INSTR(n####next, \
{ \
	uint32_t rl = cpu->instr_opcode & 0xFF; \
	uint32_t sp = cpu_get_reg(cpu, base_reg); \
	bool pc_inc = true; \
	uint32_t old_base = sp; \
	uint32_t new_base; \
	uint32_t nregs = 0; \
	for (int i = 0; i < 8; ++i) \
	{ \
			if (rl & (1 << i)) \
			nregs++; \
	} \
	if (ext_reg) \
		nregs++; \
	else if (!nregs) \
		nregs = 0x10; \
	if (!down_up) \
	{ \
		sp -= 4 * nregs; \
		cpu_set_reg(cpu, base_reg, sp); \
		new_base = sp; \
	} \
	else \
	{ \
		new_base = old_base + nregs * 4; \
	} \
	bool nowriteback = false; \
	if (rl) \
	{ \
		bool first = true; \
		for (int i = 0; i < 8; ++i) \
		{ \
			if (!(rl & (1 << i))) \
				continue; \
			if (post_pre == down_up) \
				sp += 4; \
			if (st_ld) \
			{ \
				if (i == base_reg) \
					nowriteback = true; \
				uint32_t v = cpu->get32(cpu->mem, sp, MEM_DATA_SEQ); \
				cpu_set_reg(cpu, i, v); \
			} \
			else \
			{ \
				uint32_t v; \
				if (i == base_reg) \
				{ \
					if (first) \
						v = old_base; \
					else \
						v = new_base; \
				} \
				else \
				{ \
					v = cpu_get_reg(cpu, i); \
				} \
				cpu->set32(cpu->mem, sp, v, MEM_DATA_SEQ); \
			} \
			if (post_pre != down_up) \
				sp += 4; \
			first = false; \
		} \
	} \
	if (ext_reg) \
	{ \
		if (post_pre == down_up) \
			sp += 4; \
		if (st_ld) \
		{ \
			uint32_t v = cpu->get32(cpu->mem, sp, MEM_DATA_SEQ); \
			if (cpu->arm9) \
			{ \
				CPU_SET_FLAG_T(cpu, v & 1); \
				if (v & 1) \
					v &= ~1; \
				else \
					v &= ~3; \
			} \
			else \
			{ \
				v &= ~1; \
			} \
			cpu_set_reg(cpu, CPU_REG_PC, v); \
			pc_inc = false; \
		} \
		else \
		{ \
			cpu->set32(cpu->mem, sp, cpu_get_reg(cpu, CPU_REG_LR), MEM_DATA_SEQ); \
		} \
		if (post_pre != down_up) \
			sp += 4; \
	} \
	else if (!rl) \
	{ \
		if (post_pre == down_up) \
			sp += 0x4; \
		if (st_ld) \
		{ \
			cpu_set_reg(cpu, CPU_REG_PC, cpu->get32(cpu->mem, sp, MEM_DATA_SEQ) & ~1); \
			pc_inc = false; \
		} \
		else \
		{ \
			cpu->set32(cpu->mem, sp, cpu_get_reg(cpu, CPU_REG_PC) + 6, MEM_DATA_SEQ); \
		} \
		if (post_pre != down_up) \
			sp += 0x4; \
		sp += 0x3C; \
	} \
	if (!nowriteback && down_up) \
		cpu_set_reg(cpu, base_reg, sp); \
	if (pc_inc) \
		cpu_inc_pc(cpu, 2); \
}, \
{ \
	uint32_t regs = cpu->instr_opcode & 0xFF; \
	char  *tmpd = data; \
	size_t tmps = size; \
	snprintf(data, size, #n " "); \
	tmpd += (1 + strlen(#n)); \
	tmps -= (1 + strlen(#n)); \
	if (base_reg != CPU_REG_SP) \
	{ \
		snprintf(tmpd, tmps, "r%d!, ", base_reg); \
		tmpd += 5; \
		tmps -= 5; \
	} \
	snprintf(tmpd, tmps, "{"); \
	tmpd++; \
	tmps--; \
	if (regs) \
	{ \
		for (int i = 0; i < 8; ++i) \
		{ \
			if (!(regs & (1 << i))) \
				continue; \
			snprintf(tmpd, tmps, "r%d,", i); \
			tmpd += 3; \
			tmps -= 3; \
		} \
		tmps++; \
		tmpd--; \
	} \
	if (ext_reg) \
	{ \
		if (regs) \
		{ \
			tmps--; \
			tmpd++; \
		} \
		if (st_ld) \
		{ \
			snprintf(tmpd, tmps, "r%d", CPU_REG_PC); \
			tmpd += 3; \
			tmps -= 3; \
		} \
		else \
		{ \
			snprintf(tmpd, tmps, "r%d", CPU_REG_LR); \
			tmpd += 3; \
			tmps -= 3; \
		} \
	} \
	snprintf(tmpd, tmps, "}"); \
})

THUMB_PUSHPOP(push,    , 1, 0, 0, 0, CPU_REG_SP);
THUMB_PUSHPOP(pop ,    , 0, 1, 1, 0, CPU_REG_SP);
THUMB_PUSHPOP(push, _lr, 1, 0, 0, 1, CPU_REG_SP);
THUMB_PUSHPOP(pop , _pc, 0, 1, 1, 1, CPU_REG_SP);
THUMB_PUSHPOP(stm , _r0, 0, 1, 0, 0, 0);
THUMB_PUSHPOP(stm , _r1, 0, 1, 0, 0, 1);
THUMB_PUSHPOP(stm , _r2, 0, 1, 0, 0, 2);
THUMB_PUSHPOP(stm , _r3, 0, 1, 0, 0, 3);
THUMB_PUSHPOP(stm , _r4, 0, 1, 0, 0, 4);
THUMB_PUSHPOP(stm , _r5, 0, 1, 0, 0, 5);
THUMB_PUSHPOP(stm , _r6, 0, 1, 0, 0, 6);
THUMB_PUSHPOP(stm , _r7, 0, 1, 0, 0, 7);
THUMB_PUSHPOP(ldm , _r0, 0, 1, 1, 0, 0);
THUMB_PUSHPOP(ldm , _r1, 0, 1, 1, 0, 1);
THUMB_PUSHPOP(ldm , _r2, 0, 1, 1, 0, 2);
THUMB_PUSHPOP(ldm , _r3, 0, 1, 1, 0, 3);
THUMB_PUSHPOP(ldm , _r4, 0, 1, 1, 0, 4);
THUMB_PUSHPOP(ldm , _r5, 0, 1, 1, 0, 5);
THUMB_PUSHPOP(ldm , _r6, 0, 1, 1, 0, 6);
THUMB_PUSHPOP(ldm , _r7, 0, 1, 1, 0, 7);

THUMB_INSTR(bkpt,
{
	(void)cpu;
	assert(!"unimp");
},
{
	snprintf(data, size, "bkpt");
});

#define THUMB_BRANCH(n, cond) \
THUMB_INSTR(n, \
{ \
	if (cond) \
	{ \
		int8_t nn = cpu->instr_opcode & 0xFF; \
		cpu_inc_pc(cpu, (int32_t)nn * 2 + 4); \
		cpu->has_next_thumb = 0; \
	} \
	else \
	{ \
		cpu_inc_pc(cpu, 2); \
	} \
}, \
{ \
	uint32_t nn = cpu->instr_opcode & 0xFF; \
	snprintf(data, size, #n " #0x%" PRIx32, nn * 2); \
})

THUMB_BRANCH(beq, CPU_GET_FLAG_Z(cpu));
THUMB_BRANCH(bne, !CPU_GET_FLAG_Z(cpu));
THUMB_BRANCH(bcs, CPU_GET_FLAG_C(cpu));
THUMB_BRANCH(bcc, !CPU_GET_FLAG_C(cpu));
THUMB_BRANCH(bmi, CPU_GET_FLAG_N(cpu));
THUMB_BRANCH(bpl, !CPU_GET_FLAG_N(cpu));
THUMB_BRANCH(bvs, CPU_GET_FLAG_V(cpu));
THUMB_BRANCH(bvc, !CPU_GET_FLAG_V(cpu));
THUMB_BRANCH(bhi, CPU_GET_FLAG_C(cpu) && !CPU_GET_FLAG_Z(cpu));
THUMB_BRANCH(bls, !CPU_GET_FLAG_C(cpu) || CPU_GET_FLAG_Z(cpu));
THUMB_BRANCH(bge, CPU_GET_FLAG_N(cpu) == CPU_GET_FLAG_V(cpu));
THUMB_BRANCH(blt, CPU_GET_FLAG_N(cpu) != CPU_GET_FLAG_V(cpu));
THUMB_BRANCH(bgt, !CPU_GET_FLAG_Z(cpu) && CPU_GET_FLAG_N(cpu) == CPU_GET_FLAG_V(cpu));
THUMB_BRANCH(ble, CPU_GET_FLAG_Z(cpu) || CPU_GET_FLAG_N(cpu) != CPU_GET_FLAG_V(cpu));

THUMB_INSTR(swi,
{
	cpu->regs.spsr_modes[1] = cpu->regs.cpsr;
	CPU_SET_MODE(cpu, CPU_MODE_SVC);
	cpu_update_mode(cpu);
	CPU_SET_FLAG_I(cpu, 1);
	CPU_SET_FLAG_T(cpu, 0);
	cpu_set_reg(cpu, CPU_REG_LR, cpu_get_reg(cpu, CPU_REG_PC) + 2);
	if (cpu->arm9 && (cpu->cp15.cr & 0x2000))
		cpu_set_reg(cpu, CPU_REG_PC, 0xFFFF0008);
	else
		cpu_set_reg(cpu, CPU_REG_PC, 0x8);
},
{
	uint32_t nn = cpu->instr_opcode & 0xFF;
	snprintf(data, size, "swi 0x%" PRIx32, nn);
});

THUMB_INSTR(b,
{
	int32_t nn = cpu->instr_opcode & 0x7FF;
	if (nn & 0x400)
		nn = -(~nn & 0x3FF) - 1;
	nn *= 2;
	cpu_set_reg(cpu, CPU_REG_PC, cpu_get_reg(cpu, CPU_REG_PC) + 4 + nn);
},
{
	int32_t nn = cpu->instr_opcode & 0x7FF;
	if (nn & 0x400)
		nn = -(~nn & 0x3FF) - 1;
	nn *= 2;
	snprintf(data, size, "b #%s0x%" PRIx32, nn < 0 ? "-" : "", nn < 0 ? -nn : nn);
});

THUMB_INSTR(blx_off,
{
	uint32_t nn = cpu->instr_opcode & 0x7FE;
	uint32_t pc = cpu_get_reg(cpu, CPU_REG_PC);
	uint32_t lr = (pc + 2) | 1;
	int32_t dst = (cpu_get_reg(cpu, CPU_REG_LR) & ~3) + (nn << 1);
	cpu_set_reg(cpu, CPU_REG_PC, dst);
	cpu_set_reg(cpu, CPU_REG_LR, lr);
	CPU_SET_FLAG_T(cpu, 0);
},
{
	uint32_t nn = cpu->instr_opcode & 0x7FF;
	snprintf(data, size, "blx #0x%" PRIx32, nn);
});

THUMB_INSTR(bl_setup,
{
	int32_t nn = cpu->instr_opcode & 0x7FF;
	if (nn & 0x400)
		nn = -(~nn & 0x3FF) - 1;
	cpu_set_reg(cpu, CPU_REG_LR, cpu_get_reg(cpu, CPU_REG_PC) + 4 + (nn * (1 << 12)));
	cpu_inc_pc(cpu, 2);
},
{
	uint32_t nn = cpu->instr_opcode & 0x7FF;
	snprintf(data, size, "bl #0x%" PRIx32, nn);
});

THUMB_INSTR(bl_off,
{
	uint32_t nn = cpu->instr_opcode & 0x7FF;
	uint32_t pc = cpu_get_reg(cpu, CPU_REG_PC);
	uint32_t lr = (pc + 2) | 1;
	int32_t dst = (cpu_get_reg(cpu, CPU_REG_LR) & ~1) + (nn << 1);
	cpu_set_reg(cpu, CPU_REG_PC, dst);
	cpu_set_reg(cpu, CPU_REG_LR, lr);
},
{
	uint32_t nn = cpu->instr_opcode & 0x7FF;
	snprintf(data, size, "bl #0x%" PRIx32, nn);
});

THUMB_INSTR(undef,
{
	(void)cpu;
	assert(!"unimp");
},
{
	snprintf(data, size, "undef");
});

#define REPEAT1(v) &thumb_##v
#define REPEAT2(v) REPEAT1(v), REPEAT1(v)
#define REPEAT4(v) REPEAT2(v), REPEAT2(v)
#define REPEAT8(v) REPEAT4(v), REPEAT4(v)
#define REPEAT16(v) REPEAT8(v), REPEAT8(v)
#define REPEAT32(v) REPEAT16(v), REPEAT16(v)

const struct cpu_instr *cpu_instr_thumb[0x400] =
{
	/* 0x000 */ REPEAT32(lsl_imm),
	/* 0x020 */ REPEAT32(lsr_imm),
	/* 0x040 */ REPEAT32(asr_imm),
	/* 0x060 */ REPEAT8(add_reg),
	/* 0x068 */ REPEAT8(sub_reg),
	/* 0x070 */ REPEAT8(add_imm),
	/* 0x078 */ REPEAT8(sub_imm),
	/* 0x080 */ REPEAT4(mov_i8r0),
	/* 0x084 */ REPEAT4(mov_i8r1),
	/* 0x088 */ REPEAT4(mov_i8r2),
	/* 0x08C */ REPEAT4(mov_i8r3),
	/* 0x090 */ REPEAT4(mov_i8r4),
	/* 0x094 */ REPEAT4(mov_i8r5),
	/* 0x098 */ REPEAT4(mov_i8r6),
	/* 0x09C */ REPEAT4(mov_i8r7),
	/* 0x0A0 */ REPEAT4(cmp_i8r0),
	/* 0x0A4 */ REPEAT4(cmp_i8r1),
	/* 0x0A8 */ REPEAT4(cmp_i8r2),
	/* 0x0AC */ REPEAT4(cmp_i8r3),
	/* 0x0B0 */ REPEAT4(cmp_i8r4),
	/* 0x0B4 */ REPEAT4(cmp_i8r5),
	/* 0x0B8 */ REPEAT4(cmp_i8r6),
	/* 0x0BC */ REPEAT4(cmp_i8r7),
	/* 0x0C0 */ REPEAT4(add_i8r0),
	/* 0x0C4 */ REPEAT4(add_i8r1),
	/* 0x0C8 */ REPEAT4(add_i8r2),
	/* 0x0CC */ REPEAT4(add_i8r3),
	/* 0x0D0 */ REPEAT4(add_i8r4),
	/* 0x0D4 */ REPEAT4(add_i8r5),
	/* 0x0D8 */ REPEAT4(add_i8r6),
	/* 0x0DC */ REPEAT4(add_i8r7),
	/* 0x0E0 */ REPEAT4(sub_i8r0),
	/* 0x0E4 */ REPEAT4(sub_i8r1),
	/* 0x0E8 */ REPEAT4(sub_i8r2),
	/* 0x0EC */ REPEAT4(sub_i8r3),
	/* 0x0F0 */ REPEAT4(sub_i8r4),
	/* 0x0F4 */ REPEAT4(sub_i8r5),
	/* 0x0F8 */ REPEAT4(sub_i8r6),
	/* 0x0FC */ REPEAT4(sub_i8r7),
	/* 0x100 */ REPEAT1(alu_and),
	/* 0x101 */ REPEAT1(alu_eor),
	/* 0x102 */ REPEAT1(alu_lsl),
	/* 0x103 */ REPEAT1(alu_lsr),
	/* 0x104 */ REPEAT1(alu_asr),
	/* 0x105 */ REPEAT1(alu_adc),
	/* 0x106 */ REPEAT1(alu_sbc),
	/* 0x107 */ REPEAT1(alu_ror),
	/* 0x108 */ REPEAT1(alu_tst),
	/* 0x109 */ REPEAT1(alu_neg),
	/* 0x10A */ REPEAT1(alu_cmp),
	/* 0x10B */ REPEAT1(alu_cmn),
	/* 0x10C */ REPEAT1(alu_orr),
	/* 0x10D */ REPEAT1(alu_mul),
	/* 0x10E */ REPEAT1(alu_bic),
	/* 0x10F */ REPEAT1(alu_mvn),
	/* 0x110 */ REPEAT4(addh),
	/* 0x114 */ REPEAT4(cmph),
	/* 0x118 */ REPEAT4(movh),
	/* 0x11C */ REPEAT4(bx),
	/* 0x120 */ REPEAT4(ldrpc_r0),
	/* 0x124 */ REPEAT4(ldrpc_r1),
	/* 0x128 */ REPEAT4(ldrpc_r2),
	/* 0x12C */ REPEAT4(ldrpc_r3),
	/* 0x130 */ REPEAT4(ldrpc_r4),
	/* 0x134 */ REPEAT4(ldrpc_r5),
	/* 0x138 */ REPEAT4(ldrpc_r6),
	/* 0x13C */ REPEAT4(ldrpc_r7),
	/* 0x140 */ REPEAT8(str_reg),
	/* 0x148 */ REPEAT8(strh_reg),
	/* 0x150 */ REPEAT8(strb_reg),
	/* 0x158 */ REPEAT8(ldrsb_reg),
	/* 0x160 */ REPEAT8(ldr_reg),
	/* 0x168 */ REPEAT8(ldrh_reg),
	/* 0x170 */ REPEAT8(ldrb_reg),
	/* 0x178 */ REPEAT8(ldrsh_reg),
	/* 0x180 */ REPEAT32(str_imm),
	/* 0x1A0 */ REPEAT32(ldr_imm),
	/* 0x1C0 */ REPEAT32(strb_imm),
	/* 0x1E0 */ REPEAT32(ldrb_imm),
	/* 0x200 */ REPEAT32(strh_imm),
	/* 0x220 */ REPEAT32(ldrh_imm),
	/* 0x240 */ REPEAT4(strsp_r0),
	/* 0x244 */ REPEAT4(strsp_r1),
	/* 0x248 */ REPEAT4(strsp_r2),
	/* 0x24C */ REPEAT4(strsp_r3),
	/* 0x250 */ REPEAT4(strsp_r4),
	/* 0x254 */ REPEAT4(strsp_r5),
	/* 0x258 */ REPEAT4(strsp_r6),
	/* 0x25C */ REPEAT4(strsp_r7),
	/* 0x260 */ REPEAT4(ldrsp_r0),
	/* 0x264 */ REPEAT4(ldrsp_r1),
	/* 0x268 */ REPEAT4(ldrsp_r2),
	/* 0x26C */ REPEAT4(ldrsp_r3),
	/* 0x270 */ REPEAT4(ldrsp_r4),
	/* 0x274 */ REPEAT4(ldrsp_r5),
	/* 0x278 */ REPEAT4(ldrsp_r6),
	/* 0x27C */ REPEAT4(ldrsp_r7),
	/* 0x280 */ REPEAT4(addpc_r0),
	/* 0x284 */ REPEAT4(addpc_r1),
	/* 0x288 */ REPEAT4(addpc_r2),
	/* 0x28C */ REPEAT4(addpc_r3),
	/* 0x290 */ REPEAT4(addpc_r4),
	/* 0x294 */ REPEAT4(addpc_r5),
	/* 0x298 */ REPEAT4(addpc_r6),
	/* 0x29C */ REPEAT4(addpc_r7),
	/* 0x2A0 */ REPEAT4(addsp_r0),
	/* 0x2A4 */ REPEAT4(addsp_r1),
	/* 0x2A8 */ REPEAT4(addsp_r2),
	/* 0x2AC */ REPEAT4(addsp_r3),
	/* 0x2B0 */ REPEAT4(addsp_r4),
	/* 0x2B4 */ REPEAT4(addsp_r5),
	/* 0x2B8 */ REPEAT4(addsp_r6),
	/* 0x2BC */ REPEAT4(addsp_r7),
	/* 0x2C0 */ REPEAT4(addsp_imm),
	/* 0x2C4 */ REPEAT4(undef),
	/* 0x2C8 */ REPEAT8(undef),
	/* 0x2D0 */ REPEAT4(push),
	/* 0x2D4 */ REPEAT4(push_lr),
	/* 0x2D8 */ REPEAT8(undef),
	/* 0x2E0 */ REPEAT16(undef),
	/* 0x2F0 */ REPEAT4(pop),
	/* 0x2F4 */ REPEAT4(pop_pc),
	/* 0x2F8 */ REPEAT4(bkpt),
	/* 0x2FC */ REPEAT4(undef),
	/* 0x300 */ REPEAT4(stm_r0),
	/* 0x304 */ REPEAT4(stm_r1),
	/* 0x308 */ REPEAT4(stm_r2),
	/* 0x30C */ REPEAT4(stm_r3),
	/* 0x310 */ REPEAT4(stm_r4),
	/* 0x314 */ REPEAT4(stm_r5),
	/* 0x318 */ REPEAT4(stm_r6),
	/* 0x31C */ REPEAT4(stm_r7),
	/* 0x320 */ REPEAT4(ldm_r0),
	/* 0x324 */ REPEAT4(ldm_r1),
	/* 0x328 */ REPEAT4(ldm_r2),
	/* 0x32C */ REPEAT4(ldm_r3),
	/* 0x330 */ REPEAT4(ldm_r4),
	/* 0x334 */ REPEAT4(ldm_r5),
	/* 0x338 */ REPEAT4(ldm_r6),
	/* 0x33C */ REPEAT4(ldm_r7),
	/* 0x340 */ REPEAT4(beq),
	/* 0x344 */ REPEAT4(bne),
	/* 0x348 */ REPEAT4(bcs),
	/* 0x34C */ REPEAT4(bcc),
	/* 0x350 */ REPEAT4(bmi),
	/* 0x354 */ REPEAT4(bpl),
	/* 0x358 */ REPEAT4(bvs),
	/* 0x35C */ REPEAT4(bvc),
	/* 0x360 */ REPEAT4(bhi),
	/* 0x364 */ REPEAT4(bls),
	/* 0x368 */ REPEAT4(bge),
	/* 0x36C */ REPEAT4(blt),
	/* 0x370 */ REPEAT4(bgt),
	/* 0x374 */ REPEAT4(ble),
	/* 0x378 */ REPEAT4(undef),
	/* 0x37C */ REPEAT4(swi),
	/* 0x380 */ REPEAT32(b),
	/* 0x3A0 */ REPEAT32(blx_off),
	/* 0x3C0 */ REPEAT32(bl_setup),
	/* 0x3E0 */ REPEAT32(bl_off),
};
