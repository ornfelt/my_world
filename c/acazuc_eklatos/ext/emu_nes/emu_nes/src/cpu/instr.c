#include "../internal.h"
#include "instr.h"
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define CPU_INSTR(name, exec_fn, print_fn) \
static void exec_##name(struct cpu *cpu) \
exec_fn \
static void print_##name(struct cpu *cpu, char *data, size_t size) \
print_fn \
static const struct cpu_instr name = \
{ \
	.exec = exec_##name, \
	.print = print_##name, \
}

static uint16_t ind_x_addr(struct cpu *cpu)
{
	uint8_t ind = cpu_fetch8(cpu) + cpu->regs.x;
	return ((uint16_t)mem_get(&cpu->nes->mem, (ind + 0) & 0xFF) << 0)
	     | ((uint16_t)mem_get(&cpu->nes->mem, (ind + 1) & 0xFF) << 8);
}

static uint16_t ind_y_addr(struct cpu *cpu, int inc_pcross)
{
	uint8_t ind = cpu_fetch8(cpu);
	uint16_t addr = ((uint16_t)mem_get(&cpu->nes->mem, (ind + 0) & 0xFF) << 0)
	              | ((uint16_t)mem_get(&cpu->nes->mem, (ind + 1) & 0xFF) << 8);
	uint16_t ret = addr + cpu->regs.y;
	if (inc_pcross && (ret & 0xFF00) != (addr & 0xFF00))
		cpu->instr_delay++;
	return ret;
}

#define CLSE(name, flag, v) \
CPU_INSTR(name, \
{ \
	CPU_SET_FLAG(cpu, CPU_FLAG_##flag, v); \
	cpu->instr_delay++; \
}, \
{ \
	(void)cpu; \
	snprintf(data, size, #name); \
})

CLSE(clc, C, 0);
CLSE(sec, C, 1);
CLSE(cli, I, 0);
CLSE(sei, I, 1);
CLSE(clv, V, 0);
CLSE(cld, D, 0);
CLSE(sed, D, 1);

#define LDR_IMM(r) \
CPU_INSTR(ld##r##_imm, \
{ \
	uint8_t imm = cpu_fetch8(cpu); \
	cpu->regs.r = imm; \
	CPU_SET_FLAG_Z(cpu, !imm); \
	CPU_SET_FLAG_N(cpu, imm & 0x80); \
	cpu->instr_delay += 1; \
}, \
{ \
	uint8_t imm = cpu_peek8(cpu); \
	snprintf(data, size, "ld" #r " #$%02" PRIx8, imm); \
})

LDR_IMM(a);
LDR_IMM(x);
LDR_IMM(y);

#define STR_IND16(r) \
CPU_INSTR(st##r##_ind16, \
{ \
	uint16_t ind = cpu_fetch16(cpu); \
	mem_set(&cpu->nes->mem, ind, cpu->regs.r); \
	cpu->instr_delay += 3; \
}, \
{ \
	uint16_t ind = cpu_peek16(cpu); \
	snprintf(data, size, "st" #r " $%04" PRIx16, ind); \
})

STR_IND16(a);
STR_IND16(x);
STR_IND16(y);

#define TRR(rs, rd) \
CPU_INSTR(t##rs##rd, \
{ \
	cpu->regs.rd = cpu->regs.rs; \
	if (strcmp(#rs, "x") || strcmp(#rd, "s")) \
	{ \
		CPU_SET_FLAG_Z(cpu, !cpu->regs.rd); \
		CPU_SET_FLAG_N(cpu, cpu->regs.rd & 0x80); \
	} \
	cpu->instr_delay += 1; \
}, \
{ \
	(void)cpu; \
	snprintf(data, size, "t" #rs #rd); \
})

TRR(a, y);
TRR(a, x);
TRR(s, x);
TRR(y, a);
TRR(x, a);
TRR(x, s);

#define STR_IND8(r) \
CPU_INSTR(st##r##_ind8, \
{ \
	uint8_t ind = cpu_fetch8(cpu); \
	mem_set(&cpu->nes->mem, ind, cpu->regs.r); \
	cpu->instr_delay += 2; \
}, \
{ \
	uint8_t ind = cpu_peek8(cpu); \
	snprintf(data, size, "st" #r " $%02" PRIx8, ind); \
})

STR_IND8(a);
STR_IND8(x);
STR_IND8(y);

#define LDR_IND16_R(rd, rs) \
CPU_INSTR(ld##rd##_ind16_##rs, \
{ \
	uint16_t ind = cpu_fetch16(cpu); \
	uint16_t addr = ind + cpu->regs.rs; \
	cpu->regs.rd = mem_get(&cpu->nes->mem, addr); \
	CPU_SET_FLAG_Z(cpu, !cpu->regs.rd); \
	CPU_SET_FLAG_N(cpu, cpu->regs.rd & 0x80); \
	cpu->instr_delay += 3; \
	if ((ind & 0xFF00) != (addr & 0xFF00)) \
		cpu->instr_delay += 1; \
}, \
{ \
	uint16_t ind = cpu_peek16(cpu); \
	snprintf(data, size, "ld" #rd " $%04" PRIx16 ", " #rs, ind); \
})

LDR_IND16_R(a, x);
LDR_IND16_R(a, y);
LDR_IND16_R(x, y);
LDR_IND16_R(y, x);

#define LDR_IND8(r) \
CPU_INSTR(ld##r##_ind8, \
{ \
	uint8_t ind = cpu_fetch8(cpu); \
	cpu->regs.r = mem_get(&cpu->nes->mem, ind); \
	CPU_SET_FLAG_Z(cpu, !cpu->regs.r); \
	CPU_SET_FLAG_N(cpu, cpu->regs.r & 0x80); \
	cpu->instr_delay += 2; \
}, \
{ \
	uint8_t ind = cpu_peek8(cpu); \
	snprintf(data, size, "ld" #r " $%02" PRIx8, ind); \
})

LDR_IND8(a);
LDR_IND8(x);
LDR_IND8(y);

#define LDR_IND16(r) \
CPU_INSTR(ld##r##_ind16, \
{ \
	uint16_t ind = cpu_fetch16(cpu); \
	cpu->regs.r = mem_get(&cpu->nes->mem, ind); \
	CPU_SET_FLAG_Z(cpu, !cpu->regs.r); \
	CPU_SET_FLAG_N(cpu, cpu->regs.r & 0x80); \
	cpu->instr_delay += 3; \
}, \
{ \
	uint16_t ind = cpu_peek16(cpu); \
	snprintf(data, size, "ld" #r " $%04" PRIx16, ind); \
})

LDR_IND16(a);
LDR_IND16(x);
LDR_IND16(y);

#define LDR_IND8_R(rd, rs) \
CPU_INSTR(ld##rd##_ind8_##rs, \
{ \
	uint8_t ind = cpu_fetch8(cpu); \
	cpu->regs.rd = mem_get(&cpu->nes->mem, (ind + cpu->regs.rs) & 0xFF); \
	CPU_SET_FLAG_Z(cpu, !cpu->regs.rd); \
	CPU_SET_FLAG_N(cpu, cpu->regs.rd & 0x80); \
	cpu->instr_delay += 3; \
}, \
{ \
	uint8_t ind = cpu_peek8(cpu); \
	snprintf(data, size, "ld" #rd " $%02" PRIx8 ", " #rs, ind); \
})

LDR_IND8_R(a, x);
LDR_IND8_R(x, y);
LDR_IND8_R(y, x);

#define BRANCH(name, flag, v) \
CPU_INSTR(name, \
{ \
	cpu->instr_delay += 1; \
	int8_t dd = cpu_fetch8(cpu); \
	if (CPU_GET_FLAG(cpu, CPU_FLAG_##flag) == v) \
	{ \
		uint16_t old = cpu->regs.pc - 2; \
		cpu->instr_delay += 1; \
		cpu->regs.pc += dd; \
		if ((old & 0xFF00) != (cpu->regs.pc & 0xFF00)) \
			cpu->instr_delay += 1; \
	} \
}, \
{ \
	int8_t dd = cpu_peek8(cpu); \
	snprintf(data, size, #name " $%" PRId8, dd); \
})

BRANCH(bpl, N, 0);
BRANCH(bmi, N, 1);
BRANCH(bvc, V, 0);
BRANCH(bvs, V, 1);
BRANCH(bcc, C, 0);
BRANCH(bcs, C, 1);
BRANCH(bne, Z, 0);
BRANCH(beq, Z, 1);

CPU_INSTR(pha,
{
	mem_set(&cpu->nes->mem, 0x100 + cpu->regs.s--, cpu->regs.a);
	cpu->instr_delay += 2;
},
{
	(void)cpu;
	snprintf(data, size, "pha");
});

CPU_INSTR(php,
{
	mem_set(&cpu->nes->mem, 0x100 + cpu->regs.s--, 0x30 | cpu->regs.p);
	cpu->instr_delay += 2;
},
{
	(void)cpu;
	snprintf(data, size, "php");
});

CPU_INSTR(pla,
{
	cpu->regs.a = mem_get(&cpu->nes->mem, 0x100 + ++cpu->regs.s);
	CPU_SET_FLAG_Z(cpu, !cpu->regs.a);
	CPU_SET_FLAG_N(cpu, cpu->regs.a & 0x80);
	cpu->instr_delay += 3;
},
{
	(void)cpu;
	snprintf(data, size, "pla");
});

CPU_INSTR(plp,
{
	cpu->regs.p = mem_get(&cpu->nes->mem, 0x100 + ++cpu->regs.s);
	cpu->instr_delay += 3;
},
{
	(void)cpu;
	snprintf(data, size, "plp");
});

CPU_INSTR(jsr,
{
	uint16_t imm = cpu_fetch16(cpu);
	uint16_t pc = cpu->regs.pc - 1;
	mem_set(&cpu->nes->mem, 0x100 + cpu->regs.s--, pc >> 8);
	mem_set(&cpu->nes->mem, 0x100 + cpu->regs.s--, pc >> 0);
	cpu->regs.pc = imm;
	cpu->instr_delay += 5;
},
{
	uint16_t imm = cpu_peek16(cpu);
	snprintf(data, size, "jsr $%" PRIx16, imm);
});

CPU_INSTR(rti,
{
	cpu->regs.p = (cpu->regs.p & 0x30)
	            | (mem_get(&cpu->nes->mem, 0x100 + ++cpu->regs.s) & 0xCF);
	uint16_t lo = mem_get(&cpu->nes->mem, 0x100 + ++cpu->regs.s);
	uint16_t hi = mem_get(&cpu->nes->mem, 0x100 + ++cpu->regs.s);
	cpu->regs.pc = lo | (hi << 8);
	cpu->instr_delay += 5;
},
{
	(void)cpu;
	snprintf(data, size, "rti");
});

CPU_INSTR(rts,
{
	uint16_t lo = mem_get(&cpu->nes->mem, 0x100 + ++cpu->regs.s);
	uint16_t hi = mem_get(&cpu->nes->mem, 0x100 + ++cpu->regs.s);
	cpu->regs.pc = (lo | (hi << 8)) + 1;
	cpu->instr_delay += 5;
},
{
	(void)cpu;
	snprintf(data, size, "rts");
});

CPU_INSTR(jmp_imm,
{
	uint16_t imm = cpu_fetch16(cpu);
	cpu->regs.pc = imm;
	cpu->instr_delay += 2;
},
{
	uint16_t imm = cpu_peek16(cpu);
	snprintf(data, size, "jmp $%04" PRIx16, imm);
});

CPU_INSTR(jmp_ind,
{
	uint16_t ind = cpu_fetch16(cpu);
	uint16_t lo = mem_get(&cpu->nes->mem, ind + 0);
	uint16_t hi = mem_get(&cpu->nes->mem, (ind & 0xFF00) | ((ind + 1) & 0xFF));
	cpu->regs.pc = lo | (hi << 8);
	cpu->instr_delay += 4;
},
{
	uint16_t ind = cpu_peek16(cpu);
	snprintf(data, size, "jmp ($%04" PRIx16 ")", ind);
});

CPU_INSTR(brk,
{
	CPU_SET_FLAG_B(cpu, 1);
	uint16_t pc = cpu->regs.pc + 1;
	mem_set(&cpu->nes->mem, 0x100 + cpu->regs.s--, pc >> 8);
	mem_set(&cpu->nes->mem, 0x100 + cpu->regs.s--, pc >> 0);
	mem_set(&cpu->nes->mem, 0x100 + cpu->regs.s--, cpu->regs.p | 0x30);
	CPU_SET_FLAG_I(cpu, 1);
	uint16_t lo = mem_get(&cpu->nes->mem, 0xFFFE);
	uint16_t hi = mem_get(&cpu->nes->mem, 0xFFFF);
	cpu->regs.pc = lo | (hi << 8);
	cpu->instr_delay += 6;
},
{
	(void)cpu;
	snprintf(data, size, "brk");
});

CPU_INSTR(nop,
{
	(void)cpu;
},
{
	(void)cpu;
	snprintf(data, size, "nop");
});

CPU_INSTR(nop_imm,
{
	uint8_t imm = cpu_fetch8(cpu);
	(void)imm;
},
{
	uint8_t imm = cpu_peek8(cpu);
	snprintf(data, size, "nop #$%02" PRIx8, imm);
	(void)imm;
});

CPU_INSTR(nop_ind8,
{
	uint8_t ind = cpu_fetch8(cpu);
	(void)ind;
},
{
	uint8_t ind = cpu_peek8(cpu);
	snprintf(data, size, "nop $%02" PRIx8, ind);
});

CPU_INSTR(nop_ind16,
{
	uint16_t ind = cpu_fetch16(cpu);
	(void)ind;
},
{
	uint16_t ind = cpu_peek16(cpu);
	snprintf(data, size, "nop $%04" PRIx16, ind);
});

CPU_INSTR(nop_ind8_x,
{
	uint8_t ind = cpu_fetch8(cpu);
	(void)ind;
},
{
	uint8_t ind = cpu_peek8(cpu);
	snprintf(data, size, "nop $%02" PRIx8 ", x", ind);
});

CPU_INSTR(nop_ind16_x,
{
	uint16_t ind = cpu_fetch16(cpu);
	(void)ind;
},
{
	uint16_t ind = cpu_peek16(cpu);
	snprintf(data, size, "nop $%04" PRIx16 ", x", ind);
});

CPU_INSTR(lda_ind_x,
{
	uint16_t ind = ind_x_addr(cpu);
	cpu->regs.a = mem_get(&cpu->nes->mem, ind);
	CPU_SET_FLAG_Z(cpu, !cpu->regs.a);
	CPU_SET_FLAG_N(cpu, cpu->regs.a & 0x80);
	cpu->instr_delay += 5;
},
{
	uint8_t ind = cpu_peek8(cpu);
	snprintf(data, size, "lda ($%02" PRIx8 ", x)", ind);
});

CPU_INSTR(lda_ind_y,
{
	uint16_t ind = ind_y_addr(cpu, 1);
	cpu->regs.a = mem_get(&cpu->nes->mem, ind);
	CPU_SET_FLAG_Z(cpu, !cpu->regs.a);
	CPU_SET_FLAG_N(cpu, cpu->regs.a & 0x80);
	cpu->instr_delay += 4;
},
{
	uint8_t ind = cpu_peek8(cpu);
	snprintf(data, size, "lda ($%02" PRIx8 "), y", ind);
});

#define STA_IND16_R(r) \
CPU_INSTR(sta_ind16_##r, \
{ \
	uint16_t ind = cpu_fetch16(cpu); \
	mem_set(&cpu->nes->mem, ind + cpu->regs.r, cpu->regs.a); \
	cpu->instr_delay += 4; \
}, \
{ \
	uint16_t ind = cpu_peek16(cpu); \
	snprintf(data, size, "sta $%04" PRIx16 ", " #r, ind); \
})

STA_IND16_R(x);
STA_IND16_R(y);

#define STR_IND8_R(rd, rs) \
CPU_INSTR(st##rd##_ind8_##rs, \
{ \
	uint8_t ind = cpu_fetch8(cpu); \
	mem_set(&cpu->nes->mem, (ind + cpu->regs.rs) & 0xFF, cpu->regs.rd); \
	cpu->instr_delay += 3; \
}, \
{ \
	uint8_t ind = cpu_peek8(cpu); \
	snprintf(data, size, "st" #rd " $%02" PRIx8 ", " #rs, ind); \
})

STR_IND8_R(a, x);
STR_IND8_R(x, y);
STR_IND8_R(y, x);

CPU_INSTR(sta_ind_x,
{
	uint16_t ind = ind_x_addr(cpu);
	mem_set(&cpu->nes->mem, ind, cpu->regs.a);
	cpu->instr_delay += 5;
},
{
	uint8_t ind = cpu_peek8(cpu);
	snprintf(data, size, "sta ($%02" PRIx8 ", x)", ind);
});

CPU_INSTR(sta_ind_y,
{
	uint16_t ind = ind_y_addr(cpu, 0);
	mem_set(&cpu->nes->mem, ind, cpu->regs.a);
	cpu->instr_delay += 5;
},
{
	uint8_t ind = cpu_peek8(cpu);
	snprintf(data, size, "sta ($%02" PRIx8 "), y", ind);
});

#define INR(r) \
CPU_INSTR(in##r, \
{ \
	cpu->regs.r++; \
	CPU_SET_FLAG_Z(cpu, !cpu->regs.r); \
	CPU_SET_FLAG_N(cpu, cpu->regs.r & 0x80); \
	cpu->instr_delay += 1; \
}, \
{ \
	(void)cpu; \
	snprintf(data, size, "in" #r); \
})

INR(x);
INR(y);

#define DER(r) \
CPU_INSTR(de##r, \
{ \
	cpu->regs.r--; \
	CPU_SET_FLAG_Z(cpu, !cpu->regs.r); \
	CPU_SET_FLAG_N(cpu, cpu->regs.r & 0x80); \
	cpu->instr_delay += 1; \
}, \
{ \
	(void)cpu; \
	snprintf(data, size, "de" #r); \
})

DER(x);
DER(y);

#define INDE(name, op) \
CPU_INSTR(name##c_ind8, \
{ \
	uint8_t ind = cpu_fetch8(cpu); \
	uint8_t val = mem_get(&cpu->nes->mem, ind) op 1; \
	mem_set(&cpu->nes->mem, ind, val); \
	CPU_SET_FLAG_Z(cpu, !val); \
	CPU_SET_FLAG_N(cpu, val & 0x80); \
	cpu->instr_delay += 4; \
}, \
{ \
	uint8_t ind = cpu_peek8(cpu); \
	snprintf(data, size, #name "c $%02" PRIx8, ind); \
}); \
CPU_INSTR(name##c_ind8_x, \
{ \
	uint8_t ind = cpu_fetch8(cpu); \
	uint8_t val = mem_get(&cpu->nes->mem, (ind + cpu->regs.x) & 0xFF) op 1; \
	mem_set(&cpu->nes->mem, (ind + cpu->regs.x) & 0xFF, val); \
	CPU_SET_FLAG_Z(cpu, !val); \
	CPU_SET_FLAG_N(cpu, val & 0x80); \
	cpu->instr_delay += 5; \
}, \
{ \
	uint8_t ind = cpu_peek8(cpu); \
	snprintf(data, size, #name "c $%02" PRIx8 ", x", ind); \
}); \
CPU_INSTR(name##c_ind16, \
{ \
	uint16_t ind = cpu_fetch16(cpu); \
	uint8_t val = mem_get(&cpu->nes->mem, ind) op 1; \
	mem_set(&cpu->nes->mem, ind, val); \
	CPU_SET_FLAG_Z(cpu, !val); \
	CPU_SET_FLAG_N(cpu, val & 0x80); \
	cpu->instr_delay += 5; \
}, \
{ \
	uint16_t ind = cpu_peek16(cpu); \
	snprintf(data, size, #name "c $%04" PRIx16, ind); \
}); \
CPU_INSTR(name##c_ind16_x, \
{ \
	uint16_t ind = cpu_fetch16(cpu); \
	uint8_t val = mem_get(&cpu->nes->mem, ind + cpu->regs.x) op 1; \
	mem_set(&cpu->nes->mem, ind + cpu->regs.x, val); \
	CPU_SET_FLAG_Z(cpu, !val); \
	CPU_SET_FLAG_N(cpu, val & 0x80); \
	cpu->instr_delay += 6; \
}, \
{ \
	uint16_t ind = cpu_peek16(cpu); \
	snprintf(data, size, #name "c $%04" PRIx16 ", x", ind); \
})

INDE(in, +);
INDE(de, -);

static uint8_t asl(struct cpu *cpu, uint8_t v)
{
	CPU_SET_FLAG_C(cpu, v & 0x80);
	return v << 1;
}

static uint8_t lsr(struct cpu *cpu, uint8_t v)
{
	CPU_SET_FLAG_C(cpu, v & 1);
	return v >> 1;
}

static uint8_t rol(struct cpu *cpu, uint8_t v)
{
	uint8_t c = v >> 7;
	v = (v << 1) | CPU_GET_FLAG_C(cpu);
	CPU_SET_FLAG_C(cpu, c);
	return v;
}

static uint8_t ror(struct cpu *cpu, uint8_t v)
{
	uint8_t c = v & 1;
	v = (v >> 1) | (CPU_GET_FLAG_C(cpu) << 7);
	CPU_SET_FLAG_C(cpu, c);
	return v;
}

#define SH_ROT(op) \
CPU_INSTR(op##_a, \
{ \
	cpu->regs.a = op(cpu, cpu->regs.a); \
	CPU_SET_FLAG_Z(cpu, !cpu->regs.a); \
	CPU_SET_FLAG_N(cpu, cpu->regs.a & 0x80); \
	cpu->instr_delay += 1; \
}, \
{ \
	(void)cpu; \
	snprintf(data, size, #op " a"); \
}); \
CPU_INSTR(op##_ind8, \
{ \
	uint8_t ind = cpu_fetch8(cpu); \
	uint8_t v = op(cpu, mem_get(&cpu->nes->mem, ind)); \
	mem_set(&cpu->nes->mem, ind, v); \
	CPU_SET_FLAG_Z(cpu, !v); \
	CPU_SET_FLAG_N(cpu, v & 0x80); \
	cpu->instr_delay += 4; \
}, \
{ \
	uint8_t ind = cpu_peek8(cpu); \
	snprintf(data, size, #op " $%02" PRIx8, ind); \
}); \
CPU_INSTR(op##_ind8_x, \
{ \
	uint8_t ind = cpu_fetch8(cpu); \
	uint8_t v = op(cpu, mem_get(&cpu->nes->mem, (ind + cpu->regs.x) & 0xFF)); \
	mem_set(&cpu->nes->mem, (ind + cpu->regs.x) & 0xFF, v); \
	CPU_SET_FLAG_Z(cpu, !v); \
	CPU_SET_FLAG_N(cpu, v & 0x80); \
	cpu->instr_delay += 5; \
}, \
{ \
	uint8_t ind = cpu_peek8(cpu); \
	snprintf(data, size, #op " $%02" PRIx8 ", x", ind); \
}); \
CPU_INSTR(op##_ind16, \
{ \
	uint16_t ind = cpu_fetch16(cpu); \
	uint8_t v = op(cpu, mem_get(&cpu->nes->mem, ind)); \
	mem_set(&cpu->nes->mem, ind, v); \
	CPU_SET_FLAG_Z(cpu, !v); \
	CPU_SET_FLAG_N(cpu, v & 0x80); \
	cpu->instr_delay += 5; \
}, \
{ \
	uint8_t ind = cpu_peek16(cpu); \
	snprintf(data, size, #op " $%04" PRIx16, ind); \
}); \
CPU_INSTR(op##_ind16_x, \
{ \
	uint16_t ind = cpu_fetch16(cpu); \
	uint8_t v = op(cpu, mem_get(&cpu->nes->mem, ind + cpu->regs.x)); \
	mem_set(&cpu->nes->mem, ind + cpu->regs.x, v); \
	CPU_SET_FLAG_Z(cpu, !v); \
	CPU_SET_FLAG_N(cpu, v & 0x80); \
	cpu->instr_delay += 6; \
}, \
{ \
	uint8_t ind = cpu_peek16(cpu); \
	snprintf(data, size, #op " $%04" PRIx16 ", x", ind); \
})

SH_ROT(asl);
SH_ROT(lsr);
SH_ROT(rol);
SH_ROT(ror);

#define CPR(name, r) \
CPU_INSTR(name##_imm, \
{ \
	uint8_t imm = cpu_fetch8(cpu); \
	uint8_t v = cpu->regs.r - imm; \
	CPU_SET_FLAG_C(cpu, v <= cpu->regs.r); \
	CPU_SET_FLAG_Z(cpu, !v); \
	CPU_SET_FLAG_N(cpu, v & 0x80); \
	cpu->instr_delay += 1; \
}, \
{ \
	uint8_t imm = cpu_peek8(cpu); \
	snprintf(data, size, #name " #$%02" PRIx8, imm); \
}); \
CPU_INSTR(name##_ind8, \
{ \
	uint8_t ind = cpu_fetch8(cpu); \
	uint8_t v = cpu->regs.r - mem_get(&cpu->nes->mem, ind); \
	CPU_SET_FLAG_C(cpu, v <= cpu->regs.r); \
	CPU_SET_FLAG_Z(cpu, !v); \
	CPU_SET_FLAG_N(cpu, v & 0x80); \
	cpu->instr_delay += 2; \
}, \
{ \
	uint8_t ind = cpu_peek8(cpu); \
	snprintf(data, size, #name " $%02" PRIx8, ind); \
}); \
CPU_INSTR(name##_ind16, \
{ \
	uint16_t ind = cpu_fetch16(cpu); \
	uint8_t v = cpu->regs.r - mem_get(&cpu->nes->mem, ind); \
	CPU_SET_FLAG_C(cpu, v <= cpu->regs.r); \
	CPU_SET_FLAG_Z(cpu, !v); \
	CPU_SET_FLAG_N(cpu, v & 0x80); \
	cpu->instr_delay += 3; \
}, \
{ \
	uint16_t ind = cpu_peek16(cpu); \
	snprintf(data, size, #name " $%04" PRIx16, ind); \
})

CPR(cmp, a);
CPR(cpx, x);
CPR(cpy, y);

#define CMP_IND16_R(r) \
CPU_INSTR(cmp_ind16_##r, \
{ \
	uint16_t ind = cpu_fetch16(cpu); \
	uint16_t addr = ind + cpu->regs.r; \
	uint8_t v = cpu->regs.a - mem_get(&cpu->nes->mem, addr); \
	CPU_SET_FLAG_C(cpu, v <= cpu->regs.a); \
	CPU_SET_FLAG_Z(cpu, !v); \
	CPU_SET_FLAG_N(cpu, v & 0x80); \
	cpu->instr_delay += 3; \
	if ((ind & 0xFF00) != (addr & 0xFF00)) \
		cpu->instr_delay += 1; \
}, \
{ \
	uint16_t ind = cpu_peek16(cpu); \
	snprintf(data, size, "cmp $%04" PRIx16, ind); \
})

CMP_IND16_R(x);
CMP_IND16_R(y);

CPU_INSTR(cmp_ind8_x,
{
	uint8_t ind = cpu_fetch8(cpu);
	uint8_t v = cpu->regs.a - mem_get(&cpu->nes->mem, (ind + cpu->regs.x) & 0xFF);
	CPU_SET_FLAG_C(cpu, v <= cpu->regs.a);
	CPU_SET_FLAG_Z(cpu, !v);
	CPU_SET_FLAG_N(cpu, v & 0x80);
	cpu->instr_delay += 3;
},
{
	uint8_t ind = cpu_peek8(cpu);
	snprintf(data, size, "cmp $%02" PRIx8 ", x", ind);
});

CPU_INSTR(cmp_ind_x,
{
	uint16_t ind = ind_x_addr(cpu);
	uint8_t v = cpu->regs.a - mem_get(&cpu->nes->mem, ind);
	CPU_SET_FLAG_C(cpu, v <= cpu->regs.a);
	CPU_SET_FLAG_Z(cpu, !v);
	CPU_SET_FLAG_N(cpu, v & 0x80);
	cpu->instr_delay += 5;
},
{
	uint8_t ind = cpu_peek8(cpu);
	snprintf(data, size, "cmp ($%02" PRIx8 ", x)", ind);
});

CPU_INSTR(cmp_ind_y,
{
	uint16_t ind = ind_y_addr(cpu, 1);
	uint8_t v = cpu->regs.a - mem_get(&cpu->nes->mem, ind);
	CPU_SET_FLAG_C(cpu, v <= cpu->regs.a);
	CPU_SET_FLAG_Z(cpu, !v);
	CPU_SET_FLAG_N(cpu, v & 0x80);
	cpu->instr_delay += 4;
},
{
	uint8_t ind = cpu_peek8(cpu);
	snprintf(data, size, "cmp ($%02" PRIx8 ", x)", ind);
});

CPU_INSTR(bit_ind8,
{
	uint8_t ind = cpu_fetch8(cpu);
	uint8_t mem = mem_get(&cpu->nes->mem, ind);
	uint8_t v = cpu->regs.a & mem;
	CPU_SET_FLAG_Z(cpu, !v);
	CPU_SET_FLAG_N(cpu, mem & 0x80);
	CPU_SET_FLAG_V(cpu, mem & 0x40);
	cpu->instr_delay += 2;
},
{
	uint8_t ind = cpu_peek8(cpu);
	snprintf(data, size, "bit $%02" PRIx8, ind);
});

CPU_INSTR(bit_ind16,
{
	uint16_t ind = cpu_fetch16(cpu);
	uint8_t mem = mem_get(&cpu->nes->mem, ind);
	uint8_t v = cpu->regs.a & mem;
	CPU_SET_FLAG_Z(cpu, !v);
	CPU_SET_FLAG_N(cpu, mem & 0x80);
	CPU_SET_FLAG_V(cpu, mem & 0x40);
	cpu->instr_delay += 3;
},
{
	uint16_t ind = cpu_peek16(cpu);
	snprintf(data, size, "bit $%04" PRIx16, ind);
});

static void adc(struct cpu *cpu, uint8_t n)
{
	uint8_t c = CPU_GET_FLAG_C(cpu);
	uint8_t op = n + c;
	uint8_t v = cpu->regs.a + op;
	CPU_SET_FLAG_N(cpu, v & 0x80);
	CPU_SET_FLAG_Z(cpu, !v);
	CPU_SET_FLAG_C(cpu, c ? v <= cpu->regs.a : v < cpu->regs.a);
	CPU_SET_FLAG_V(cpu, (~(cpu->regs.a ^ n) & (cpu->regs.a ^ v)) & 0x80);
	cpu->regs.a = v;
}

static void sbc(struct cpu *cpu, uint8_t n)
{
	adc(cpu, ~n);
}

static void and(struct cpu *cpu, uint8_t n)
{
	uint8_t v = cpu->regs.a & n;
	CPU_SET_FLAG_N(cpu, v & 0x80);
	CPU_SET_FLAG_Z(cpu, !v);
	cpu->regs.a = v;
}

static void eor(struct cpu *cpu, uint8_t n)
{
	uint8_t v = cpu->regs.a ^ n;
	CPU_SET_FLAG_N(cpu, v & 0x80);
	CPU_SET_FLAG_Z(cpu, !v);
	cpu->regs.a = v;
}

static void ora(struct cpu *cpu, uint8_t n)
{
	uint8_t v = cpu->regs.a | n;
	CPU_SET_FLAG_N(cpu, v & 0x80);
	CPU_SET_FLAG_Z(cpu, !v);
	cpu->regs.a = v;
}

#define ALU(op) \
CPU_INSTR(op##_imm, \
{ \
	uint8_t imm = cpu_fetch8(cpu); \
	op(cpu, imm); \
	cpu->instr_delay += 1; \
}, \
{ \
	uint8_t imm = cpu_peek8(cpu); \
	snprintf(data, size, #op " #$%02" PRIx8, imm); \
}); \
CPU_INSTR(op##_ind8, \
{ \
	uint8_t ind = cpu_fetch8(cpu); \
	op(cpu, mem_get(&cpu->nes->mem, ind)); \
	cpu->instr_delay += 2; \
}, \
{ \
	uint8_t ind = cpu_peek8(cpu); \
	snprintf(data, size, #op " $%02" PRIx8, ind); \
}); \
CPU_INSTR(op##_ind8_x, \
{ \
	uint8_t ind = cpu_fetch8(cpu); \
	op(cpu, mem_get(&cpu->nes->mem, (ind + cpu->regs.x) & 0xFF)); \
	cpu->instr_delay += 3; \
}, \
{ \
	uint8_t ind = cpu_peek8(cpu); \
	snprintf(data, size, #op " $%02" PRIx8 ", x", ind); \
}); \
CPU_INSTR(op##_ind16, \
{ \
	uint16_t ind = cpu_fetch16(cpu); \
	op(cpu, mem_get(&cpu->nes->mem, ind)); \
	cpu->instr_delay += 3; \
}, \
{ \
	uint16_t ind = cpu_peek16(cpu); \
	snprintf(data, size, #op " $%04" PRIx16, ind); \
}); \
CPU_INSTR(op##_ind16_x, \
{ \
	uint16_t ind = cpu_fetch16(cpu); \
	uint16_t addr = ind + cpu->regs.x; \
	op(cpu, mem_get(&cpu->nes->mem, addr)); \
	cpu->instr_delay += 3; \
	if ((ind & 0xFF00) != (addr & 0xFF00)) \
		cpu->instr_delay += 1; \
}, \
{ \
	uint16_t ind = cpu_peek16(cpu); \
	snprintf(data, size, #op " $%04" PRIx16 ", x", ind); \
}); \
CPU_INSTR(op##_ind16_y, \
{ \
	uint16_t ind = cpu_fetch16(cpu); \
	uint16_t addr = ind + cpu->regs.y; \
	op(cpu, mem_get(&cpu->nes->mem, addr)); \
	cpu->instr_delay += 3; \
	if ((ind & 0xFF00) != (addr & 0xFF00)) \
		cpu->instr_delay += 1; \
}, \
{ \
	uint16_t ind = cpu_peek16(cpu); \
	snprintf(data, size, #op " $%04" PRIx16 ", y", ind); \
}); \
CPU_INSTR(op##_ind_x, \
{ \
	uint16_t ind = ind_x_addr(cpu); \
	op(cpu, mem_get(&cpu->nes->mem, ind)); \
	cpu->instr_delay += 5; \
}, \
{ \
	uint8_t ind = cpu_peek8(cpu); \
	snprintf(data, size, #op " ($%02" PRIx8 ", x)", ind); \
}); \
CPU_INSTR(op##_ind_y, \
{ \
	uint16_t ind = ind_y_addr(cpu, 1); \
	op(cpu, mem_get(&cpu->nes->mem, ind)); \
	cpu->instr_delay += 4; \
}, \
{ \
	uint8_t ind = cpu_peek8(cpu); \
	snprintf(data, size, #op " ($%02" PRIx8 "), y", ind); \
})

ALU(adc);
ALU(sbc);
ALU(and);
ALU(eor);
ALU(ora);

CPU_INSTR(kil,
{
	(void)cpu;
	/* XXX */
	fflush(NULL);
	abort();
},
{
	(void)cpu;
	snprintf(data, size, "kil");
});

CPU_INSTR(sax_ind8,
{
	uint8_t ind = cpu_fetch8(cpu);
	mem_set(&cpu->nes->mem, ind, cpu->regs.a & cpu->regs.x);
	cpu->instr_delay += 2;
},
{
	uint8_t ind = cpu_peek8(cpu);
	snprintf(data, size, "sax $%02" PRIx8, ind);
});

CPU_INSTR(sax_ind8_y,
{
	uint8_t ind = cpu_fetch8(cpu);
	mem_set(&cpu->nes->mem, (ind + cpu->regs.y) & 0xFF, cpu->regs.a & cpu->regs.x);
	cpu->instr_delay += 3;
},
{
	uint8_t ind = cpu_peek8(cpu);
	snprintf(data, size, "sax $%02" PRIx8 ", y", ind);
});

CPU_INSTR(sax_ind16,
{
	uint16_t ind = cpu_fetch16(cpu);
	mem_set(&cpu->nes->mem, ind, cpu->regs.a & cpu->regs.x);
	cpu->instr_delay += 3;
},
{
	uint16_t ind = cpu_peek16(cpu);
	snprintf(data, size, "sax $%04" PRIx16, ind);
});

CPU_INSTR(sax_ind_x,
{
	uint16_t ind = ind_x_addr(cpu);
	mem_set(&cpu->nes->mem, ind, cpu->regs.a & cpu->regs.x);
	cpu->instr_delay += 5;
},
{
	uint8_t ind = cpu_peek8(cpu);
	snprintf(data, size, "sax ($%02" PRIx8 ", x)", ind);
});

CPU_INSTR(lax_imm,
{
	uint8_t imm = cpu_fetch8(cpu);
	cpu->regs.a = imm;
	cpu->regs.x = imm;
	CPU_SET_FLAG_Z(cpu, !cpu->regs.x);
	CPU_SET_FLAG_N(cpu, cpu->regs.x & 0x80);
	cpu->instr_delay += 1;
},
{
	uint8_t imm = cpu_peek8(cpu);
	snprintf(data, size, "lax #$%02" PRIx8, imm);
});

CPU_INSTR(lax_ind8,
{
	uint8_t ind = cpu_fetch8(cpu);
	cpu->regs.a = mem_get(&cpu->nes->mem, ind);
	cpu->regs.x = cpu->regs.a;
	CPU_SET_FLAG_Z(cpu, !cpu->regs.x);
	CPU_SET_FLAG_N(cpu, cpu->regs.x & 0x80);
	cpu->instr_delay += 2;
},
{
	uint8_t ind = cpu_peek8(cpu);
	snprintf(data, size, "lax $%02" PRIx8, ind);
});

CPU_INSTR(lax_ind8_y,
{
	uint8_t ind = cpu_fetch8(cpu);
	cpu->regs.a = mem_get(&cpu->nes->mem, (ind + cpu->regs.y) & 0xFF);
	cpu->regs.x = cpu->regs.a;
	CPU_SET_FLAG_Z(cpu, !cpu->regs.x);
	CPU_SET_FLAG_N(cpu, cpu->regs.x & 0x80);
	cpu->instr_delay += 3;
},
{
	uint8_t ind = cpu_peek8(cpu);
	snprintf(data, size, "lax $%02" PRIx8 ", y", ind);
});

CPU_INSTR(lax_ind16,
{
	uint16_t ind = cpu_fetch16(cpu);
	cpu->regs.a = mem_get(&cpu->nes->mem, ind);
	cpu->regs.x = cpu->regs.a;
	CPU_SET_FLAG_Z(cpu, !cpu->regs.x);
	CPU_SET_FLAG_N(cpu, cpu->regs.x & 0x80);
	cpu->instr_delay += 3;
},
{
	uint16_t ind = cpu_peek16(cpu);
	snprintf(data, size, "lax $%04" PRIx16, ind);
});

CPU_INSTR(lax_ind16_y,
{
	uint16_t ind = cpu_fetch16(cpu);
	cpu->regs.a = mem_get(&cpu->nes->mem, ind + cpu->regs.y);
	cpu->regs.x = cpu->regs.a;
	CPU_SET_FLAG_Z(cpu, !cpu->regs.x);
	CPU_SET_FLAG_N(cpu, cpu->regs.x & 0x80);
	cpu->instr_delay += 3;
},
{
	uint16_t ind = cpu_peek16(cpu);
	snprintf(data, size, "lax $%04" PRIx16 ", y", ind);
});

CPU_INSTR(lax_ind_x,
{
	uint16_t ind = ind_x_addr(cpu);
	cpu->regs.a = mem_get(&cpu->nes->mem, ind);
	cpu->regs.x = cpu->regs.a;
	CPU_SET_FLAG_Z(cpu, !cpu->regs.x);
	CPU_SET_FLAG_N(cpu, cpu->regs.x & 0x80);
	cpu->instr_delay += 5;
},
{
	uint8_t ind = cpu_peek8(cpu);
	snprintf(data, size, "lax ($%02" PRIx8 ", x)", ind);
});

CPU_INSTR(lax_ind_y,
{
	uint16_t ind = ind_y_addr(cpu, 1);
	cpu->regs.a = mem_get(&cpu->nes->mem, ind);
	cpu->regs.x = cpu->regs.a;
	CPU_SET_FLAG_Z(cpu, !cpu->regs.x);
	CPU_SET_FLAG_N(cpu, cpu->regs.x & 0x80);
	cpu->instr_delay += 4;
},
{
	uint8_t ind = cpu_peek8(cpu);
	snprintf(data, size, "lax ($%02" PRIx8 "), y", ind);
});

CPU_INSTR(axs_imm,
{
	uint8_t imm = cpu_fetch8(cpu);
	uint8_t n = cpu->regs.a & cpu->regs.x;
	uint8_t v = n - imm;
	CPU_SET_FLAG_C(cpu, v <= n);
	CPU_SET_FLAG_Z(cpu, !v);
	CPU_SET_FLAG_N(cpu, v & 0x80);
	cpu->regs.x = v;
	cpu->instr_delay += 1;
},
{
	uint8_t imm = cpu_peek8(cpu);
	snprintf(data, size, "axs #$%02" PRIx8, imm);
});

static uint8_t slo(struct cpu *cpu, uint8_t v)
{
	uint8_t n = asl(cpu, v);
	ora(cpu, n);
	return n;
}

static uint8_t rla(struct cpu *cpu, uint8_t v)
{
	uint8_t n = rol(cpu, v);
	and(cpu, n);
	return n;
}

static uint8_t sre(struct cpu *cpu, uint8_t v)
{
	uint8_t n = lsr(cpu, v);
	eor(cpu, n);
	return n;
}

static uint8_t rra(struct cpu *cpu, uint8_t v)
{
	uint8_t n = ror(cpu, v);
	adc(cpu, n);
	return n;
}

static uint8_t dcp(struct cpu *cpu, uint8_t v)
{
	uint8_t n = v - 1;
	v = cpu->regs.a - n;
	CPU_SET_FLAG_C(cpu, v <= cpu->regs.a);
	CPU_SET_FLAG_Z(cpu, !v);
	CPU_SET_FLAG_N(cpu, v & 0x80);
	return n;
}

static uint8_t isc(struct cpu *cpu, uint8_t v)
{
	uint8_t n = v + 1;
	sbc(cpu, n);
	return n;
}

#define ALU_EXT(name) \
CPU_INSTR(name##_ind8, \
{ \
	uint8_t ind = cpu_fetch8(cpu); \
	mem_set(&cpu->nes->mem, ind, name(cpu, mem_get(&cpu->nes->mem, ind))); \
	cpu->instr_delay += 4; \
}, \
{ \
	uint8_t ind = cpu_peek8(cpu); \
	snprintf(data, size, #name " $%02" PRIx8, ind); \
}); \
CPU_INSTR(name##_ind8_x, \
{ \
	uint8_t ind = cpu_fetch8(cpu); \
	mem_set(&cpu->nes->mem, (ind + cpu->regs.x) & 0xFF, \
	        name(cpu, mem_get(&cpu->nes->mem, (ind + cpu->regs.x) & 0xFF))); \
	cpu->instr_delay += 5; \
}, \
{ \
	uint8_t ind = cpu_peek8(cpu); \
	snprintf(data, size, #name " $%02" PRIx8 ", x", ind); \
}); \
CPU_INSTR(name##_ind16, \
{ \
	uint16_t ind = cpu_fetch16(cpu); \
	mem_set(&cpu->nes->mem, ind, name(cpu, mem_get(&cpu->nes->mem, ind))); \
	cpu->instr_delay += 5; \
}, \
{ \
	uint16_t ind = cpu_peek16(cpu); \
	snprintf(data, size, #name " $%04" PRIx16, ind); \
}); \
CPU_INSTR(name##_ind16_x, \
{ \
	uint16_t ind = cpu_fetch16(cpu); \
	mem_set(&cpu->nes->mem, ind + cpu->regs.x, \
	        name(cpu, mem_get(&cpu->nes->mem, ind + cpu->regs.x))); \
	cpu->instr_delay += 6; \
}, \
{ \
	uint16_t ind = cpu_peek16(cpu); \
	snprintf(data, size, #name " $%04" PRIx16 ", x", ind); \
}); \
CPU_INSTR(name##_ind16_y, \
{ \
	uint16_t ind = cpu_fetch16(cpu); \
	mem_set(&cpu->nes->mem, ind + cpu->regs.y, name(cpu, mem_get(&cpu->nes->mem, ind + cpu->regs.y))); \
	cpu->instr_delay += 6; \
}, \
{ \
	uint16_t ind = cpu_peek16(cpu); \
	snprintf(data, size, #name " $%04" PRIx16 ", y", ind); \
}); \
CPU_INSTR(name##_ind_x, \
{ \
	uint16_t ind = ind_x_addr(cpu); \
	mem_set(&cpu->nes->mem, ind, name(cpu, mem_get(&cpu->nes->mem, ind))); \
	cpu->instr_delay += 7; \
}, \
{ \
	uint8_t ind = cpu_peek8(cpu); \
	snprintf(data, size, #name " ($%02" PRIx8 ", x)", ind); \
}); \
CPU_INSTR(name##_ind_y, \
{ \
	uint16_t ind = ind_y_addr(cpu, 0); \
	mem_set(&cpu->nes->mem, ind, name(cpu, mem_get(&cpu->nes->mem, ind))); \
	cpu->instr_delay += 7; \
}, \
{ \
	uint8_t ind = cpu_peek8(cpu); \
	snprintf(data, size, #name " ($%02" PRIx8 "), y", ind); \
})

ALU_EXT(slo);
ALU_EXT(rla);
ALU_EXT(sre);
ALU_EXT(rra);
ALU_EXT(dcp);
ALU_EXT(isc);

CPU_INSTR(anc_imm,
{
	uint8_t imm = cpu_fetch8(cpu);
	and(cpu, imm);
	CPU_SET_FLAG_C(cpu, cpu->regs.a & 0x80);
	cpu->instr_delay += 1;
},
{
	uint8_t imm = cpu_peek8(cpu);
	printf(data, size, "anc #$%02" PRIx8, imm);
});

CPU_INSTR(alr_imm,
{
	uint8_t imm = cpu_fetch8(cpu);
	cpu->regs.a = lsr(cpu, cpu->regs.a & imm);
	CPU_SET_FLAG_N(cpu, cpu->regs.a & 0x80);
	CPU_SET_FLAG_Z(cpu, !cpu->regs.a);
	cpu->instr_delay += 1;
},
{
	uint8_t imm = cpu_peek8(cpu);
	snprintf(data, size, "alr #$%02" PRIx8, imm);
});

CPU_INSTR(arr_imm,
{
	uint8_t imm = cpu_fetch8(cpu);
	and(cpu, imm);
	uint8_t v = cpu->regs.a + imm;
	CPU_SET_FLAG_V(cpu, (~(cpu->regs.a ^ imm) & (cpu->regs.a ^ v)) & 0x80);
	cpu->regs.a = ror(cpu, cpu->regs.a);
	cpu->instr_delay += 1;
},
{
	uint8_t imm = cpu_peek8(cpu);
	snprintf(data, size, "arr #$%02" PRIx8, imm);
});

CPU_INSTR(xaa_imm,
{
	uint8_t imm = cpu_fetch8(cpu);
	cpu->regs.a = cpu->regs.x;
	and(cpu, imm);
	cpu->instr_delay += 1;
},
{
	uint8_t imm = cpu_peek8(cpu);
	snprintf(data, size, "xaa #$%02" PRIx8, imm);
});

CPU_INSTR(las_ind16_y,
{
	uint16_t ind = cpu_fetch16(cpu);
	uint16_t addr = ind + cpu->regs.y;
	cpu->regs.s = cpu->regs.s & mem_get(&cpu->nes->mem, addr);
	cpu->regs.a = cpu->regs.s;
	cpu->regs.x = cpu->regs.s;
	cpu->instr_delay += 3;
	if ((ind & 0xFF00) != (addr & 0xFF00))
		cpu->instr_delay += 1;
},
{
	uint16_t ind = cpu_peek16(cpu);
	snprintf(data, size, "las $%04" PRIx16 ", y", ind);
});

CPU_INSTR(shy_ind16_x,
{
	uint16_t ind = cpu_fetch16(cpu);
	mem_set(&cpu->nes->mem, ind + cpu->regs.x, cpu->regs.y & ((ind >> 8) + 1));
	cpu->instr_delay += 4;
},
{
	uint16_t ind = cpu_peek16(cpu);
	snprintf(data, size, "shy $%04" PRIx16 ", x", ind);
});

CPU_INSTR(shx_ind16_y,
{
	uint16_t ind = cpu_fetch16(cpu);
	mem_set(&cpu->nes->mem, ind + cpu->regs.y, cpu->regs.x & ((ind >> 8) + 1));
	cpu->instr_delay += 4;
},
{
	uint16_t ind = cpu_peek16(cpu);
	snprintf(data, size, "shx $%04" PRIx16 ", y", ind);
});

CPU_INSTR(ahx_ind_y,
{
	uint16_t addr = ind_y_addr(cpu, 0);
	mem_set(&cpu->nes->mem, addr, cpu->regs.a & cpu->regs.x & ((addr >> 8) + 1));
	cpu->instr_delay += 5;
},
{
	uint8_t ind = cpu_peek8(cpu);
	snprintf(data, size, "ahx ($%02" PRIx8 "), y", ind);
});

CPU_INSTR(ahx_ind16_y,
{
	uint16_t addr = cpu_fetch16(cpu) + cpu->regs.y;
	mem_set(&cpu->nes->mem, addr, cpu->regs.a & cpu->regs.x & ((addr + 1) >> 8));
	cpu->instr_delay += 4;
},
{
	uint16_t ind = cpu_peek16(cpu);
	snprintf(data, size, "ahx $%04" PRIx16 ", y", ind);
});

CPU_INSTR(tas_ind16_y,
{
	uint16_t ind = cpu_fetch16(cpu);
	cpu->regs.s = cpu->regs.a & cpu->regs.x;
	mem_set(&cpu->nes->mem, ind + cpu->regs.y, cpu->regs.s & ((ind >> 8) + 1));
	cpu->instr_delay += 4;
},
{
	uint16_t ind = cpu_peek16(cpu);
	snprintf(data, size, "tas $%04" PRIx16 ", y", ind);
});

CPU_INSTR(irq,
{
	CPU_SET_FLAG_B(cpu, 0);
	uint16_t pc = cpu->regs.pc;
	mem_set(&cpu->nes->mem, 0x100 + cpu->regs.s--, pc >> 8);
	mem_set(&cpu->nes->mem, 0x100 + cpu->regs.s--, pc >> 0);
	mem_set(&cpu->nes->mem, 0x100 + cpu->regs.s--, cpu->regs.p | 0x20);
	CPU_SET_FLAG_I(cpu, 1);
	uint16_t lo = mem_get(&cpu->nes->mem, 0xFFFE);
	uint16_t hi = mem_get(&cpu->nes->mem, 0xFFFF);
	cpu->regs.pc = lo | (hi << 8);
	cpu->instr_delay += 6;
},
{
	(void)cpu;
	snprintf(data, size, "irq");
});

CPU_INSTR(nmi,
{
	CPU_SET_FLAG_B(cpu, 0);
	uint16_t pc = cpu->regs.pc;
	mem_set(&cpu->nes->mem, 0x100 + cpu->regs.s--, pc >> 8);
	mem_set(&cpu->nes->mem, 0x100 + cpu->regs.s--, pc >> 0);
	mem_set(&cpu->nes->mem, 0x100 + cpu->regs.s--, cpu->regs.p | 0x20);
	CPU_SET_FLAG_I(cpu, 1);
	uint16_t lo = mem_get(&cpu->nes->mem, 0xFFFA);
	uint16_t hi = mem_get(&cpu->nes->mem, 0xFFFB);
	cpu->regs.pc = lo | (hi << 8);
	cpu->instr_delay += 6;
},
{
	(void)cpu;
	snprintf(data, size, "nmi");
});

CPU_INSTR(reset,
{
	CPU_SET_FLAG_B(cpu, 1);
	cpu->regs.s -= 3;
	CPU_SET_FLAG_I(cpu, 1);
	uint16_t lo = mem_get(&cpu->nes->mem, 0xFFFC);
	uint16_t hi = mem_get(&cpu->nes->mem, 0xFFFD);
	cpu->regs.pc = lo | (hi << 8);
	cpu->instr_delay += 6;
},
{
	(void)cpu;
	snprintf(data, size, "reset");
});

const struct cpu_instr *cpu_instr[256] =
{
	/* 0x00 */ &brk, &ora_ind_x, &kil, &slo_ind_x,
	/* 0x04 */ &nop_ind8, &ora_ind8, &asl_ind8, &slo_ind8,
	/* 0x08 */ &php, &ora_imm, &asl_a, &anc_imm,
	/* 0x0c */ &nop_ind16, &ora_ind16, &asl_ind16, &slo_ind16,
	/* 0x10 */ &bpl, &ora_ind_y, &kil, &slo_ind_y,
	/* 0x14 */ &nop_ind8_x, &ora_ind8_x, &asl_ind8_x, &slo_ind8_x,
	/* 0x18 */ &clc, &ora_ind16_y, &nop, &slo_ind16_y,
	/* 0x1C */ &nop_ind16_x, &ora_ind16_x, &asl_ind16_x, &slo_ind16_x,
	/* 0x20 */ &jsr, &and_ind_x, &kil, &rla_ind_x,
	/* 0x24 */ &bit_ind8, &and_ind8, &rol_ind8, &rla_ind8,
	/* 0x28 */ &plp, &and_imm, &rol_a, &anc_imm,
	/* 0x2C */ &bit_ind16, &and_ind16, &rol_ind16, &rla_ind16,
	/* 0x30 */ &bmi, &and_ind_y, &kil, &rla_ind_y,
	/* 0x34 */ &nop_ind8_x, &and_ind8_x, &rol_ind8_x, &rla_ind8_x,
	/* 0x38 */ &sec, &and_ind16_y, &nop, &rla_ind16_y,
	/* 0x3C */ &nop_ind16_x, &and_ind16_x, &rol_ind16_x, &rla_ind16_x,
	/* 0x40 */ &rti, &eor_ind_x, &kil, &sre_ind_x,
	/* 0x44 */ &nop_ind8, &eor_ind8, &lsr_ind8, &sre_ind8,
	/* 0x48 */ &pha, &eor_imm, &lsr_a, &alr_imm,
	/* 0x4C */ &jmp_imm, &eor_ind16, &lsr_ind16, &sre_ind16,
	/* 0x50 */ &bvc, &eor_ind_y, &kil, &sre_ind_y,
	/* 0x54 */ &nop_ind8_x, &eor_ind8_x, &lsr_ind8_x, &sre_ind8_x,
	/* 0x58 */ &cli, &eor_ind16_y, &nop, &sre_ind16_y,
	/* 0x5C */ &nop_ind16_x, &eor_ind16_x, &lsr_ind16_x, &sre_ind16_x,
	/* 0x60 */ &rts, &adc_ind_x, &kil, &rra_ind_x,
	/* 0x64 */ &nop_ind8, &adc_ind8, &ror_ind8, &rra_ind8,
	/* 0x68 */ &pla, &adc_imm, &ror_a, &arr_imm,
	/* 0x6C */ &jmp_ind, &adc_ind16, &ror_ind16, &rra_ind16,
	/* 0x70 */ &bvs, &adc_ind_y, &kil, &rra_ind_y,
	/* 0x74 */ &nop_ind8_x, &adc_ind8_x, &ror_ind8_x, &rra_ind8_x,
	/* 0x78 */ &sei, &adc_ind16_y, &nop, &rra_ind16_y,
	/* 0x7C */ &nop_ind16_x, &adc_ind16_x, &ror_ind16_x, &rra_ind16_x,
	/* 0x80 */ &nop_imm, &sta_ind_x, &nop_imm, &sax_ind_x,
	/* 0x84 */ &sty_ind8, &sta_ind8, &stx_ind8, &sax_ind8,
	/* 0x88 */ &dey, &nop_imm, &txa, &xaa_imm,
	/* 0x8C */ &sty_ind16, &sta_ind16, &stx_ind16, &sax_ind16,
	/* 0x90 */ &bcc, &sta_ind_y, &kil, &ahx_ind_y,
	/* 0x94 */ &sty_ind8_x, &sta_ind8_x, &stx_ind8_y, &sax_ind8_y,
	/* 0x98 */ &tya, &sta_ind16_y, &txs, &tas_ind16_y,
	/* 0x9C */ &shy_ind16_x, &sta_ind16_x, &shx_ind16_y, &ahx_ind16_y,
	/* 0xA0 */ &ldy_imm, &lda_ind_x, &ldx_imm, &lax_ind_x,
	/* 0xA4 */ &ldy_ind8, &lda_ind8, &ldx_ind8, &lax_ind8,
	/* 0xA8 */ &tay, &lda_imm, &tax, &lax_imm,
	/* 0xAC */ &ldy_ind16, &lda_ind16, &ldx_ind16, &lax_ind16,
	/* 0xB0 */ &bcs, &lda_ind_y, &kil, &lax_ind_y,
	/* 0xB4 */ &ldy_ind8_x, &lda_ind8_x, &ldx_ind8_y, &lax_ind8_y,
	/* 0xB8 */ &clv, &lda_ind16_y, &tsx, &las_ind16_y,
	/* 0xBC */ &ldy_ind16_x, &lda_ind16_x, &ldx_ind16_y, &lax_ind16_y,
	/* 0xC0 */ &cpy_imm, &cmp_ind_x, &nop_imm, &dcp_ind_x,
	/* 0xC4 */ &cpy_ind8, &cmp_ind8, &dec_ind8, &dcp_ind8,
	/* 0xC8 */ &iny, &cmp_imm, &dex, &axs_imm,
	/* 0xCC */ &cpy_ind16, &cmp_ind16, &dec_ind16, &dcp_ind16,
	/* 0xD0 */ &bne, &cmp_ind_y, &kil, &dcp_ind_y,
	/* 0xD4 */ &nop_ind8_x, &cmp_ind8_x, &dec_ind8_x, &dcp_ind8_x,
	/* 0xD8 */ &cld, &cmp_ind16_y, &nop, &dcp_ind16_y,
	/* 0xDC */ &nop_ind16_x, &cmp_ind16_x, &dec_ind16_x, &dcp_ind16_x,
	/* 0xE0 */ &cpx_imm, &sbc_ind_x, &nop_imm, &isc_ind_x,
	/* 0xE4 */ &cpx_ind8, &sbc_ind8, &inc_ind8, &isc_ind8,
	/* 0xE8 */ &inx, &sbc_imm, &nop, &sbc_imm,
	/* 0xEC */ &cpx_ind16, &sbc_ind16, &inc_ind16, &isc_ind16,
	/* 0xF0 */ &beq, &sbc_ind_y, &kil, &isc_ind_y,
	/* 0xF4 */ &nop_ind8_x, &sbc_ind8_x, &inc_ind8_x, &isc_ind8_x,
	/* 0xF8 */ &sed, &sbc_ind16_y, &nop, &isc_ind16_y,
	/* 0xFC */ &nop_ind16_x, &sbc_ind16_x, &inc_ind16_x, &isc_ind16_x,
};

const struct cpu_instr *special_instr[3] =
{
	&irq, &nmi, &reset,
};
