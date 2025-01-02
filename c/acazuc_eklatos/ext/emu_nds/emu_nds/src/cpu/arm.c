#include "instr.h"
#include "../cpu.h"
#include "../mem.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define ARM_INSTR(name, exec_fn, print_fn) \
static void exec_##name(struct cpu *cpu) \
exec_fn \
static void print_##name(struct cpu *cpu, char *data, size_t size) \
{ \
	(void)cpu; \
	print_fn \
} \
static const struct cpu_instr arm_##name = \
{ \
	.exec = exec_##name, \
	.print = print_##name, \
}

#define ARM_LSL(v, s) (((s) >= 32) ? 0 : ((v) << (s)))
#define ARM_LSR(v, s) (((s) >= 32) ? 0 : ((v) >> (s)))
#define ARM_ASR(v, s) (((s) >= 32) ? (v & 0x80000000UL) : (uint32_t)((int32_t)(v) >> (s)))
#define ARM_ROR(v, s) (((v) >> (s)) | ((v) << (32 - (s))))

static void exec_alu_flags_logical(struct cpu *cpu, uint32_t v)
{
	CPU_SET_FLAG_N(cpu, v & 0x80000000UL);
	CPU_SET_FLAG_Z(cpu, !v);
}

static void exec_alu_flags_add(struct cpu *cpu, uint32_t v, uint32_t op1, uint32_t op2)
{
	CPU_SET_FLAG_V(cpu, (~(op1 ^ op2) & (v ^ op2)) & 0x80000000UL);
	exec_alu_flags_logical(cpu, v);
}

static void exec_alu_flags_sub(struct cpu *cpu, uint32_t v, uint32_t op1, uint32_t op2)
{
	CPU_SET_FLAG_V(cpu, ((op1 ^ op2) & (v ^ op1)) & 0x80000000UL);
	exec_alu_flags_logical(cpu, v);
}

static void exec_alu_and(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	cpu_set_reg(cpu, rd, op1 & op2);
}

static void exec_alu_ands(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	uint32_t v = op1 & op2;
	cpu_set_reg(cpu, rd, v);
	if (rd != CPU_REG_PC)
		exec_alu_flags_logical(cpu, v);
}

static void exec_alu_eor(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	cpu_set_reg(cpu, rd, op1 ^ op2);
}

static void exec_alu_eors(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	uint32_t v = op1 ^ op2;
	cpu_set_reg(cpu, rd, v);
	if (rd != CPU_REG_PC)
		exec_alu_flags_logical(cpu, v);
}

static void exec_alu_sub(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	cpu_set_reg(cpu, rd, op1 - op2);
}

static void exec_alu_subs(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	uint32_t v = op1 - op2;
	cpu_set_reg(cpu, rd, v);
	if (rd != CPU_REG_PC)
	{
		exec_alu_flags_sub(cpu, v, op1, op2);
		CPU_SET_FLAG_C(cpu, op2 <= op1);
	}
}

static void exec_alu_rsb(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	cpu_set_reg(cpu, rd, op2 - op1);
}

static void exec_alu_rsbs(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	uint32_t v = op2 - op1;
	cpu_set_reg(cpu, rd, v);
	if (rd != CPU_REG_PC)
	{
		exec_alu_flags_sub(cpu, v, op2, op1);
		CPU_SET_FLAG_C(cpu, op1 <= op2);
	}
}

static void exec_alu_add(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	cpu_set_reg(cpu, rd, op1 + op2);
}

static void exec_alu_adds(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	uint32_t v = op1 + op2;
	cpu_set_reg(cpu, rd, v);
	if (rd != CPU_REG_PC)
	{
		exec_alu_flags_add(cpu, v, op1, op2);
		CPU_SET_FLAG_C(cpu, v < op1);
	}
}

static void exec_alu_adc(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	cpu_set_reg(cpu, rd, op1 + op2 + CPU_GET_FLAG_C(cpu));
}

static void exec_alu_adcs(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	uint32_t c = CPU_GET_FLAG_C(cpu);
	uint32_t v = op1 + op2 + c;
	cpu_set_reg(cpu, rd, v);
	if (rd != CPU_REG_PC)
	{
		exec_alu_flags_add(cpu, v, op1, op2 + c);
		CPU_SET_FLAG_C(cpu, c ? v <= op1 : v < op1);
	}
}

static void exec_alu_sbc(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	cpu_set_reg(cpu, rd, op1 - op2 + CPU_GET_FLAG_C(cpu) - 1);
}

static void exec_alu_sbcs(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	uint32_t c = CPU_GET_FLAG_C(cpu);
	uint32_t v2 = op2 + 1 - c;
	uint32_t v = op1 - v2;
	cpu_set_reg(cpu, rd, v);
	if (rd != CPU_REG_PC)
	{
		exec_alu_flags_sub(cpu, v, op1, v2);
		CPU_SET_FLAG_C(cpu, c ? op2 <= op1 : v < op1);
	}
}

static void exec_alu_rsc(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	cpu_set_reg(cpu, rd, op2 - op1 + CPU_GET_FLAG_C(cpu) - 1);
}

static void exec_alu_rscs(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	uint32_t c = CPU_GET_FLAG_C(cpu);
	uint32_t v1 = op1 + 1 - c;
	uint32_t v = op2 - v1;
	cpu_set_reg(cpu, rd, v);
	if (rd != CPU_REG_PC)
	{
		exec_alu_flags_sub(cpu, v, op2, v1);
		CPU_SET_FLAG_C(cpu, c ? op1 <= op2 : v < op2);
	}
}

static void exec_alu_tsts(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	uint32_t v = op1 & op2;
	if (rd != CPU_REG_PC)
		exec_alu_flags_logical(cpu, v);
}

static void exec_alu_teqs(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	uint32_t v = op1 ^ op2;
	if (rd != CPU_REG_PC)
		exec_alu_flags_logical(cpu, v);
}

static void exec_alu_cmps(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	uint32_t v = op1 - op2;
	if (rd != CPU_REG_PC)
	{
		exec_alu_flags_sub(cpu, v, op1, op2);
		CPU_SET_FLAG_C(cpu, op2 <= op1);
	}
}

static void exec_alu_cmns(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	uint32_t v = op1 + op2;
	if (rd != CPU_REG_PC)
	{
		exec_alu_flags_add(cpu, v, op1, op2);
		CPU_SET_FLAG_C(cpu, v < op1);
	}
}

static void exec_alu_orr(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	uint32_t v = op1 | op2;
	cpu_set_reg(cpu, rd, v);
}

static void exec_alu_orrs(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	uint32_t v = op1 | op2;
	cpu_set_reg(cpu, rd, v);
	if (rd != CPU_REG_PC)
		exec_alu_flags_logical(cpu, v);
}

static void exec_alu_mov(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	(void)op1;
	cpu_set_reg(cpu, rd, op2);
}

static void exec_alu_movs(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	(void)op1;
	uint32_t v = op2;
	cpu_set_reg(cpu, rd, v);
	if (rd != CPU_REG_PC)
		exec_alu_flags_logical(cpu, v);
}

static void exec_alu_bic(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	cpu_set_reg(cpu, rd, op1 & ~op2);
}

static void exec_alu_bics(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	uint32_t v = op1 & ~op2;
	cpu_set_reg(cpu, rd, v);
	if (rd != CPU_REG_PC)
		exec_alu_flags_logical(cpu, v);
}

static void exec_alu_mvn(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	(void)op1;
	cpu_set_reg(cpu, rd, ~op2);
}

static void exec_alu_mvns(struct cpu *cpu, uint32_t rd, uint32_t op1, uint32_t op2)
{
	(void)op1;
	uint32_t v = ~op2;
	cpu_set_reg(cpu, rd, v);
	if (rd != CPU_REG_PC)
		exec_alu_flags_logical(cpu, v);
}

#define ARM_ALU_DECODEIR(cpu, pcoff) \
	uint32_t rd = (cpu->instr_opcode >> 12) & 0xF; \
	uint32_t op1r = (cpu->instr_opcode >> 16) & 0xF; \
	uint32_t op1 = cpu_get_reg(cpu, op1r); \
	op1 += (op1r == CPU_REG_PC) ? pcoff : 0; \
	uint32_t op2r = (cpu->instr_opcode >>  0) & 0xF; \
	uint32_t op2s = cpu_get_reg(cpu, op2r); \
	op2s += (op2r == CPU_REG_PC) ? pcoff : 0

#define ARM_ALU_DECODEI(cpu) \
	uint8_t shift = (cpu->instr_opcode >> 7) & 0x1F; \
	ARM_ALU_DECODEIR(cpu, 8)

#define ARM_ALU_DECODER(cpu) \
	uint8_t shift = cpu_get_reg(cpu, (cpu->instr_opcode >> 8) & 0xF) & 0xFF; \
	ARM_ALU_DECODEIR(cpu, 12)

#define ARM_ALU_DECODEP(cpu) \
	uint8_t shift = (cpu->instr_opcode >> 7) & 0x1F; \
	uint32_t rd = (cpu->instr_opcode >> 12) & 0xF; \
	uint32_t op1 = (cpu->instr_opcode >> 16) & 0xF; \
	uint32_t op2 = (cpu->instr_opcode >>  0) & 0xF

#define ARM_ALU_OP_ROT(op, ctest, sflag, useout, rot, rot_name, rot_opi, rot_opr) \
ARM_INSTR(op##_##rot##i, \
{ \
	ARM_ALU_DECODEI(cpu); \
	rot_opi \
	exec_alu_##op(cpu, rd, op1, op2); \
	if (sflag && rd == CPU_REG_PC) \
	{ \
		cpu->regs.cpsr = *cpu->regs.spsr; \
		cpu_update_mode(cpu); \
	} \
	if (!useout || rd != CPU_REG_PC) \
		cpu_inc_pc(cpu, 4); \
}, \
{ \
	ARM_ALU_DECODEP(cpu); \
	if (useout) \
		snprintf(data, size, "%s r%" PRIu32 ", r%" PRIu32 ", r%" PRIu32 ", " rot_name " #0x%x", #op, rd, op1, op2, shift); \
	else \
		snprintf(data, size, "%s r%" PRIu32 ", r%" PRIu32 ", " rot_name " #0x%x", #op, op1, op2, shift); \
}); \
ARM_INSTR(op##_##rot##r, \
{ \
	ARM_ALU_DECODER(cpu); \
	rot_opr \
	exec_alu_##op(cpu, rd, op1, op2); \
	if (sflag && rd == CPU_REG_PC) \
	{ \
		cpu->regs.cpsr = *cpu->regs.spsr; \
		cpu_update_mode(cpu); \
	} \
	if (!useout || rd != CPU_REG_PC) \
		cpu_inc_pc(cpu, 4); \
}, \
{ \
	ARM_ALU_DECODEP(cpu); \
	if (useout) \
		snprintf(data, size, "%s r%" PRIu32 ", r%" PRIu32 ", r%" PRIu32 ", " rot_name " r%d", #op, rd, op1, op2, shift >> 1); \
	else \
		snprintf(data, size, "%s r%" PRIu32 ", r%" PRIu32 ", " rot_name " r%d", #op, op1, op2, shift >> 1); \
})

#define ARM_ALU_LLI(ctest) uint32_t op2 = ARM_LSL(op2s, shift); if (ctest && shift) { CPU_SET_FLAG_C(cpu, op2s & (1 << (32 - shift))); }
#define ARM_ALU_LLR(ctest) uint32_t op2 = ARM_LSL(op2s, shift); if (ctest && shift) { CPU_SET_FLAG_C(cpu, op2s & (1 << (32 - shift))); }
#define ARM_ALU_LRI(ctest) uint32_t op2 = ARM_LSR(op2s, shift ? shift : 32); if (ctest) { CPU_SET_FLAG_C(cpu, shift ? (op2s & (1 << (shift - 1))) : (op2s & 0x80000000UL)); }
#define ARM_ALU_LRR(ctest) uint32_t op2 = ARM_LSR(op2s, shift); if (ctest && shift) { CPU_SET_FLAG_C(cpu, op2s & (1 << (shift - 1))); }
#define ARM_ALU_ARI(ctest) \
	uint32_t op2; \
	if (!shift) \
	{ \
		op2 = op2s & 0x80000000UL; \
		if (op2) \
		{ \
			op2 = 0xFFFFFFFFUL; \
			if (ctest) \
				CPU_SET_FLAG_C(cpu, 1); \
		} \
		else \
		{ \
			if (ctest) \
				CPU_SET_FLAG_C(cpu, 0); \
		} \
	} \
	else \
	{ \
		op2 = ARM_ASR(op2s, shift ? shift : 32); \
		if (ctest) \
			CPU_SET_FLAG_C(cpu, op2s & (1 << (shift - 1))); \
	}
#define ARM_ALU_ARR(ctest) uint32_t op2 = ARM_ASR(op2s, shift); if (ctest && shift) { CPU_SET_FLAG_C(cpu, op2s & (1 << (shift - 1))); }
#define ARM_ALU_RRI(ctest) \
	uint32_t op2; \
	if (!shift) \
	{ \
		op2 = ARM_ROR(op2s, 1) & 0x7FFFFFFF; \
		op2 |= CPU_GET_FLAG_C(cpu) << 31; \
		if (ctest) \
			CPU_SET_FLAG_C(cpu, op2s & 0x1); \
	} \
	else \
	{ \
		op2 = ARM_ROR(op2s, shift); \
		if (ctest) \
			CPU_SET_FLAG_C(cpu, op2s & (1 << (shift - 1))); \
	}
#define ARM_ALU_RRR(ctest) uint32_t op2 = ARM_ROR(op2s, shift); if (ctest && shift) { CPU_SET_FLAG_C(cpu, op2s & (1 << (shift - 1))); }

#define ARM_ALU_OP(op, sflag, ctest, useout) \
ARM_ALU_OP_ROT(op, ctest, sflag, useout, ll, "LSL", ARM_ALU_LLI(ctest), ARM_ALU_LLR(ctest)); \
ARM_ALU_OP_ROT(op, ctest, sflag, useout, lr, "LSR", ARM_ALU_LRI(ctest), ARM_ALU_LRR(ctest)); \
ARM_ALU_OP_ROT(op, ctest, sflag, useout, ar, "ASR", ARM_ALU_ARI(ctest), ARM_ALU_ARR(ctest)); \
ARM_ALU_OP_ROT(op, ctest, sflag, useout, rr, "ROR", ARM_ALU_RRI(ctest), ARM_ALU_RRR(ctest)); \
ARM_INSTR(op##_imm, \
{ \
	uint32_t shift = (cpu->instr_opcode >> 8) & 0xF; \
	uint32_t rd = (cpu->instr_opcode >> 12) & 0xF; \
	uint32_t op1r = (cpu->instr_opcode >> 16) & 0xF; \
	uint32_t op1 = cpu_get_reg(cpu, op1r); \
	op1 += (op1r == CPU_REG_PC) ? 8 : 0; \
	uint32_t op2s = (cpu->instr_opcode >> 0) & 0xFF; \
	uint32_t op2 = ARM_ROR(op2s, shift * 2); \
	if (ctest && shift) \
		CPU_SET_FLAG_C(cpu, op2s & (1 << ((shift * 2) - 1))); \
	if (sflag && rd == CPU_REG_PC) \
	{ \
		cpu->regs.cpsr = *cpu->regs.spsr; \
		cpu_update_mode(cpu); \
	} \
	exec_alu_##op(cpu, rd, op1, op2); \
	if (!useout || rd != CPU_REG_PC) \
		cpu_inc_pc(cpu, 4); \
}, \
{ \
	uint32_t shift = (cpu->instr_opcode >> 8) & 0xF; \
	uint32_t rd = (cpu->instr_opcode >> 12) & 0xF; \
	uint32_t op1 = (cpu->instr_opcode >> 16) & 0xF; \
	uint32_t op2 = (cpu->instr_opcode >> 0) & 0xFF; \
	if (useout) \
		snprintf(data, size, "%s r%" PRIu32 ", r%" PRIu32 ", #0x%" PRIx32 ", ROR #0x%" PRIx32, #op, rd, op1, op2, shift * 2); \
	else \
		snprintf(data, size, "%s r%" PRIu32 ", #0x%" PRIx32 ", ROR #0x%" PRIx32, #op, op1, op2, shift * 2); \
})

#define ARM_ALU_OPS(op, ctest, useout) \
	ARM_ALU_OP(op   , 0, 0    , useout); \
	ARM_ALU_OP(op##s, 1, ctest, useout)

ARM_ALU_OPS(and, 1, 1);
ARM_ALU_OPS(eor, 1, 1);
ARM_ALU_OPS(sub, 0, 1);
ARM_ALU_OPS(rsb, 0, 1);
ARM_ALU_OPS(add, 0, 1);
ARM_ALU_OPS(adc, 0, 1);
ARM_ALU_OPS(sbc, 0, 1);
ARM_ALU_OPS(rsc, 0, 1);
ARM_ALU_OP(tsts, 1, 1, 0);
ARM_ALU_OP(teqs, 1, 1, 0);
ARM_ALU_OP(cmps, 1, 0, 0);
ARM_ALU_OP(cmns, 1, 0, 0);
ARM_ALU_OPS(orr, 1, 1);
ARM_ALU_OPS(mov, 1, 1);
ARM_ALU_OPS(bic, 1, 1);
ARM_ALU_OPS(mvn, 1, 1);

#define MULOP(n, wmod, rs_lower_upper, rm_lower_upper, halfword, signed_unsigned, longword, accum, setcond) \
ARM_INSTR(n, \
{ \
	uint32_t rmr = (cpu->instr_opcode >>  0) & 0xF; \
	uint32_t rsr = (cpu->instr_opcode >>  8) & 0xF; \
	uint32_t rnr = (cpu->instr_opcode >> 12) & 0xF; \
	uint32_t rdr = (cpu->instr_opcode >> 16) & 0xF; \
	uint32_t rm = cpu_get_reg(cpu, rmr); \
	uint32_t rs = cpu_get_reg(cpu, rsr); \
	if (halfword) \
	{ \
		int16_t hrm; \
		int16_t hrs; \
		if (rm_lower_upper) \
			hrm = (int16_t)(uint16_t)(rm >> 16); \
		else \
			hrm = (int16_t)(uint16_t)(rm & 0xFFFF); \
		if (rs_lower_upper) \
			hrs = (int16_t)(uint16_t)(rs >> 16); \
		else \
			hrs = (int16_t)(uint16_t)(rs & 0xFFFF); \
		if (longword) \
		{ \
			uint64_t res = (int64_t)hrm * (int64_t)hrs; \
			if (accum) \
			{ \
				uint64_t rn = cpu_get_reg(cpu, rnr); \
				uint64_t rd = cpu_get_reg(cpu, rdr); \
				res += (rd << 32) | rn; \
				cpu->instr_delay++; \
			} \
			cpu_set_reg(cpu, rdr, res >> 32); \
			cpu_set_reg(cpu, rnr, res >>  0); \
			if (setcond) \
			{ \
				CPU_SET_FLAG_Z(cpu, !res); \
				CPU_SET_FLAG_N(cpu, res & 0x8000000000000000ULL); \
				if (!cpu->arm9) \
				{ \
					CPU_SET_FLAG_C(cpu, 0); \
					CPU_SET_FLAG_V(cpu, 0); \
				} \
			} \
		} \
		else if (wmod) \
		{ \
			uint32_t res = (int32_t)(((int64_t)rm * (int64_t)hrs) / 0x10000); \
			if (accum) \
			{ \
				uint32_t rn = cpu_get_reg(cpu, rnr); \
				res += rn; \
				cpu->instr_delay++; \
			} \
			cpu_set_reg(cpu, rdr, res); \
			if (setcond) \
			{ \
				CPU_SET_FLAG_Z(cpu, !res); \
				CPU_SET_FLAG_N(cpu, res & 0x80000000UL); \
				if (!cpu->arm9) \
				{ \
					CPU_SET_FLAG_C(cpu, 0); \
					CPU_SET_FLAG_V(cpu, 0); \
				} \
			} \
		} \
		else \
		{ \
			uint32_t res = (int32_t)hrm * (int32_t)hrs; \
			if (accum) \
			{ \
				uint32_t rn = cpu_get_reg(cpu, rnr); \
				res += rn; \
				cpu->instr_delay++; \
			} \
			cpu_set_reg(cpu, rdr, res); \
			if (setcond) \
			{ \
				CPU_SET_FLAG_Z(cpu, !res); \
				CPU_SET_FLAG_N(cpu, res & 0x80000000UL); \
				if (!cpu->arm9) \
				{ \
					CPU_SET_FLAG_C(cpu, 0); \
					CPU_SET_FLAG_V(cpu, 0); \
				} \
			} \
		} \
	} \
	else if (longword) \
	{ \
		uint64_t res; \
		if (signed_unsigned) \
			res = (uint64_t)rm * (uint64_t)rs; \
		else \
			res = (int64_t)(int32_t)rm * (int64_t)(int32_t)rs; \
		cpu->instr_delay++; \
		if (accum) \
		{ \
			uint64_t rn = cpu_get_reg(cpu, rnr); \
			uint64_t rd = cpu_get_reg(cpu, rdr); \
			res += (rd << 32) | rn; \
			cpu->instr_delay++; \
		} \
		cpu_set_reg(cpu, rdr, res >> 32); \
		cpu_set_reg(cpu, rnr, res >>  0); \
		if (setcond) \
		{ \
			CPU_SET_FLAG_Z(cpu, !res); \
			CPU_SET_FLAG_N(cpu, res & 0x8000000000000000ULL); \
			if (!cpu->arm9) \
			{ \
				CPU_SET_FLAG_C(cpu, 0); \
				CPU_SET_FLAG_V(cpu, 0); \
			} \
		} \
	} \
	else \
	{ \
		uint32_t res; \
		res = rm * rs; \
		cpu->instr_delay++; \
		if (accum) \
		{ \
			uint32_t rn = cpu_get_reg(cpu, rnr); \
			res += rn; \
			cpu->instr_delay++; \
		} \
		cpu_set_reg(cpu, rdr, res); \
		if (setcond) \
		{ \
			CPU_SET_FLAG_Z(cpu, !res); \
			CPU_SET_FLAG_N(cpu, res & 0x80000000UL); \
			if (!cpu->arm9) \
			{ \
				CPU_SET_FLAG_C(cpu, 0); \
				CPU_SET_FLAG_V(cpu, 0); \
			} \
		} \
	} \
	cpu_inc_pc(cpu, 4); \
	cpu->instr_delay++; \
}, \
{ \
	uint32_t rm = (cpu->instr_opcode >>  0) & 0xF; \
	uint32_t rs = (cpu->instr_opcode >>  8) & 0xF; \
	uint32_t rn = (cpu->instr_opcode >> 12) & 0xF; \
	uint32_t rd = (cpu->instr_opcode >> 16) & 0xF; \
	if (longword) \
	{ \
		snprintf(data, size, #n " r%" PRIu32 ", r%" PRIu32 ", r%" PRIu32 ", %" PRIu32, rd, rn, rm, rs); \
	} \
	else \
	{ \
		if (accum) \
			snprintf(data, size, #n " r%" PRIu32 ", r%" PRIu32 ", r%" PRIu32 ", %" PRIu32, rd, rm, rs, rn); \
		else \
			snprintf(data, size, #n " r%" PRIu32 ", r%" PRIu32 ", r%" PRIu32, rd, rm, rs); \
	} \
})

MULOP(mul    , 0, 0, 0, 0, 1, 0, 0, 0);
MULOP(muls   , 0, 0, 0, 0, 1, 0, 0, 1);
MULOP(mla    , 0, 0, 0, 0, 1, 0, 1, 0);
MULOP(mlas   , 0, 0, 0, 0, 1, 0, 1, 1);
MULOP(umull  , 0, 0, 0, 0, 1, 1, 0, 0);
MULOP(umulls , 0, 0, 0, 0, 1, 1, 0, 1);
MULOP(umlal  , 0, 0, 0, 0, 1, 1, 1, 0);
MULOP(umlals , 0, 0, 0, 0, 1, 1, 1, 1);
MULOP(smull  , 0, 0, 0, 0, 0, 1, 0, 0);
MULOP(smulls , 0, 0, 0, 0, 0, 1, 0, 1);
MULOP(smlal  , 0, 0, 0, 0, 0, 1, 1, 0);
MULOP(smlals , 0, 0, 0, 0, 0, 1, 1, 1);
MULOP(smlabb , 0, 0, 0, 1, 0, 0, 1, 0);
MULOP(smlatb , 0, 1, 0, 1, 0, 0, 1, 0);
MULOP(smlabt , 0, 0, 1, 1, 0, 0, 1, 0);
MULOP(smlatt , 0, 1, 1, 1, 0, 0, 1, 0);
MULOP(smlawb , 1, 0, 0, 1, 0, 0, 1, 0);
MULOP(smlawt , 1, 0, 1, 1, 0, 0, 1, 0);
MULOP(smulbb , 0, 0, 0, 1, 0, 0, 0, 0);
MULOP(smultb , 0, 1, 0, 1, 0, 0, 0, 0);
MULOP(smulbt , 0, 0, 1, 1, 0, 0, 0, 0);
MULOP(smultt , 0, 1, 1, 1, 0, 0, 0, 0);
MULOP(smulwb , 1, 0, 0, 1, 0, 0, 0, 0);
MULOP(smulwt , 1, 0, 1, 1, 0, 0, 0, 0);
MULOP(smlalbb, 0, 0, 0, 1, 0, 1, 1, 0);
MULOP(smlaltb, 0, 1, 0, 1, 0, 1, 1, 0);
MULOP(smlalbt, 0, 0, 1, 1, 0, 1, 1, 0);
MULOP(smlaltt, 0, 1, 1, 1, 0, 1, 1, 0);

#define STLD_ROT_LL offset = ARM_LSL(rm, shift)
#define STLD_ROT_LR offset = ARM_LSR(rm, shift ? shift : 32)
#define STLD_ROT_AR offset = ARM_ASR(rm, shift ? shift : 32)
#define STLD_ROT_RR \
	if (!shift) \
	{ \
		offset = ARM_ROR(rm, 1) & 0x7FFFFFFF; \
		offset |= CPU_GET_FLAG_C(cpu) << 31; \
	} \
	else \
	{ \
		offset = ARM_ROR(rm, shift); \
	}
#define STLD_ROT_NO offset = rm

#define STLD_BTBT(opname, oparg, rotname, rotstr, imm_reg, post_pre, dec_inc, word_byte, writeback_mm, st_ld, regshift) \
ARM_INSTR(opname##_##oparg####rotname, \
{ \
	uint32_t rnr = (cpu->instr_opcode >> 16) & 0xF; \
	uint32_t rn = cpu_get_reg(cpu, rnr); \
	rn += (rnr == CPU_REG_PC) ? 8 : 0; \
	uint32_t rdr = (cpu->instr_opcode >> 12) & 0xF; \
	uint32_t offset; \
	if (imm_reg) \
	{ \
		uint8_t shift = (cpu->instr_opcode >> 7) & 0x1F; \
		(void)shift; \
		uint32_t rm = cpu_get_reg(cpu, cpu->instr_opcode & 0xF); \
		regshift; \
	} \
	else \
	{ \
		offset = cpu->instr_opcode & 0xFFF; \
	} \
	if (post_pre) \
	{ \
		if (dec_inc) \
			rn += offset; \
		else \
			rn -= offset; \
	} \
	if (st_ld) \
	{ \
		uint32_t addr = rn; \
		if (!word_byte && !writeback_mm) \
			addr &= ~3; \
		uint32_t v; \
		if (word_byte) \
			v = cpu->get8(cpu->mem, addr, MEM_DATA_NSEQ); \
		else \
			v = cpu->get32(cpu->mem, addr, MEM_DATA_NSEQ); \
		if (!word_byte && !writeback_mm) \
			v = ARM_ROR(v, (rn & 3) * 8); \
		if (rdr == CPU_REG_PC) \
		{ \
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
				v &= ~3; \
			} \
		} \
		cpu_set_reg(cpu, rdr, v); \
	} \
	else \
	{ \
		uint32_t rd = cpu_get_reg(cpu, rdr); \
		rd += (rdr == CPU_REG_PC) ? 12 : 0; \
		if (word_byte) \
			cpu->set8(cpu->mem, rn, rd, MEM_DATA_NSEQ); \
		else \
			cpu->set32(cpu->mem, rn, rd, MEM_DATA_NSEQ); \
	} \
	if (post_pre && writeback_mm && (!st_ld || rnr != rdr)) \
		cpu_set_reg(cpu, rnr, rn); \
	if (!post_pre && (!st_ld || rdr != rnr)) \
	{ \
		if (dec_inc) \
			rn += offset; \
		else \
			rn -= offset; \
		cpu_set_reg(cpu, rnr, rn); \
	} \
	if (!st_ld || rdr != CPU_REG_PC) \
		cpu_inc_pc(cpu, 4); \
}, \
{ \
	uint32_t rd = (cpu->instr_opcode >> 12) & 0xF; \
	uint32_t rn = (cpu->instr_opcode >> 16) & 0xF; \
	if (imm_reg) \
	{ \
		uint8_t shift = (cpu->instr_opcode >> 7) & 0x1F; \
		uint8_t rm = cpu->instr_opcode & 0xF; \
		if (post_pre) \
			snprintf(data, size, #opname " r%" PRIu32 ", [r%" PRIu32 ", %sr%d, " rotstr " #0x%x]%s", rd, rn, dec_inc ? "" : "-", rm, shift, writeback_mm ? "!" : ""); \
		else \
			snprintf(data, size, #opname " r%" PRIu32 ", [r%" PRIu32 "], %sr%d, " rotstr " #0x%x", rd, rn, dec_inc ? "" : "-", rm, shift); \
	} \
	else \
	{ \
		uint32_t offset = cpu->instr_opcode & 0xFFF; \
		if (post_pre) \
			snprintf(data, size, #opname " r%" PRIu32 ", [r%" PRIu32 ", #%s0x%" PRIx32 "]%s", rd, rn, dec_inc ? "" : "-", offset, writeback_mm ? "!" : ""); \
		else \
			snprintf(data, size, #opname " r%" PRIu32 ", [r%" PRIu32 "], #%s0x%" PRIx32, rd, rn, dec_inc ? "" : "-", offset); \
	} \
})

#define STLD_HDST(opname, oparg, post_pre, dec_inc, reg_imm, writeback, st_ld, op) \
ARM_INSTR(opname##_##oparg, \
{ \
	uint32_t rnr = (cpu->instr_opcode >> 16) & 0xF; \
	uint32_t rn = cpu_get_reg(cpu, rnr); \
	rn += (rnr == CPU_REG_PC) ? 8 : 0; \
	uint32_t rdr = (cpu->instr_opcode >> 12) & 0xF; \
	uint32_t offset; \
	int post_update = 1; \
	if (reg_imm) \
		offset = ((cpu->instr_opcode & 0xF00) >> 4) | (cpu->instr_opcode & 0xF); \
	else \
		offset = cpu_get_reg(cpu, cpu->instr_opcode & 0xF); \
	if (post_pre) \
	{ \
		if (dec_inc) \
			rn += offset; \
		else \
			rn -= offset; \
	} \
	if (st_ld) \
	{ \
		if (op == 1) \
		{ \
			uint32_t v = cpu->get16(cpu->mem, rn & ~1, MEM_DATA_NSEQ); \
			if ((rn & 1) && !cpu->arm9) \
				v = ARM_ROR(v, 8); \
			cpu_set_reg(cpu, rdr, v); \
		} \
		else if (op == 2) \
		{ \
			cpu_set_reg(cpu, rdr, (int8_t)cpu->get8(cpu->mem, rn, MEM_DATA_NSEQ)); \
		} \
		else if (op == 3) \
		{ \
			if ((rn & 1) && !cpu->arm9) \
				cpu_set_reg(cpu, rdr, (int8_t)cpu->get8(cpu->mem, rn, MEM_DATA_NSEQ)); \
			else \
				cpu_set_reg(cpu, rdr, (int16_t)cpu->get16(cpu->mem, rn, MEM_DATA_NSEQ)); \
		} \
		post_update = (rdr != rnr); \
	} \
	else \
	{ \
		uint32_t rd = cpu_get_reg(cpu, rdr); \
		rd += (rdr == CPU_REG_PC) ? 12 : 0; \
		if (op == 1) \
		{ \
			cpu->set16(cpu->mem, rn, rd, MEM_DATA_NSEQ); \
		} \
		else if (op == 2) \
		{ \
			uint32_t rdr2 = (rdr + 1) & 0xF; \
			cpu_set_reg(cpu, rdr , cpu->get32(cpu->mem, rn + 0, MEM_DATA_NSEQ)); \
			cpu_set_reg(cpu, rdr2, cpu->get32(cpu->mem, rn + 4, MEM_DATA_NSEQ)); \
			post_update = (rdr != rnr && rdr2 != rnr); \
		} \
		else if (op == 3) \
		{ \
			uint32_t rdr2 = (rdr + 1) & 0xF; \
			uint32_t rd2 = cpu_get_reg(cpu, rdr2); \
			rd2 += (rdr2 == CPU_REG_PC) ? 12 : 0; \
			cpu->set32(cpu->mem, rn + 0, rd, MEM_DATA_NSEQ); \
			cpu->set32(cpu->mem, rn + 4, rd2, MEM_DATA_NSEQ); \
		} \
	} \
	if (post_pre && writeback && post_update) \
		cpu_set_reg(cpu, rnr, rn); \
	if (!post_pre && post_update) \
	{ \
		if (dec_inc) \
			rn += offset; \
		else \
			rn -= offset; \
		cpu_set_reg(cpu, rnr, rn); \
	} \
	if (rdr != CPU_REG_PC) \
		cpu_inc_pc(cpu, 4); \
}, \
{ \
	uint32_t rd = (cpu->instr_opcode >> 12) & 0xF; \
	uint32_t rn = (cpu->instr_opcode >> 16) & 0xF; \
	if (reg_imm) \
	{ \
		uint32_t offset = ((cpu->instr_opcode & 0xF00) >> 4) | (cpu->instr_opcode & 0xF); \
		if (post_pre) \
			snprintf(data, size, #opname " r%" PRIu32 ", [r%" PRIu32 ", #%s0x%" PRIx32 "]%s", rd, rn, dec_inc ? "" : "-", offset, writeback ? "!" : ""); \
		else \
			snprintf(data, size, #opname " r%" PRIu32 ", [r%" PRIu32 "], #%s0x%" PRIx32, rd, rn, dec_inc ? "" : "-", offset); \
	} \
	else \
	{ \
		uint32_t offset = cpu->instr_opcode & 0xF; \
		if (post_pre) \
			snprintf(data, size, #opname " r%" PRIu32 ", [r%" PRIu32 ", %sr%" PRIu32 "]%s", rd, rn, dec_inc ? "" : "-", offset, writeback ? "!" : ""); \
		else \
			snprintf(data, size, #opname " r%" PRIu32 ", [r%" PRIu32 "], %sr%" PRIu32, rd, rn, dec_inc ? "" : "-", offset); \
	} \
})

#define STLD_PTR(v, dec_inc) \
STLD_BTBT(str  , v, ll, "LSL", 1, 0, dec_inc, 0, 0, 0, STLD_ROT_LL); \
STLD_BTBT(str  , v, lr, "LSR", 1, 0, dec_inc, 0, 0, 0, STLD_ROT_LR); \
STLD_BTBT(str  , v, ar, "ASR", 1, 0, dec_inc, 0, 0, 0, STLD_ROT_AR); \
STLD_BTBT(str  , v, rr, "ROR", 1, 0, dec_inc, 0, 0, 0, STLD_ROT_RR); \
STLD_HDST(strh, v, 0, dec_inc, 0, 0, 0, 1); \
STLD_HDST(strd, v, 0, dec_inc, 0, 0, 0, 3); \
STLD_BTBT(strb , v, ll, "LSL", 1, 0, dec_inc, 1, 0, 0, STLD_ROT_LL); \
STLD_BTBT(strb , v, lr, "LSR", 1, 0, dec_inc, 1, 0, 0, STLD_ROT_LR); \
STLD_BTBT(strb , v, ar, "ASR", 1, 0, dec_inc, 1, 0, 0, STLD_ROT_AR); \
STLD_BTBT(strb , v, rr, "ROR", 1, 0, dec_inc, 1, 0, 0, STLD_ROT_RR); \
STLD_BTBT(strt , v, ll, "LSL", 1, 0, dec_inc, 0, 1, 0, STLD_ROT_LL); \
STLD_BTBT(strt , v, lr, "LSR", 1, 0, dec_inc, 0, 1, 0, STLD_ROT_LR); \
STLD_BTBT(strt , v, ar, "ASR", 1, 0, dec_inc, 0, 1, 0, STLD_ROT_AR); \
STLD_BTBT(strt , v, rr, "ROR", 1, 0, dec_inc, 0, 1, 0, STLD_ROT_RR); \
STLD_BTBT(strbt, v, ll, "LSL", 1, 0, dec_inc, 1, 1, 0, STLD_ROT_LL); \
STLD_BTBT(strbt, v, lr, "LSR", 1, 0, dec_inc, 1, 1, 0, STLD_ROT_LR); \
STLD_BTBT(strbt, v, ar, "ASR", 1, 0, dec_inc, 1, 1, 0, STLD_ROT_AR); \
STLD_BTBT(strbt, v, rr, "ROR", 1, 0, dec_inc, 1, 1, 0, STLD_ROT_RR); \
STLD_BTBT(ldr  , v, ll, "LSL", 1, 0, dec_inc, 0, 0, 1, STLD_ROT_LL); \
STLD_BTBT(ldr  , v, lr, "LSR", 1, 0, dec_inc, 0, 0, 1, STLD_ROT_LR); \
STLD_BTBT(ldr  , v, ar, "ASR", 1, 0, dec_inc, 0, 0, 1, STLD_ROT_AR); \
STLD_BTBT(ldr  , v, rr, "ROR", 1, 0, dec_inc, 0, 0, 1, STLD_ROT_RR); \
STLD_HDST(ldrh, v, 0, dec_inc, 0, 0, 1, 1); \
STLD_HDST(ldrd, v, 0, dec_inc, 0, 0, 0, 2); \
STLD_BTBT(ldrb , v, ll, "LSL", 1, 0, dec_inc, 1, 0, 1, STLD_ROT_LL); \
STLD_BTBT(ldrb , v, lr, "LSR", 1, 0, dec_inc, 1, 0, 1, STLD_ROT_LR); \
STLD_BTBT(ldrb , v, ar, "ASR", 1, 0, dec_inc, 1, 0, 1, STLD_ROT_AR); \
STLD_BTBT(ldrb , v, rr, "ROR", 1, 0, dec_inc, 1, 0, 1, STLD_ROT_RR); \
STLD_BTBT(ldrt , v, ll, "LSL", 1, 0, dec_inc, 0, 1, 1, STLD_ROT_LL); \
STLD_BTBT(ldrt , v, lr, "LSR", 1, 0, dec_inc, 0, 1, 1, STLD_ROT_LR); \
STLD_BTBT(ldrt , v, ar, "ASR", 1, 0, dec_inc, 0, 1, 1, STLD_ROT_AR); \
STLD_BTBT(ldrt , v, rr, "ROR", 1, 0, dec_inc, 0, 1, 1, STLD_ROT_RR); \
STLD_BTBT(ldrbt, v, ll, "LSL", 1, 0, dec_inc, 1, 1, 1, STLD_ROT_LL); \
STLD_BTBT(ldrbt, v, lr, "LSR", 1, 0, dec_inc, 1, 1, 1, STLD_ROT_LR); \
STLD_BTBT(ldrbt, v, ar, "ASR", 1, 0, dec_inc, 1, 1, 1, STLD_ROT_AR); \
STLD_BTBT(ldrbt, v, rr, "ROR", 1, 0, dec_inc, 1, 1, 1, STLD_ROT_RR); \
STLD_HDST(ldrsb, v, 0, dec_inc, 0, 0, 1, 2); \
STLD_HDST(ldrsh, v, 0, dec_inc, 0, 0, 1, 3)

STLD_PTR(ptrm, 0);
STLD_PTR(ptrp, 1);

#define STLD_PTI(v, dec_inc) \
STLD_BTBT(str  , v, , , 0, 0, dec_inc, 0, 0, 0, STLD_ROT_NO); \
STLD_HDST(strh , v, 0, dec_inc, 1, 0, 0, 1); \
STLD_HDST(strd , v, 0, dec_inc, 1, 0, 0, 3); \
STLD_BTBT(strb , v, , , 0, 0, dec_inc, 1, 0, 0, STLD_ROT_NO); \
STLD_BTBT(strt , v, , , 0, 0, dec_inc, 0, 1, 0, STLD_ROT_NO); \
STLD_BTBT(strbt, v, , , 0, 0, dec_inc, 1, 1, 0, STLD_ROT_NO); \
STLD_BTBT(ldr  , v, , , 0, 0, dec_inc, 0, 0, 1, STLD_ROT_NO); \
STLD_HDST(ldrh , v, 0, dec_inc, 1, 0, 1, 1); \
STLD_HDST(ldrd , v, 0, dec_inc, 1, 0, 0, 2); \
STLD_BTBT(ldrb , v, , , 0, 0, dec_inc, 1, 0, 1, STLD_ROT_NO); \
STLD_BTBT(ldrt , v, , , 0, 0, dec_inc, 0, 1, 1, STLD_ROT_NO); \
STLD_BTBT(ldrbt, v, , , 0, 0, dec_inc, 1, 1, 1, STLD_ROT_NO); \
STLD_HDST(ldrsb, v, 0, dec_inc, 1, 0, 1, 2); \
STLD_HDST(ldrsh, v, 0, dec_inc, 1, 0, 1, 3)

STLD_PTI(ptim, 0);
STLD_PTI(ptip, 1);

#define STLD_R(v, dec_inc, writeback) \
STLD_BTBT(str  , v, ll, "LSL", 1, 1, dec_inc, 0, writeback, 0, STLD_ROT_LL); \
STLD_BTBT(str  , v, lr, "LSR", 1, 1, dec_inc, 0, writeback, 0, STLD_ROT_LR); \
STLD_BTBT(str  , v, ar, "ASR", 1, 1, dec_inc, 0, writeback, 0, STLD_ROT_AR); \
STLD_BTBT(str  , v, rr, "ROR", 1, 1, dec_inc, 0, writeback, 0, STLD_ROT_RR); \
STLD_HDST(strh , v, 1, dec_inc, 0, writeback, 0, 1); \
STLD_HDST(strd , v, 1, dec_inc, 0, writeback, 0, 3); \
STLD_BTBT(strb , v, ll, "LSL", 1, 1, dec_inc, 1, writeback, 0, STLD_ROT_LL); \
STLD_BTBT(strb , v, lr, "LSR", 1, 1, dec_inc, 1, writeback, 0, STLD_ROT_LR); \
STLD_BTBT(strb , v, ar, "ASR", 1, 1, dec_inc, 1, writeback, 0, STLD_ROT_AR); \
STLD_BTBT(strb , v, rr, "ROR", 1, 1, dec_inc, 1, writeback, 0, STLD_ROT_RR); \
STLD_BTBT(ldr  , v, ll, "LSL", 1, 1, dec_inc, 0, writeback, 1, STLD_ROT_LL); \
STLD_BTBT(ldr  , v, lr, "LSR", 1, 1, dec_inc, 0, writeback, 1, STLD_ROT_LR); \
STLD_BTBT(ldr  , v, ar, "ASR", 1, 1, dec_inc, 0, writeback, 1, STLD_ROT_AR); \
STLD_BTBT(ldr  , v, rr, "ROR", 1, 1, dec_inc, 0, writeback, 1, STLD_ROT_RR); \
STLD_HDST(ldrh , v, 1, dec_inc, 0, writeback, 1, 1); \
STLD_HDST(ldrd , v, 1, dec_inc, 0, writeback, 0, 2); \
STLD_BTBT(ldrb , v, ll, "LSL", 1, 1, dec_inc, 1, writeback, 1, STLD_ROT_LL); \
STLD_BTBT(ldrb , v, lr, "LSR", 1, 1, dec_inc, 1, writeback, 1, STLD_ROT_LR); \
STLD_BTBT(ldrb , v, ar, "ASR", 1, 1, dec_inc, 1, writeback, 1, STLD_ROT_AR); \
STLD_BTBT(ldrb , v, rr, "ROR", 1, 1, dec_inc, 1, writeback, 1, STLD_ROT_RR); \
STLD_HDST(ldrsb, v, 1, dec_inc, 0, writeback, 1, 2); \
STLD_HDST(ldrsh, v, 1, dec_inc, 0, writeback, 1, 3)

STLD_R(ofrm, 0, 0);
STLD_R(ofrp, 1, 0);
STLD_R(prrm, 0, 1);
STLD_R(prrp, 1, 1);

#define STLD_I(v, dec_inc, writeback) \
STLD_BTBT(str  , v, , , 0, 1, dec_inc, 0, writeback, 0, STLD_ROT_NO); \
STLD_HDST(strh , v, 1, dec_inc, 1, writeback, 0, 1); \
STLD_HDST(strd , v, 1, dec_inc, 1, writeback, 0, 3); \
STLD_BTBT(strb , v, , , 0, 1, dec_inc, 1, writeback, 0, STLD_ROT_NO); \
STLD_BTBT(ldr  , v, , , 0, 1, dec_inc, 0, writeback, 1, STLD_ROT_NO); \
STLD_HDST(ldrh , v, 1, dec_inc, 1, writeback, 1, 1); \
STLD_HDST(ldrd , v, 1, dec_inc, 1, writeback, 0, 2); \
STLD_BTBT(ldrb , v, , , 0, 1, dec_inc, 1, writeback, 1, STLD_ROT_NO); \
STLD_HDST(ldrsb, v, 1, dec_inc, 1, writeback, 1, 2); \
STLD_HDST(ldrsh, v, 1, dec_inc, 1, writeback, 1, 3)

STLD_I(ofim, 0, 0);
STLD_I(ofip, 1, 0);
STLD_I(prim, 0, 1);
STLD_I(prip, 1, 1);

#define STLDM(opname, oparg, post_pre, down_up, usermode, writeback, st_ld) \
ARM_INSTR(opname####oparg, \
{ \
	uint32_t rnr = (cpu->instr_opcode >> 16) & 0xF; \
	uint32_t rl = (cpu->instr_opcode >>  0) & 0xFFFF; \
	uint32_t rn = cpu_get_reg(cpu, rnr); \
	bool pc_inc = true; \
	uint32_t old_base = rn; \
	uint32_t new_base; \
	uint32_t nregs; \
	if (rl) \
	{ \
		nregs = 0; \
		for (int i = 0; i < 16; ++i) \
		{ \
			if (rl & (1 << i)) \
				nregs++; \
		} \
	} \
	else \
	{ \
		nregs = 0x10; \
	} \
	if (!down_up) \
	{ \
		rn -= 4 * nregs; \
		if (writeback) \
			cpu_set_reg(cpu, rnr, rn); \
		new_base = rn; \
	} \
	else \
	{ \
		new_base = old_base + nregs * 4; \
	} \
	bool nowriteback = false; \
	if (rl) \
	{ \
		bool first = true; \
		for (uint32_t i = 0; i < 16; ++i) \
		{ \
			if (!(rl & (1 << i))) \
				continue; \
			if (post_pre == down_up) \
				rn += 4; \
			if (st_ld) \
			{ \
				uint32_t v = cpu->get32(cpu->mem, rn, MEM_DATA_SEQ); \
				if (i == CPU_REG_PC) \
				{ \
					pc_inc = false; \
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
						v &= ~3; \
					} \
				} \
				if (usermode) \
				{ \
					if (i == rnr && CPU_GET_MODE(cpu) == CPU_MODE_USR) \
						nowriteback = true; \
					cpu->regs.r[i] = v; \
				} \
				else \
				{ \
					if (i == rnr) \
						nowriteback = true; \
					cpu_set_reg(cpu, i, v); \
				} \
			} \
			else \
			{ \
				uint32_t v; \
				if (usermode) \
				{ \
					if (i == rnr && CPU_GET_MODE(cpu) == CPU_MODE_USR) \
					{ \
						if (first) \
							v = old_base; \
						else \
							v = new_base; \
					} \
					else \
					{ \
						v = cpu->regs.r[i]; \
					} \
				} \
				else \
				{ \
					if (i == rnr) \
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
				} \
				if (i == CPU_REG_PC) \
					v += 12; \
				cpu->set32(cpu->mem, rn, v, MEM_DATA_SEQ); \
			} \
			if (post_pre != down_up) \
				rn += 4; \
			first = false; \
		} \
	} \
	else \
	{ \
		if (post_pre == down_up) \
			rn += 0x4; \
		if (st_ld) \
		{ \
			uint32_t v = cpu->get32(cpu->mem, rn, MEM_DATA_SEQ); \
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
				v &= ~3; \
			} \
			cpu_set_reg(cpu, CPU_REG_PC, v); \
			pc_inc = false; \
		} \
		else \
		{ \
			uint32_t v = cpu_get_reg(cpu, CPU_REG_PC) + 12; \
			cpu->set32(cpu->mem, rn, v, MEM_DATA_SEQ); \
		} \
		if (post_pre != down_up) \
			rn += 0x4; \
		rn += 0x3C; \
	} \
	if (writeback && !nowriteback && down_up) \
		cpu_set_reg(cpu, rnr, rn); \
	if (pc_inc) \
		cpu_inc_pc(cpu, 4); \
	cpu->instr_delay++; \
}, \
{ \
	uint32_t rn = (cpu->instr_opcode >> 16) & 0xF; \
	uint32_t rl = (cpu->instr_opcode >>  0) & 0xFFFF; \
	int res; \
	char *tmpd = data; \
	size_t tmps = size; \
	res = snprintf(data, size, #opname " r%" PRIu32 "%s, {", rn, writeback ? "!" : ""); \
	tmpd += res; \
	tmps -= res; \
	if (rl) \
	{ \
		for (int i = 0; i < 16; ++i) \
		{ \
			if (!(rl & (1 << i))) \
				continue; \
			snprintf(tmpd, tmps, "r%d,", i); \
			tmpd += i > 9 ? 4 : 3; \
			tmps -= i > 9 ? 4 : 3; \
		} \
		tmpd--; \
		tmps++; \
	} \
	snprintf(tmpd, tmps, "}%s", usermode ? "^" : ""); \
})

STLDM(stmda,    , 0, 0, 0, 0, 0);
STLDM(stmdb,    , 1, 0, 0, 0, 0);
STLDM(stmia,    , 0, 1, 0, 0, 0);
STLDM(stmib,    , 1, 1, 0, 0, 0);
STLDM(stmda, _u , 0, 0, 1, 0, 0);
STLDM(stmdb, _u , 1, 0, 1, 0, 0);
STLDM(stmia, _u , 0, 1, 1, 0, 0);
STLDM(stmib, _u , 1, 1, 1, 0, 0);
STLDM(stmda, _w , 0, 0, 0, 1, 0);
STLDM(stmdb, _w , 1, 0, 0, 1, 0);
STLDM(stmia, _w , 0, 1, 0, 1, 0);
STLDM(stmib, _w , 1, 1, 0, 1, 0);
STLDM(stmda, _uw, 0, 0, 1, 1, 0);
STLDM(stmdb, _uw, 1, 0, 1, 1, 0);
STLDM(stmia, _uw, 0, 1, 1, 1, 0);
STLDM(stmib, _uw, 1, 1, 1, 1, 0);
STLDM(ldmda,    , 0, 0, 0, 0, 1);
STLDM(ldmdb,    , 1, 0, 0, 0, 1);
STLDM(ldmia,    , 0, 1, 0, 0, 1);
STLDM(ldmib,    , 1, 1, 0, 0, 1);
STLDM(ldmda, _u , 0, 0, 1, 0, 1);
STLDM(ldmdb, _u , 1, 0, 1, 0, 1);
STLDM(ldmia, _u , 0, 1, 1, 0, 1);
STLDM(ldmib, _u , 1, 1, 1, 0, 1);
STLDM(ldmda, _w , 0, 0, 0, 1, 1);
STLDM(ldmdb, _w , 1, 0, 0, 1, 1);
STLDM(ldmia, _w , 0, 1, 0, 1, 1);
STLDM(ldmib, _w , 1, 1, 0, 1, 1);
STLDM(ldmda, _uw, 0, 0, 1, 1, 1);
STLDM(ldmdb, _uw, 1, 0, 1, 1, 1);
STLDM(ldmia, _uw, 0, 1, 1, 1, 1);
STLDM(ldmib, _uw, 1, 1, 1, 1, 1);

#define STLDC(v) \
ARM_INSTR(stc_##v, \
{ \
	(void)cpu; \
	assert(!"unimp"); \
}, \
{ \
	snprintf(data, size, "stc " #v); \
}); \
ARM_INSTR(ldc_##v, \
{ \
	(void)cpu; \
	assert(!"unimp"); \
}, \
{ \
	snprintf(data, size, "ldc " #v); \
})

STLDC(ofm);
STLDC(prm);
STLDC(ofp);
STLDC(prp);
STLDC(unm);
STLDC(ptm);
STLDC(unp);
STLDC(ptp);

ARM_INSTR(clz,
{
	uint32_t rd = (cpu->instr_opcode >> 12) & 0xF;
	uint32_t rmr = (cpu->instr_opcode >>  0) & 0xF;
	uint32_t rm = cpu_get_reg(cpu, rmr);
	uint32_t nzero = 0;
	for (int i = 31; i >= 0; --i)
	{
		if (rm & (1 << i))
			break;
		nzero++;
	}
	cpu_set_reg(cpu, rd, nzero);
	cpu_inc_pc(cpu, 4);
},
{
	uint32_t rd = (cpu->instr_opcode >> 12) & 0xF;
	uint32_t rm = (cpu->instr_opcode >>  0) & 0xF;
	snprintf(data, size, "clz r%" PRIu32 ", r%" PRIu32, rd, rm);
});

ARM_INSTR(bkpt,
{
	(void)cpu;
	assert(!"unimp");
},
{
	snprintf(data, size, "bkpt");
});

#define ARM_MRS(n, psr) \
ARM_INSTR(mrs_##n, \
{ \
	uint32_t v; \
	if (psr) \
		v = *cpu->regs.spsr; \
	else \
		v = cpu->regs.cpsr; \
	uint32_t rd = (cpu->instr_opcode >> 12) & 0xF; \
	cpu_set_reg(cpu, rd, v); \
	cpu_inc_pc(cpu, 4); \
	cpu->instr_delay += 1; \
}, \
{ \
	uint32_t rd = (cpu->instr_opcode >> 12) & 0xF; \
	snprintf(data, size, "mrs r%" PRIu32 ", %s", rd, psr ? "spsr" : "cpsr"); \
})

ARM_MRS(rc, 0);
ARM_MRS(rs, 1);

#define ARM_MSR(n, imm, psr) \
ARM_INSTR(msr_##n, \
{ \
	uint32_t v; \
	if (imm) \
		v = ARM_ROR(cpu->instr_opcode & 0xFF, ((cpu->instr_opcode >> 8) & 0xF) * 2); \
	else \
		v = cpu_get_reg(cpu, cpu->instr_opcode & 0xF); \
	uint32_t mask = 0; \
	if (cpu->instr_opcode & (1 << 19)) \
		mask |= 0xFF000000ULL; \
	if (CPU_GET_MODE(cpu) != CPU_MODE_USR) \
	{ \
		if (cpu->instr_opcode & (1 << 18)) \
			mask |= 0x00FF0000; \
		if (cpu->instr_opcode & (1 << 17)) \
			mask |= 0x0000FF00; \
		if (cpu->instr_opcode & (1 << 16)) \
		{ \
			mask |= 0x000000DF; \
			if (psr) \
				mask |= 0x00000020; \
		} \
	} \
	if (psr) \
	{ \
		if (CPU_GET_MODE(cpu) != CPU_MODE_USR && CPU_GET_MODE(cpu) != CPU_MODE_SYS) \
			*cpu->regs.spsr = (*cpu->regs.spsr & ~mask) | (v & mask); \
	} \
	else \
	{ \
		cpu->regs.cpsr = (cpu->regs.cpsr & ~mask) | (v & mask); \
	} \
	cpu_inc_pc(cpu, 4); \
	if (!psr) \
		cpu_update_mode(cpu); \
	cpu->instr_delay += 1; \
}, \
{ \
	char flags[5]; \
	memset(flags, 0, sizeof(flags)); \
	size_t fn = 0; \
	if (cpu->instr_opcode & (1 << 19)) \
		flags[fn++] = 'f'; \
	if (cpu->instr_opcode & (1 << 18)) \
		flags[fn++] = 's'; \
	if (cpu->instr_opcode & (1 << 17)) \
		flags[fn++] = 'x'; \
	if (cpu->instr_opcode & (1 << 16)) \
		flags[fn++] = 'c'; \
	if (imm) \
		snprintf(data, size, "msr %s_%s #0x%" PRIx32 ", ROR #0x%" PRIx32, psr ? "spsr" : "cpsr", flags, cpu->instr_opcode & 0xFF, ((cpu->instr_opcode >> 8) & 0xF) * 2); \
	else \
		snprintf(data, size, "msr %s_%s r%" PRIu32, psr ? "spsr" : "cpsr", flags, cpu->instr_opcode & 0xF); \
})

ARM_MSR(rc, 0, 0);
ARM_MSR(rs, 0, 1);
ARM_MSR(ic, 1, 0);
ARM_MSR(is, 1, 1);

ARM_INSTR(qadd,
{
	(void)cpu;
	assert(!"unimp");
},
{
	snprintf(data, size, "qadd");
});

ARM_INSTR(qsub,
{
	(void)cpu;
	assert(!"unimp");
},
{
	(void)cpu;
	snprintf(data, size, "qsub");
});

ARM_INSTR(qdadd,
{
	(void)cpu;
	assert(!"unimp");
},
{
	snprintf(data, size, "qdadd");
});

ARM_INSTR(qdsub,
{
	(void)cpu;
	assert(!"unimp");
},
{
	snprintf(data, size, "qdsub");
});

#define ARM_SWP(n, word_byte) \
ARM_INSTR(n, \
{ \
	uint32_t rd  = (cpu->instr_opcode >> 12) & 0xF; \
	uint32_t rm  = (cpu->instr_opcode >>  0) & 0xF; \
	uint32_t rnr = (cpu->instr_opcode >> 16) & 0xF; \
	uint32_t rn = cpu_get_reg(cpu, rnr); \
	if (word_byte) \
	{ \
		uint8_t v = cpu->get8(cpu->mem, rn, MEM_DATA_SEQ); \
		cpu->set8(cpu->mem, rn, cpu_get_reg(cpu, rm), MEM_DATA_SEQ); \
		cpu_set_reg(cpu, rd, v); \
	} \
	else \
	{ \
		uint32_t v = cpu->get32(cpu->mem, rn & ~3, MEM_DATA_SEQ); \
		v = ARM_ROR(v, (rn & 3) * 8); \
		cpu->set32(cpu->mem, rn, cpu_get_reg(cpu, rm), MEM_DATA_SEQ); \
		cpu_set_reg(cpu, rd, v); \
	} \
	cpu_inc_pc(cpu, 4); \
	cpu->instr_delay++; \
}, \
{ \
	uint32_t rd = (cpu->instr_opcode >> 12) & 0xF; \
	uint32_t rm = (cpu->instr_opcode >>  0) & 0xF; \
	uint32_t rn = (cpu->instr_opcode >> 16) & 0xF; \
	snprintf(data, size, #n " r%" PRIu32 ", r%" PRIu32 ", [r%" PRIu32 "]", rd, rm, rn); \
})

ARM_SWP(swp , 0);
ARM_SWP(swpb, 1);

ARM_INSTR(b,
{
	int32_t v = cpu->instr_opcode & 0x7FFFFF;
	if (cpu->instr_opcode & 0x800000)
		v = -(~v & 0x7FFFFF) - 1;
	cpu_inc_pc(cpu, 8 + 4 * v);
},
{
	int32_t v = cpu->instr_opcode & 0x7FFFFF;
	if (cpu->instr_opcode & 0x800000)
		v = -(~v & 0x7FFFFF) - 1;
	snprintf(data, size, "b %c0x%" PRIx32, v < 0 ? '-' : '+', v < 0 ? -v * 4 : v * 4);
});

ARM_INSTR(bl,
{
	int32_t v = cpu->instr_opcode & 0x7FFFFF;
	if (cpu->instr_opcode & 0x800000)
		v = -(~v & 0x7FFFFF) - 1;
	cpu_set_reg(cpu, CPU_REG_LR, cpu_get_reg(cpu, CPU_REG_PC) + 4);
	cpu_inc_pc(cpu, 8 + 4 * v);
},
{
	int32_t v = cpu->instr_opcode & 0x7FFFFF;
	if (cpu->instr_opcode & 0x800000)
		v = -(~v & 0x7FFFFF) - 1;
	snprintf(data, size, "bl %c0x%" PRIx32, v < 0 ? '-' : '+', v < 0 ? -v * 4 : v * 4);
});

ARM_INSTR(bx,
{
	uint8_t rnr = cpu->instr_opcode & 0xF;
	uint32_t rn = cpu_get_reg(cpu, rnr);
	cpu_set_reg(cpu, CPU_REG_PC, rn & ~1);
	CPU_SET_FLAG_T(cpu, rn & 0x1);
},
{
	uint8_t rn = cpu->instr_opcode & 0xF;
	snprintf(data, size, "bx r%" PRIu8, rn);
});

ARM_INSTR(blx,
{
	uint8_t rnr = cpu->instr_opcode & 0xF;
	uint32_t rn = cpu_get_reg(cpu, rnr);
	cpu_set_reg(cpu, CPU_REG_LR, cpu_get_reg(cpu, CPU_REG_PC) + 4);
	cpu_set_reg(cpu, CPU_REG_PC, rn & ~1);
	CPU_SET_FLAG_T(cpu, rn & 0x1);
},
{
	uint8_t rn = cpu->instr_opcode & 0xF;
	snprintf(data, size, "blx r%" PRIu8, rn);
});

ARM_INSTR(cdp,
{
	(void)cpu;
	assert(!"unimp");
},
{
	(void)cpu;
	snprintf(data, size, "cdp");
});

ARM_INSTR(mcr,
{
	uint8_t opc = (cpu->instr_opcode >> 21) & 0x7;
	uint8_t cn  = (cpu->instr_opcode >> 16) & 0xF;
	uint8_t rd  = (cpu->instr_opcode >> 12) & 0xF;
	uint8_t pn  = (cpu->instr_opcode >>  8) & 0xF;
	uint8_t cp  = (cpu->instr_opcode >>  5) & 0x7;
	uint8_t cm  = (cpu->instr_opcode >>  0) & 0xF;
	assert(cpu->arm9);
	assert(pn == 15);
	assert(opc == 0);
	cp15_write(cpu, cn, cm, cp, cpu_get_reg(cpu, rd));
	cpu_inc_pc(cpu, 4);
	cpu->instr_delay += 2;
},
{
	uint8_t opc = (cpu->instr_opcode >> 21) & 0x7;
	uint8_t cn  = (cpu->instr_opcode >> 16) & 0xF;
	uint8_t rd  = (cpu->instr_opcode >> 12) & 0xF;
	uint8_t pn  = (cpu->instr_opcode >>  8) & 0xF;
	uint8_t cp  = (cpu->instr_opcode >>  5) & 0x7;
	uint8_t cm  = (cpu->instr_opcode >>  0) & 0xF;
	snprintf(data, size, "mcr p%d, %d, r%d, cr%d, cr%d, %d", pn, opc, rd, cn, cm, cp);
});

ARM_INSTR(mrc,
{
	uint8_t opc = (cpu->instr_opcode >> 21) & 0x7;
	uint8_t cn  = (cpu->instr_opcode >> 16) & 0xF;
	uint8_t rd  = (cpu->instr_opcode >> 12) & 0xF;
	uint8_t pn  = (cpu->instr_opcode >>  8) & 0xF;
	uint8_t cp  = (cpu->instr_opcode >>  5) & 0x7;
	uint8_t cm  = (cpu->instr_opcode >>  0) & 0xF;
	assert(cpu->arm9);
	assert(pn == 15);
	assert(opc == 0);
	cpu_set_reg(cpu, rd, cp15_read(cpu, cn, cm, cp));
	cpu_inc_pc(cpu, 4);
	cpu->instr_delay += 2;
},
{
	uint8_t opc = (cpu->instr_opcode >> 21) & 0x7;
	uint8_t cn  = (cpu->instr_opcode >> 16) & 0xF;
	uint8_t rd  = (cpu->instr_opcode >> 12) & 0xF;
	uint8_t pn  = (cpu->instr_opcode >>  8) & 0xF;
	uint8_t cp  = (cpu->instr_opcode >>  5) & 0x7;
	uint8_t cm  = (cpu->instr_opcode >>  0) & 0xF;
	snprintf(data, size, "mrc p%d, %d, r%d, cr%d, cr%d, %d", pn, opc, rd, cn, cm, cp);
});

ARM_INSTR(swi,
{
	cpu->regs.spsr_modes[1] = cpu->regs.cpsr;
	CPU_SET_MODE(cpu, CPU_MODE_SVC);
	cpu_update_mode(cpu);
	CPU_SET_FLAG_I(cpu, 1);
	cpu_set_reg(cpu, CPU_REG_LR, cpu_get_reg(cpu, CPU_REG_PC) + 4);
	if (cpu->arm9 && (cpu->cp15.cr & 0x2000))
		cpu_set_reg(cpu, CPU_REG_PC, 0xFFFF0008);
	else
		cpu_set_reg(cpu, CPU_REG_PC, 0x8);
},
{
	uint32_t nn = cpu->instr_opcode & 0xF;
	nn |= (cpu->instr_opcode >> 4) & 0xFFF0;
	snprintf(data, size, "swi 0x%" PRIx32, nn);
});

ARM_INSTR(undef,
{
	(void)cpu;
	printf("undef 0x%08" PRIx32 "\n", cpu->instr_opcode);
	cpu->debug = CPU_DEBUG_ALL_ML;
	//assert(!"unimp");
	cpu_inc_pc(cpu, 4);
},
{
	snprintf(data, size, "undef");
});

ARM_INSTR(blx_imm,
{
	int32_t v = cpu->instr_opcode & 0x7FFFFF;
	if (cpu->instr_opcode & 0x800000)
		v = -(~v & 0x7FFFFF) - 1;
	cpu_set_reg(cpu, CPU_REG_LR, cpu_get_reg(cpu, CPU_REG_PC) + 4);
	v *= 4;
	v += ((cpu->instr_opcode >> 23) & 0x2);
	cpu_inc_pc(cpu, 8 + v);
	CPU_SET_FLAG_T(cpu, 1);
},
{
	int32_t v = cpu->instr_opcode & 0x7FFFFF;
	if (cpu->instr_opcode & 0x800000)
		v = -(~v & 0x7FFFFF) - 1;
	snprintf(data, size, "blx %c0x%" PRIx32, v < 0 ? '-' : '+', v < 0 ? -v * 4 : v * 4);
});

#define REPEAT1(v) &arm_##v
#define REPEAT2(v) REPEAT1(v), REPEAT1(v)
#define REPEAT4(v) REPEAT2(v), REPEAT2(v)
#define REPEAT8(v) REPEAT4(v), REPEAT4(v)
#define REPEAT16(v) REPEAT8(v), REPEAT8(v)
#define REPEAT32(v) REPEAT16(v), REPEAT16(v)
#define REPEAT64(v) REPEAT32(v), REPEAT32(v)
#define REPEAT128(v) REPEAT64(v), REPEAT64(v)
#define REPEAT256(v) REPEAT128(v), REPEAT128(v)

#define ALU_OP(op, v9, vB, vD, vF) \
	REPEAT1(op##_lli), REPEAT1(op##_llr), \
	REPEAT1(op##_lri), REPEAT1(op##_lrr), \
	REPEAT1(op##_ari), REPEAT1(op##_arr), \
	REPEAT1(op##_rri), REPEAT1(op##_rrr), \
	REPEAT1(op##_lli), REPEAT1(v9), \
	REPEAT1(op##_lri), REPEAT1(vB), \
	REPEAT1(op##_ari), REPEAT1(vD), \
	REPEAT1(op##_rri), REPEAT1(vF)

#define ALU_OPS(op, v09, v0B, v0D, v0F, v19, v1B, v1D, v1F) \
	ALU_OP(op, v09, v0B, v0D, v0F), \
	ALU_OP(op##s, v19, v1B, v1D, v1F)

#define CDP_LINE(v) \
	REPEAT1(cdp), REPEAT1(v), REPEAT1(cdp), REPEAT1(v), \
	REPEAT1(cdp), REPEAT1(v), REPEAT1(cdp), REPEAT1(v), \
	REPEAT1(cdp), REPEAT1(v), REPEAT1(cdp), REPEAT1(v), \
	REPEAT1(cdp), REPEAT1(v), REPEAT1(cdp), REPEAT1(v)

#define STLD_BLANK(v) \
	REPEAT1(v##ll), REPEAT1(undef), \
	REPEAT1(v##lr), REPEAT1(undef), \
	REPEAT1(v##ar), REPEAT1(undef), \
	REPEAT1(v##rr), REPEAT1(undef), \
	REPEAT1(v##ll), REPEAT1(undef), \
	REPEAT1(v##lr), REPEAT1(undef), \
	REPEAT1(v##ar), REPEAT1(undef), \
	REPEAT1(v##rr), REPEAT1(undef)

const struct cpu_instr *cpu_instr_arm[0x1000] =
{
	/* 0x000 */ ALU_OPS(and, mul  , strh_ptrm, ldrd_ptrm, strd_ptrm, muls  , ldrh_ptrm, ldrsb_ptrm, ldrsh_ptrm),
	/* 0x020 */ ALU_OPS(eor, mla  , strh_ptrm, ldrd_ptrm, strd_ptrm, mlas  , ldrh_ptrm, ldrsb_ptrm, ldrsh_ptrm),
	/* 0x040 */ ALU_OPS(sub, undef, strh_ptim, ldrd_ptim, strd_ptim, undef , ldrh_ptim, ldrsb_ptim, ldrsh_ptim),
	/* 0x060 */ ALU_OPS(rsb, undef, strh_ptim, ldrd_ptim, strd_ptim, undef , ldrh_ptim, ldrsb_ptim, ldrsh_ptim),
	/* 0x080 */ ALU_OPS(add, umull, strh_ptrp, ldrd_ptrp, strd_ptrp, umulls, ldrh_ptrp, ldrsb_ptrp, ldrsh_ptrp),
	/* 0x0A0 */ ALU_OPS(adc, umlal, strh_ptrp, ldrd_ptrp, strd_ptrp, umlals, ldrh_ptrp, ldrsb_ptrp, ldrsh_ptrp),
	/* 0x0C0 */ ALU_OPS(sbc, smull, strh_ptip, ldrd_ptip, strd_ptip, smulls, ldrh_ptip, ldrsb_ptip, ldrsh_ptip),
	/* 0x0E0 */ ALU_OPS(rsc, smlal, strh_ptip, ldrd_ptip, strd_ptip, smlals, ldrh_ptip, ldrsb_ptip, ldrsh_ptip),
	/* 0x100 */ REPEAT1(mrs_rc) , REPEAT1(undef)    , REPEAT1(undef)  , REPEAT1(undef),
	/* 0x104 */ REPEAT1(undef)  , REPEAT1(qadd)     , REPEAT1(undef)  , REPEAT1(undef),
	/* 0x108 */ REPEAT1(smlabb) , REPEAT1(swp)      , REPEAT1(smlatb) , REPEAT1(strh_ofrm),
	/* 0x10C */ REPEAT1(smlabt) , REPEAT1(ldrd_ofrm), REPEAT1(smlatt) , REPEAT1(strd_ofrm),
	/* 0x110 */ ALU_OP(tsts, undef, ldrh_ofrm, ldrsb_ofrm, ldrsh_ofrm),
	/* 0x120 */ REPEAT1(msr_rc) , REPEAT1(bx)       , REPEAT1(undef)  , REPEAT1(blx),
	/* 0x124 */ REPEAT1(undef)  , REPEAT1(qsub)     , REPEAT1(undef)  , REPEAT1(bkpt),
	/* 0x128 */ REPEAT1(smlawb) , REPEAT1(undef)    , REPEAT1(smulwb) , REPEAT1(strh_prrm),
	/* 0x12C */ REPEAT1(smlawt) , REPEAT1(ldrd_prrm), REPEAT1(smulwt) , REPEAT1(strd_prrm),
	/* 0x130 */ ALU_OP(teqs, undef, ldrh_prrm, ldrsb_prrm, ldrsh_prrm),
	/* 0x140 */ REPEAT1(mrs_rs) , REPEAT1(undef)    , REPEAT1(undef)  , REPEAT1(undef),
	/* 0x144 */ REPEAT1(undef)  , REPEAT1(qdadd)    , REPEAT1(undef)  , REPEAT1(undef),
	/* 0x148 */ REPEAT1(smlalbb), REPEAT1(swpb)     , REPEAT1(smlaltb), REPEAT1(strh_ofim),
	/* 0x14C */ REPEAT1(smlalbt), REPEAT1(ldrd_ofim), REPEAT1(smlaltt), REPEAT1(strd_ofim),
	/* 0x150 */ ALU_OP(cmps, undef, ldrh_ofim, ldrsb_ofim, ldrsh_ofim),
	/* 0x160 */ REPEAT1(msr_rs) , REPEAT1(clz)      , REPEAT1(undef)  , REPEAT1(undef),
	/* 0x164 */ REPEAT1(undef)  , REPEAT1(qdsub)    , REPEAT1(undef)  , REPEAT1(undef),
	/* 0x168 */ REPEAT1(smulbb) , REPEAT1(undef)    , REPEAT1(smultb) , REPEAT1(strh_prim),
	/* 0x16C */ REPEAT1(smulbt) , REPEAT1(ldrd_prim), REPEAT1(smultt) , REPEAT1(strd_prim),
	/* 0x170 */ ALU_OP(cmns, undef, ldrh_prim, ldrsb_prim, ldrsh_prim),
	/* 0x180 */ ALU_OPS(orr, undef, strh_ofrp, ldrd_ofrp, strd_ofrp, undef, ldrh_ofrp, ldrsb_ofrp, ldrsh_ofrp),
	/* 0x1A0 */ ALU_OPS(mov, undef, strh_prrp, ldrd_prrp, strd_prrp, undef, ldrh_prrp, ldrsb_prrp, ldrsh_prrp),
	/* 0x1C0 */ ALU_OPS(bic, undef, strh_ofip, ldrd_ofip, strd_ofip, undef, ldrh_ofip, ldrsb_ofip, ldrsh_ofip),
	/* 0x1E0 */ ALU_OPS(mvn, undef, strh_prip, ldrd_prip, strd_prip, undef, ldrh_prip, ldrsb_prip, ldrsh_prip),
	/* 0x200 */ REPEAT16(and_imm),
	/* 0x210 */ REPEAT16(ands_imm),
	/* 0x220 */ REPEAT16(eor_imm),
	/* 0x230 */ REPEAT16(eors_imm),
	/* 0x240 */ REPEAT16(sub_imm),
	/* 0x250 */ REPEAT16(subs_imm),
	/* 0x260 */ REPEAT16(rsb_imm),
	/* 0x270 */ REPEAT16(rsbs_imm),
	/* 0x280 */ REPEAT16(add_imm),
	/* 0x290 */ REPEAT16(adds_imm),
	/* 0x2A0 */ REPEAT16(adc_imm),
	/* 0x2B0 */ REPEAT16(adcs_imm),
	/* 0x2C0 */ REPEAT16(sbc_imm),
	/* 0x2D0 */ REPEAT16(sbcs_imm),
	/* 0x2E0 */ REPEAT16(rsc_imm),
	/* 0x2F0 */ REPEAT16(rscs_imm),
	/* 0x300 */ REPEAT16(undef),
	/* 0x310 */ REPEAT16(tsts_imm),
	/* 0x320 */ REPEAT16(msr_ic),
	/* 0x330 */ REPEAT16(teqs_imm),
	/* 0x340 */ REPEAT16(undef),
	/* 0x350 */ REPEAT16(cmps_imm),
	/* 0x360 */ REPEAT16(msr_is),
	/* 0x370 */ REPEAT16(cmns_imm),
	/* 0x380 */ REPEAT16(orr_imm),
	/* 0x390 */ REPEAT16(orrs_imm),
	/* 0x3A0 */ REPEAT16(mov_imm),
	/* 0x3B0 */ REPEAT16(movs_imm),
	/* 0x3C0 */ REPEAT16(bic_imm),
	/* 0x3D0 */ REPEAT16(bics_imm),
	/* 0x3E0 */ REPEAT16(mvn_imm),
	/* 0x3F0 */ REPEAT16(mvns_imm),
	/* 0x400 */ REPEAT16(str_ptim),
	/* 0x410 */ REPEAT16(ldr_ptim),
	/* 0x420 */ REPEAT16(strt_ptim),
	/* 0x430 */ REPEAT16(ldrt_ptim),
	/* 0x440 */ REPEAT16(strb_ptim),
	/* 0x450 */ REPEAT16(ldrb_ptim),
	/* 0x460 */ REPEAT16(strbt_ptim),
	/* 0x470 */ REPEAT16(ldrbt_ptim),
	/* 0x480 */ REPEAT16(str_ptip),
	/* 0x490 */ REPEAT16(ldr_ptip),
	/* 0x4A0 */ REPEAT16(strt_ptip),
	/* 0x4B0 */ REPEAT16(ldrt_ptip),
	/* 0x4C0 */ REPEAT16(strb_ptip),
	/* 0x4D0 */ REPEAT16(ldrb_ptip),
	/* 0x4E0 */ REPEAT16(strbt_ptip),
	/* 0x4F0 */ REPEAT16(ldrbt_ptip),
	/* 0x500 */ REPEAT16(str_ofim),
	/* 0x510 */ REPEAT16(ldr_ofim),
	/* 0x520 */ REPEAT16(str_prim),
	/* 0x530 */ REPEAT16(ldr_prim),
	/* 0x540 */ REPEAT16(strb_ofim),
	/* 0x550 */ REPEAT16(ldrb_ofim),
	/* 0x560 */ REPEAT16(strb_prim),
	/* 0x570 */ REPEAT16(ldrb_prim),
	/* 0x580 */ REPEAT16(str_ofip),
	/* 0x590 */ REPEAT16(ldr_ofip),
	/* 0x5A0 */ REPEAT16(str_prip),
	/* 0x5B0 */ REPEAT16(ldr_prip),
	/* 0x5C0 */ REPEAT16(strb_ofip),
	/* 0x5D0 */ REPEAT16(ldrb_ofip),
	/* 0x5E0 */ REPEAT16(strb_prip),
	/* 0x5F0 */ REPEAT16(ldrb_prip),
	/* 0x600 */ STLD_BLANK(str_ptrm),
	/* 0x610 */ STLD_BLANK(ldr_ptrm),
	/* 0x620 */ STLD_BLANK(strt_ptrm),
	/* 0x630 */ STLD_BLANK(ldrt_ptrm),
	/* 0x640 */ STLD_BLANK(strb_ptrm),
	/* 0x650 */ STLD_BLANK(ldrb_ptrm),
	/* 0x660 */ STLD_BLANK(strbt_ptrm),
	/* 0x670 */ STLD_BLANK(ldrbt_ptrm),
	/* 0x680 */ STLD_BLANK(str_ptrp),
	/* 0x690 */ STLD_BLANK(ldr_ptrp),
	/* 0x6A0 */ STLD_BLANK(strt_ptrp),
	/* 0x6B0 */ STLD_BLANK(ldrt_ptrp),
	/* 0x6C0 */ STLD_BLANK(strb_ptrp),
	/* 0x6D0 */ STLD_BLANK(ldrb_ptrp),
	/* 0x6E0 */ STLD_BLANK(strbt_ptrp),
	/* 0x6F0 */ STLD_BLANK(ldrbt_ptrp),
	/* 0x700 */ STLD_BLANK(str_ofrm),
	/* 0x710 */ STLD_BLANK(ldr_ofrm),
	/* 0x720 */ STLD_BLANK(str_prrm),
	/* 0x730 */ STLD_BLANK(ldr_prrm),
	/* 0x740 */ STLD_BLANK(strb_ofrm),
	/* 0x750 */ STLD_BLANK(ldrb_ofrm),
	/* 0x760 */ STLD_BLANK(strb_prrm),
	/* 0x770 */ STLD_BLANK(ldrb_prrm),
	/* 0x780 */ STLD_BLANK(str_ofrp),
	/* 0x790 */ STLD_BLANK(ldr_ofrp),
	/* 0x7A0 */ STLD_BLANK(str_prrp),
	/* 0x7B0 */ STLD_BLANK(ldr_prrp),
	/* 0x7C0 */ STLD_BLANK(strb_ofrp),
	/* 0x7D0 */ STLD_BLANK(ldrb_ofrp),
	/* 0x7E0 */ STLD_BLANK(strb_prrp),
	/* 0x7F0 */ STLD_BLANK(ldrb_prrp),
	/* 0x800 */ REPEAT16(stmda),
	/* 0x810 */ REPEAT16(ldmda),
	/* 0x820 */ REPEAT16(stmda_w),
	/* 0x830 */ REPEAT16(ldmda_w),
	/* 0x840 */ REPEAT16(stmda_u),
	/* 0x850 */ REPEAT16(ldmda_u),
	/* 0x860 */ REPEAT16(stmda_uw),
	/* 0x870 */ REPEAT16(ldmda_uw),
	/* 0x880 */ REPEAT16(stmia),
	/* 0x890 */ REPEAT16(ldmia),
	/* 0x8A0 */ REPEAT16(stmia_w),
	/* 0x8B0 */ REPEAT16(ldmia_w),
	/* 0x8C0 */ REPEAT16(stmia_u),
	/* 0x8D0 */ REPEAT16(ldmia_u),
	/* 0x8E0 */ REPEAT16(stmia_uw),
	/* 0x8F0 */ REPEAT16(ldmia_uw),
	/* 0x900 */ REPEAT16(stmdb),
	/* 0x910 */ REPEAT16(ldmdb),
	/* 0x920 */ REPEAT16(stmdb_w),
	/* 0x930 */ REPEAT16(ldmdb_w),
	/* 0x940 */ REPEAT16(stmdb_u),
	/* 0x950 */ REPEAT16(ldmdb_u),
	/* 0x960 */ REPEAT16(stmdb_uw),
	/* 0x970 */ REPEAT16(ldmdb_uw),
	/* 0x980 */ REPEAT16(stmib),
	/* 0x990 */ REPEAT16(ldmib),
	/* 0x9A0 */ REPEAT16(stmib_w),
	/* 0x9B0 */ REPEAT16(ldmib_w),
	/* 0x9C0 */ REPEAT16(stmib_u),
	/* 0x9D0 */ REPEAT16(ldmib_u),
	/* 0x9E0 */ REPEAT16(stmib_uw),
	/* 0x9F0 */ REPEAT16(ldmib_uw),
	/* 0xA00 */ REPEAT256(b),
	/* 0xB00 */ REPEAT256(bl),
	/* 0xC00 */ REPEAT16(stc_ofm),
	/* 0xC10 */ REPEAT16(ldc_ofm),
	/* 0xC20 */ REPEAT16(stc_prm),
	/* 0xC30 */ REPEAT16(ldc_prm),
	/* 0xC40 */ REPEAT16(stc_ofm),
	/* 0xC50 */ REPEAT16(ldc_ofm),
	/* 0xC60 */ REPEAT16(stc_prm),
	/* 0xC70 */ REPEAT16(ldc_prm),
	/* 0xC80 */ REPEAT16(stc_ofp),
	/* 0xC90 */ REPEAT16(ldc_ofp),
	/* 0xCA0 */ REPEAT16(stc_prp),
	/* 0xCB0 */ REPEAT16(ldc_prp),
	/* 0xCC0 */ REPEAT16(stc_ofp),
	/* 0xCD0 */ REPEAT16(ldc_ofp),
	/* 0xCE0 */ REPEAT16(stc_prp),
	/* 0xCF0 */ REPEAT16(ldc_prp),
	/* 0xD00 */ REPEAT16(stc_unm),
	/* 0xD10 */ REPEAT16(ldc_unm),
	/* 0xD20 */ REPEAT16(stc_ptm),
	/* 0xD30 */ REPEAT16(ldc_ptm),
	/* 0xD40 */ REPEAT16(stc_unm),
	/* 0xD50 */ REPEAT16(ldc_unm),
	/* 0xD60 */ REPEAT16(stc_ptm),
	/* 0xD70 */ REPEAT16(ldc_ptm),
	/* 0xD80 */ REPEAT16(stc_unp),
	/* 0xD90 */ REPEAT16(ldc_unp),
	/* 0xDA0 */ REPEAT16(stc_ptp),
	/* 0xDB0 */ REPEAT16(ldc_ptp),
	/* 0xDC0 */ REPEAT16(stc_unp),
	/* 0xDD0 */ REPEAT16(ldc_unp),
	/* 0xDE0 */ REPEAT16(stc_ptp),
	/* 0xDF0 */ REPEAT16(ldc_ptp),
	/* 0xE00 */ CDP_LINE(mcr),
	/* 0xE10 */ CDP_LINE(mrc),
	/* 0xE20 */ CDP_LINE(mcr),
	/* 0xE30 */ CDP_LINE(mrc),
	/* 0xE40 */ CDP_LINE(mcr),
	/* 0xE50 */ CDP_LINE(mrc),
	/* 0xE60 */ CDP_LINE(mcr),
	/* 0xE70 */ CDP_LINE(mrc),
	/* 0xE80 */ CDP_LINE(mcr),
	/* 0xE90 */ CDP_LINE(mrc),
	/* 0xEA0 */ CDP_LINE(mcr),
	/* 0xEB0 */ CDP_LINE(mrc),
	/* 0xEC0 */ CDP_LINE(mcr),
	/* 0xED0 */ CDP_LINE(mrc),
	/* 0xEE0 */ CDP_LINE(mcr),
	/* 0xEF0 */ CDP_LINE(mrc),
	/* 0xF00 */ REPEAT256(swi),
};

const struct cpu_instr *cpu_instr_blx_imm = &arm_blx_imm;
