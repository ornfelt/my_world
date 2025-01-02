#include "../cpu.h"
#include "../mem.h"
#include "instr.h"

#include <inttypes.h>
#include <stdio.h>

#define CPU_INSTR(name, exec_fn, print_fn) \
static bool exec_##name(struct cpu *cpu) \
exec_fn \
static void print_##name(struct cpu *cpu, char *data, size_t size) \
{ \
	(void)cpu; \
	print_fn \
} \
static const struct cpu_instr name = \
{ \
	.exec = exec_##name, \
	.print = print_##name, \
}

CPU_INSTR(nop,
{
	cpu->regs.pc++;
	return true;
},
{
	snprintf(data, size, "nop");
});

CPU_INSTR(stop,
{
	switch (cpu->instr_cycle)
	{
		case 0:
		{
			mem_set_reg(cpu->mem, MEM_REG_DIV, 0);
			uint8_t key1 = mem_get_reg(cpu->mem, MEM_REG_KEY1);
			if (key1 & 1)
			{
				cpu->mem->doublespeed = !cpu->mem->doublespeed;
				mem_set_reg(cpu->mem, MEM_REG_KEY1, key1 & 0xFE);
			}
			else
			{
				cpu->state = CPU_STOP;
			}
			break;
		}
	}
	cpu->regs.pc += 2;
	return true;
},
{
	snprintf(data, size, "stop");
});

CPU_INSTR(halt,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			cpu->state = CPU_HALT;
			break;
	}
	cpu->regs.pc++;
	return true;
},
{
	snprintf(data, size, "halt");
});

CPU_INSTR(ei,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			cpu->ei = 2;
			break;
	}
	cpu->regs.pc++;
	return true;
},
{
	snprintf(data, size, "ei");
});

CPU_INSTR(di,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			cpu->ime = false;
			break;
	}
	cpu->regs.pc++;
	return true;
},
{
	snprintf(data, size, "di");
});

#define INCR(r) \
CPU_INSTR(inc_##r, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
		{ \
			uint8_t v = cpu->regs.r; \
			cpu->regs.r++; \
			CPU_SET_FLAG_Z(cpu, !cpu->regs.r); \
			CPU_SET_FLAG_N(cpu, 0); \
			CPU_SET_FLAG_H(cpu, (v & 0xF) == 0xF); \
			break; \
		} \
	} \
	cpu->regs.pc += 1; \
	return true; \
}, \
{ \
	snprintf(data, size, "inc " #r); \
})

INCR(a);
INCR(b);
INCR(c);
INCR(d);
INCR(e);
INCR(h);
INCR(l);

#define DECR(r) \
CPU_INSTR(dec_##r, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
		{ \
			uint8_t v = cpu->regs.r; \
			cpu->regs.r--; \
			CPU_SET_FLAG_Z(cpu, !cpu->regs.r); \
			CPU_SET_FLAG_N(cpu, 1); \
			CPU_SET_FLAG_H(cpu, !(v & 0xF)); \
			break; \
		} \
	} \
	cpu->regs.pc += 1; \
	return true; \
}, \
{ \
	snprintf(data, size, "dec " #r); \
})

DECR(a);
DECR(b);
DECR(c);
DECR(d);
DECR(e);
DECR(h);
DECR(l);

static void alu_add(struct cpu *cpu, uint8_t add)
{
	uint8_t v = cpu->regs.a;
	cpu->regs.a += add;
	CPU_SET_FLAG_Z(cpu, !cpu->regs.a);
	CPU_SET_FLAG_N(cpu, 0);
	CPU_SET_FLAG_H(cpu, ((v & 0xF) + (add & 0xF)) & 0x10);
	CPU_SET_FLAG_C(cpu, v > cpu->regs.a);
}

static void alu_adc(struct cpu *cpu, uint8_t add)
{
	uint8_t v = cpu->regs.a;
	uint8_t c = CPU_GET_FLAG_C(cpu);
	cpu->regs.a += add + c;
	CPU_SET_FLAG_Z(cpu, !cpu->regs.a);
	CPU_SET_FLAG_N(cpu, 0);
	CPU_SET_FLAG_H(cpu, ((v & 0xF) + (add & 0xF) + c) & 0x10);
	CPU_SET_FLAG_C(cpu, c ? (v >= cpu->regs.a) : (v > cpu->regs.a));
}

static void alu_sub(struct cpu *cpu, uint8_t sub)
{
	uint8_t v = cpu->regs.a;
	cpu->regs.a -= sub;
	CPU_SET_FLAG_Z(cpu, !cpu->regs.a);
	CPU_SET_FLAG_N(cpu, 1);
	CPU_SET_FLAG_H(cpu, (v & 0xF) < (sub & 0xF));
	CPU_SET_FLAG_C(cpu, cpu->regs.a > v);
}

static void alu_sbc(struct cpu *cpu, uint8_t sub)
{
	uint8_t v = cpu->regs.a;
	uint8_t c = CPU_GET_FLAG_C(cpu);
	cpu->regs.a -= sub + c;
	CPU_SET_FLAG_Z(cpu, !cpu->regs.a);
	CPU_SET_FLAG_N(cpu, 1);
	CPU_SET_FLAG_H(cpu, (v & 0xF) < (sub & 0xF) + c);
	CPU_SET_FLAG_C(cpu, c ? (cpu->regs.a >= v) : (cpu->regs.a > v));
}

static void alu_and(struct cpu *cpu, uint8_t and)
{
	cpu->regs.a &= and;
	CPU_SET_FLAG_Z(cpu, !cpu->regs.a);
	CPU_SET_FLAG_N(cpu, 0);
	CPU_SET_FLAG_H(cpu, 1);
	CPU_SET_FLAG_C(cpu, 0);
}

static void alu_xor(struct cpu *cpu, uint8_t xor)
{
	cpu->regs.a ^= xor;
	CPU_SET_FLAG_Z(cpu, !cpu->regs.a);
	CPU_SET_FLAG_N(cpu, 0);
	CPU_SET_FLAG_H(cpu, 0);
	CPU_SET_FLAG_C(cpu, 0);
}

static void alu_or(struct cpu *cpu, uint8_t or)
{
	cpu->regs.a |= or;
	CPU_SET_FLAG_Z(cpu, !cpu->regs.a);
	CPU_SET_FLAG_N(cpu, 0);
	CPU_SET_FLAG_H(cpu, 0);
	CPU_SET_FLAG_C(cpu, 0);
}

static void alu_cp(struct cpu *cpu, uint8_t cp)
{
	CPU_SET_FLAG_Z(cpu, cpu->regs.a == cp);
	CPU_SET_FLAG_N(cpu, 1);
	CPU_SET_FLAG_H(cpu, (cpu->regs.a & 0xF) < (cp & 0xF));
	CPU_SET_FLAG_C(cpu, cpu->regs.a < cp);
}

#define ALU_R(name, r) \
CPU_INSTR(name##_a_##r, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
		{ \
			alu_##name(cpu, cpu->regs.r); \
			break; \
		} \
	} \
	cpu->regs.pc += 1; \
	return true; \
}, \
{ \
	snprintf(data, size, #name " a, " #r); \
})

#define ALU(name) \
ALU_R(name, a); \
ALU_R(name, b); \
ALU_R(name, c); \
ALU_R(name, d); \
ALU_R(name, e); \
ALU_R(name, h); \
ALU_R(name, l); \
CPU_INSTR(name##_a_hl, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
		{ \
			alu_##name(cpu, mem_get(cpu->mem, cpu->regs.hl)); \
			break; \
		} \
	} \
	cpu->regs.pc += 1; \
	return true; \
}, \
{ \
	snprintf(data, size, #name " a, (hl)"); \
}); \
CPU_INSTR(name##_a_n, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
		{ \
			alu_##name(cpu, mem_get(cpu->mem, cpu->regs.pc + 1)); \
			break; \
		} \
	} \
	cpu->regs.pc += 2; \
	return true; \
}, \
{ \
	snprintf(data, size, #name " a, $%02" PRIX8, mem_get(cpu->mem, cpu->regs.pc + 1)); \
})

ALU(add);
ALU(adc);
ALU(sub);
ALU(sbc);
ALU(and);
ALU(xor);
ALU(or);
ALU(cp);

CPU_INSTR(inc_rhl,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			cpu->instr_tmp.u8[0] = mem_get(cpu->mem, cpu->regs.hl);
			return false;
		case 2:
		{
			uint8_t v = cpu->instr_tmp.u8[0] + 1;
			mem_set(cpu->mem, cpu->regs.hl, v);
			CPU_SET_FLAG_Z(cpu, !v);
			CPU_SET_FLAG_N(cpu, 0);
			CPU_SET_FLAG_H(cpu, !(v & 0xF));
			break;
		}
	}
	cpu->regs.pc += 1;
	return true;
},
{
	snprintf(data, size, "inc (hl)");
});

CPU_INSTR(dec_rhl,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			cpu->instr_tmp.u8[0] = mem_get(cpu->mem, cpu->regs.hl);
			return false;
		case 2:
		{
			uint8_t v = cpu->instr_tmp.u8[0] - 1;
			mem_set(cpu->mem, cpu->regs.hl, v);
			CPU_SET_FLAG_Z(cpu, !v);
			CPU_SET_FLAG_N(cpu, 1);
			CPU_SET_FLAG_H(cpu, (v & 0xF) == 0xF);
			break;
		}
	}
	cpu->regs.pc += 1;
	return true;
},
{
	snprintf(data, size, "dec (hl)");
});

CPU_INSTR(daa,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			if (CPU_GET_FLAG_N(cpu))
			{
				if (CPU_GET_FLAG_H(cpu))
					cpu->regs.a -= 0x06;
				if (CPU_GET_FLAG_C(cpu))
					cpu->regs.a -= 0x60;
			}
			else
			{
				uint16_t v = cpu->regs.a;
				if (CPU_GET_FLAG_H(cpu) || (v & 0x0F) > 0x09)
					v += 0x06;
				if (CPU_GET_FLAG_C(cpu) || v > 0x9F)
					v += 0x60;
				if (v & 0x100)
					CPU_SET_FLAG_C(cpu, 1);
				cpu->regs.a = v;
			}
			CPU_SET_FLAG_Z(cpu, !cpu->regs.a);
			CPU_SET_FLAG_H(cpu, 0);
			break;
	}
	cpu->regs.pc += 1;
	return true;
},
{
	snprintf(data, size, "daa");
});

CPU_INSTR(cpl,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			cpu->regs.a ^= 0xFF;
			CPU_SET_FLAG_N(cpu, 1);
			CPU_SET_FLAG_H(cpu, 1);
			break;
	}
	cpu->regs.pc += 1;
	return true;
},
{
	snprintf(data, size, "cpl");
});

CPU_INSTR(scf,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			CPU_SET_FLAG_N(cpu, 0);
			CPU_SET_FLAG_H(cpu, 0);
			CPU_SET_FLAG_C(cpu, 1);
			break;
	}
	cpu->regs.pc += 1;
	return true;
},
{
	snprintf(data, size, "scf");
});

CPU_INSTR(ccf,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			CPU_SET_FLAG_N(cpu, 0);
			CPU_SET_FLAG_H(cpu, 0);
			CPU_SET_FLAG_C(cpu, !CPU_GET_FLAG_C(cpu));
			break;
	}
	cpu->regs.pc += 1;
	return true;
},
{
	snprintf(data, size, "ccf");
});

#define INC_RR(rr) \
CPU_INSTR(inc_##rr, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			cpu->regs.rr++; \
			break; \
	} \
	cpu->regs.pc += 1; \
	return true; \
}, \
{ \
	snprintf(data, size, "inc " #rr); \
})

INC_RR(bc);
INC_RR(de);
INC_RR(hl);
INC_RR(sp);

#define DEC_RR(rr) \
CPU_INSTR(dec_##rr, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			cpu->regs.rr--; \
			break; \
	} \
	cpu->regs.pc += 1; \
	return true; \
}, \
{ \
	snprintf(data, size, "dec " #rr); \
})

DEC_RR(bc);
DEC_RR(de);
DEC_RR(hl);
DEC_RR(sp);

#define ADD_HL_RR(rr) \
CPU_INSTR(add_hl_##rr, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
		{ \
			uint16_t v = cpu->regs.hl; \
			cpu->regs.hl += cpu->regs.rr; \
			CPU_SET_FLAG_N(cpu, 0); \
			CPU_SET_FLAG_H(cpu, (cpu->regs.hl & 0xFFF) < (v & 0xFFF)); \
			CPU_SET_FLAG_C(cpu, cpu->regs.hl < v); \
			break; \
		} \
	} \
	cpu->regs.pc += 1; \
	return true; \
}, \
{ \
	snprintf(data, size, "add hl, " #rr); \
})

ADD_HL_RR(bc);
ADD_HL_RR(de);
ADD_HL_RR(hl);
ADD_HL_RR(sp);

CPU_INSTR(add_sp_n,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			cpu->instr_tmp.u16[1] = cpu->regs.sp;
			cpu->instr_tmp.u16[0] = cpu->regs.sp + (int8_t)mem_get(cpu->mem, cpu->regs.pc + 1);
			return false;
		case 2:
			cpu->regs.sp = (cpu->regs.sp & 0xFF00) | (cpu->instr_tmp.u16[0] & 0x00FF);
			return false;
		case 3:
		{
			uint16_t v = cpu->instr_tmp.u16[1];
			cpu->regs.sp = (cpu->regs.sp & 0x00FF) | (cpu->instr_tmp.u16[0] & 0xFF00);
			CPU_SET_FLAG_Z(cpu, 0);
			CPU_SET_FLAG_N(cpu, 0);
			CPU_SET_FLAG_H(cpu, (cpu->regs.sp & 0xF) < (v & 0xF));
			CPU_SET_FLAG_C(cpu, (cpu->regs.sp & 0xFF) < (v & 0xFF));
			break;
		}
	}
	cpu->regs.pc += 2;
	return true;
},
{
	snprintf(data, size, "add sp, $%02" PRIX8, mem_get(cpu->mem, cpu->regs.pc + 1));
});

CPU_INSTR(ld_hl_spn,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			cpu->instr_tmp.i8[0] = (int8_t)mem_get(cpu->mem, cpu->regs.pc + 1);
			return false;
		case 2:
		{
			uint8_t v = cpu->regs.sp;
			cpu->regs.hl = cpu->regs.sp + cpu->instr_tmp.i8[0];
			CPU_SET_FLAG_Z(cpu, 0);
			CPU_SET_FLAG_N(cpu, 0);
			CPU_SET_FLAG_H(cpu, (cpu->regs.hl & 0xF) < (v & 0xF));
			CPU_SET_FLAG_C(cpu, (cpu->regs.hl & 0xFF) < (v & 0xFF));
			break;
		}
	}
	cpu->regs.pc += 2;
	return true;
},
{
	snprintf(data, size, "ld hl, sp + $%02" PRIX8, mem_get(cpu->mem, cpu->regs.pc + 1));
});

#define JR_F(name, flag, ref) \
CPU_INSTR(jr_##name##_n, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			cpu->instr_tmp.i8[0] = (int8_t)mem_get(cpu->mem, cpu->regs.pc + 1); \
			if (CPU_GET_FLAG(cpu, CPU_FLAG_##flag) != ref) \
				break; \
			return false; \
		case 2: \
			cpu->regs.pc += cpu->instr_tmp.i8[0] + 2; \
			return true; \
	} \
	cpu->regs.pc += 2; \
	return true; \
}, \
{ \
	snprintf(data, size, "jr " #name ", %" PRId8, (int8_t)mem_get(cpu->mem, cpu->regs.pc + 1)); \
})

JR_F(nz, Z, 0);
JR_F( z, Z, 1);
JR_F(nc, C, 0);
JR_F( c, C, 1);

#define JP_F(name, flag, ref) \
CPU_INSTR(jp_##name##_nn, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			cpu->instr_tmp.u16[0] = mem_get(cpu->mem, cpu->regs.pc + 1); \
			return false; \
		case 2: \
			cpu->instr_tmp.u16[0] |= mem_get(cpu->mem, cpu->regs.pc + 2) << 8; \
			if (CPU_GET_FLAG(cpu, CPU_FLAG_##flag) != ref) \
				break; \
			return false; \
		case 3: \
			cpu->regs.pc = cpu->instr_tmp.u16[0]; \
			return true; \
	} \
	cpu->regs.pc += 3; \
	return true; \
}, \
{ \
	snprintf(data, size, "jp " #name ", $%02" PRIX8 "%02" PRIX8, mem_get(cpu->mem, cpu->regs.pc + 2), mem_get(cpu->mem, cpu->regs.pc + 1)); \
})

JP_F(nz, Z, 0);
JP_F( z, Z, 1);
JP_F(nc, C, 0);
JP_F( c, C, 1);

#define CALL_F(name, flag, ref) \
CPU_INSTR(call_##name##_nn, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			cpu->instr_tmp.u16[0] = mem_get(cpu->mem, cpu->regs.pc + 1); \
			return false; \
		case 2: \
			cpu->instr_tmp.u16[0] |= mem_get(cpu->mem, cpu->regs.pc + 2) << 8; \
			if (CPU_GET_FLAG(cpu, CPU_FLAG_##flag) != ref) \
				break; \
			return false; \
		case 3: \
			cpu->instr_tmp.u16[1] = cpu->regs.pc + 3; \
			return false; \
		case 4: \
			mem_set(cpu->mem, --cpu->regs.sp, cpu->instr_tmp.u16[1] >> 8); \
			return false; \
		case 5: \
			mem_set(cpu->mem, --cpu->regs.sp, cpu->instr_tmp.u16[1]); \
			cpu->regs.pc = cpu->instr_tmp.u16[0]; \
			return true; \
	} \
	cpu->regs.pc += 3; \
	return true; \
}, \
{ \
	snprintf(data, size, "call " #name ", $%02" PRIX8 "%02" PRIX8, mem_get(cpu->mem, cpu->regs.pc + 2), mem_get(cpu->mem, cpu->regs.pc + 1)); \
})

CALL_F(nz, Z, 0);
CALL_F( z, Z, 1);
CALL_F(nc, C, 0);
CALL_F( c, C, 1);

#define RET_F(name, flag, ref) \
CPU_INSTR(ret_##name, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			if (CPU_GET_FLAG(cpu, CPU_FLAG_##flag) != ref) \
				break; \
			return false; \
		case 2: \
			cpu->instr_tmp.u16[0] = mem_get(cpu->mem, cpu->regs.sp++); \
			return false; \
		case 3: \
			cpu->instr_tmp.u16[0] |= mem_get(cpu->mem, cpu->regs.sp++) << 8; \
			return false; \
		case 4: \
			cpu->regs.pc = cpu->instr_tmp.u16[0]; \
			return true; \
	} \
	cpu->regs.pc += 1; \
	return true; \
}, \
{ \
	snprintf(data, size, "ret " #name); \
})

RET_F(nz, Z, 0);
RET_F( z, Z, 1);
RET_F(nc, C, 0);
RET_F( c, C, 1);

CPU_INSTR(jr_dd,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			cpu->instr_tmp.i8[0] = (int8_t)mem_get(cpu->mem, cpu->regs.pc + 1);
			return false;
		case 2:
			cpu->regs.pc += cpu->instr_tmp.i8[0] + 2;
			return true;
	}
	cpu->regs.pc += 2;
	return true;
},
{
	snprintf(data, size, "jr %" PRId8, (int8_t)mem_get(cpu->mem, cpu->regs.pc + 1));
});

CPU_INSTR(jp_nn,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			cpu->instr_tmp.u16[0] = mem_get(cpu->mem, cpu->regs.pc + 1);
			return false;
		case 2:
			cpu->instr_tmp.u16[0] |= mem_get(cpu->mem, cpu->regs.pc + 2) << 8;
			return false;
		case 3:
			cpu->regs.pc = cpu->instr_tmp.u16[0];
			return true;
	}
	cpu->regs.pc += 3;
	return true;
},
{
	snprintf(data, size, "jp $%02" PRIX8 "%02" PRIX8, mem_get(cpu->mem, cpu->regs.pc + 2), mem_get(cpu->mem, cpu->regs.pc + 1));
});

CPU_INSTR(jp_hl,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			cpu->regs.pc = cpu->regs.hl;
			return true;
	}
	cpu->regs.pc += 1;
	return true;
},
{
	snprintf(data, size, "jp hl");
});

CPU_INSTR(call_nn,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			cpu->instr_tmp.u16[0] = mem_get(cpu->mem, cpu->regs.pc + 1);
			return false;
		case 2:
			cpu->instr_tmp.u16[0] |= mem_get(cpu->mem, cpu->regs.pc + 2) << 8;
			return false;
		case 3:
			cpu->instr_tmp.u16[1] = cpu->regs.pc + 3;
			return false;
		case 4:
			mem_set(cpu->mem, --cpu->regs.sp, cpu->instr_tmp.u16[1] >> 8);
			return false;
		case 5:
			mem_set(cpu->mem, --cpu->regs.sp, cpu->instr_tmp.u16[1]);
			cpu->regs.pc = cpu->instr_tmp.u16[0];
			return true;
	}
	cpu->regs.pc += 3;
	return true;
},
{
	snprintf(data, size, "call $%02" PRIX8 "%02" PRIX8, mem_get(cpu->mem, cpu->regs.pc + 2), mem_get(cpu->mem, cpu->regs.pc + 1));
});

CPU_INSTR(reti,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			cpu->instr_tmp.u16[0] = mem_get(cpu->mem, cpu->regs.sp++);
			return false;
		case 2:
			cpu->instr_tmp.u16[0] |= mem_get(cpu->mem, cpu->regs.sp++) << 8;
			return false;
		case 3:
			cpu->ime = true;
			cpu->regs.pc = cpu->instr_tmp.u16[0];
			return true;
	}
	cpu->regs.pc += 1;
	return true;
},
{
	snprintf(data, size, "reti");
});

CPU_INSTR(ret,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			cpu->instr_tmp.u16[0] = mem_get(cpu->mem, cpu->regs.sp++);
			return false;
		case 2:
			cpu->instr_tmp.u16[0] |= mem_get(cpu->mem, cpu->regs.sp++) << 8;
			return false;
		case 3:
			cpu->regs.pc = cpu->instr_tmp.u16[0];
			return true;
	}
	cpu->regs.pc += 1;
	return true;
},
{
	snprintf(data, size, "ret");
});

#define RST(v) \
CPU_INSTR(rst_##v, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			cpu->instr_tmp.u16[0] = cpu->regs.pc + 1; \
			return false; \
		case 2: \
			mem_set(cpu->mem, --cpu->regs.sp, cpu->instr_tmp.u16[0] >> 8); \
			return false; \
		case 3: \
			mem_set(cpu->mem, --cpu->regs.sp, cpu->instr_tmp.u16[0]); \
			cpu->regs.pc = 0x##v; \
			return true; \
	} \
	cpu->regs.pc += 1; \
	return true; \
}, \
{ \
	snprintf(data, size, "rst $%02" PRIX8, 0x##v); \
})

RST(00);
RST(08);
RST(10);
RST(18);
RST(20);
RST(28);
RST(30);
RST(38);

#define INT(v) \
CPU_INSTR(int_##v, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			cpu->instr_tmp.u16[0] = cpu->regs.pc; \
			return false; \
		case 2: \
			mem_set(cpu->mem, --cpu->regs.sp, cpu->instr_tmp.u16[0] >> 8); \
			return false; \
		case 3: \
			mem_set(cpu->mem, --cpu->regs.sp, cpu->instr_tmp.u16[0]); \
			return false; \
		case 4: \
			cpu->regs.pc = 0x##v; \
			cpu->ime = false; \
			break; \
	} \
	return true; \
}, \
{ \
	snprintf(data, size, "int $%02" PRIX8, 0x##v); \
})

INT(40);
INT(48);
INT(50);
INT(58);
INT(60);

#define LD_RR_NN(r1, r2) \
CPU_INSTR(ld_##r1####r2##_nn, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			cpu->regs.r2 = mem_get(cpu->mem, cpu->regs.pc + 1); \
			return false; \
		case 2: \
			cpu->regs.r1 = mem_get(cpu->mem, cpu->regs.pc + 2); \
			break; \
	} \
	cpu->regs.pc += 3; \
	return true; \
}, \
{ \
	snprintf(data, size, "ld " #r1 #r2 ", $%02" PRIX8 "%02" PRIX8, mem_get(cpu->mem, cpu->regs.pc + 2), mem_get(cpu->mem, cpu->regs.pc + 1)); \
})

LD_RR_NN(b, c);
LD_RR_NN(d, e);
LD_RR_NN(h, l);
LD_RR_NN(s, p);

CPU_INSTR(ld_nn_sp,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			cpu->instr_tmp.u16[0] = mem_get(cpu->mem, cpu->regs.pc + 1);
			return false;
		case 2:
			cpu->instr_tmp.u16[0] |= mem_get(cpu->mem, cpu->regs.pc + 2) << 8;
			return false;
		case 3:
			mem_set(cpu->mem, cpu->instr_tmp.u16[0], cpu->regs.sp);
			return false;
		case 4:
			mem_set(cpu->mem, cpu->instr_tmp.u16[0] + 1, cpu->regs.sp >> 8);
			break;
	}
	cpu->regs.pc += 3;
	return true;
},
{
	snprintf(data, size, "ld ($%02" PRIX8 "%02" PRIX8 "), sp", mem_get(cpu->mem, cpu->regs.pc + 2), mem_get(cpu->mem, cpu->regs.pc + 1));
});

#define PUSH_RR(r1, r2) \
CPU_INSTR(push_##r1####r2, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			return false; \
		case 2: \
			mem_set(cpu->mem, --cpu->regs.sp, cpu->regs.r1); \
			return false; \
		case 3: \
			mem_set(cpu->mem, --cpu->regs.sp, cpu->regs.r2); \
			break; \
	} \
	cpu->regs.pc += 1; \
	return true; \
}, \
{ \
	snprintf(data, size, "push " #r1 #r2); \
})

PUSH_RR(b, c);
PUSH_RR(d, e);
PUSH_RR(h, l);
PUSH_RR(a, f);

#define POP_RR(r1, r2) \
CPU_INSTR(pop_##r1####r2, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			cpu->regs.r2 = mem_get(cpu->mem, cpu->regs.sp++); \
			return false; \
		case 2: \
			cpu->regs.r1 = mem_get(cpu->mem, cpu->regs.sp++); \
			cpu->regs.f &= 0xF0; \
			break; \
	} \
	cpu->regs.pc += 1; \
	return true; \
}, \
{ \
	snprintf(data, size, "pop " #r1 #r2); \
})

POP_RR(b, c);
POP_RR(d, e);
POP_RR(h, l);
POP_RR(a, f);;

CPU_INSTR(ld_sp_hl,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			cpu->regs.sp = cpu->regs.hl;
			break;
	}
	cpu->regs.pc += 1;
	return true;
},
{
	snprintf(data, size, "ld sp, hl");
});

#define LD_RR_A(rr) \
CPU_INSTR(ld_##rr##_a, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			mem_set(cpu->mem, cpu->regs.rr, cpu->regs.a); \
			break; \
	} \
	cpu->regs.pc += 1; \
	return true; \
}, \
{ \
	snprintf(data, size, "ld (" #rr "), a"); \
})

LD_RR_A(bc);
LD_RR_A(de);

#define LD_A_RR(rr) \
CPU_INSTR(ld_a_##rr, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			cpu->regs.a = mem_get(cpu->mem, cpu->regs.rr); \
			break; \
	} \
	cpu->regs.pc += 1; \
	return true; \
}, \
{ \
	snprintf(data, size, "ld a, (" #rr ")"); \
})

LD_A_RR(bc);
LD_A_RR(de);

#define LD_R_N(r) \
CPU_INSTR(ld_##r##_n, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			cpu->regs.r = mem_get(cpu->mem, cpu->regs.pc + 1); \
			break; \
	} \
	cpu->regs.pc += 2; \
	return true; \
}, \
{ \
	snprintf(data, size, "ld " #r ", $%02" PRIX8, mem_get(cpu->mem, cpu->regs.pc + 1)); \
})

LD_R_N(a);
LD_R_N(b);
LD_R_N(c);
LD_R_N(d);
LD_R_N(e);
LD_R_N(h);
LD_R_N(l);

#define LD_R_R(r1, r2) \
CPU_INSTR(ld_##r1##_##r2, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			cpu->regs.r1 = cpu->regs.r2; \
			break; \
	} \
	cpu->regs.pc += 1; \
	return true; \
}, \
{ \
	snprintf(data, size, "ld " #r1 ", " #r2); \
})

#define LD_R(r) \
LD_R_R(r, a); \
LD_R_R(r, b); \
LD_R_R(r, c); \
LD_R_R(r, d); \
LD_R_R(r, e); \
LD_R_R(r, h); \
LD_R_R(r, l); \
CPU_INSTR(ld_##r##_hl, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			cpu->regs.r = mem_get(cpu->mem, cpu->regs.hl); \
			break; \
	} \
	cpu->regs.pc += 1; \
	return true; \
}, \
{ \
	snprintf(data, size, "ld " #r ", (hl)"); \
}); \
CPU_INSTR(ld_hl_##r, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			mem_set(cpu->mem, cpu->regs.hl, cpu->regs.r); \
			break; \
	} \
	cpu->regs.pc += 1; \
	return true; \
}, \
{ \
	snprintf(data, size, "ld (hl), " #r); \
})

LD_R(a);
LD_R(b);
LD_R(c);
LD_R(d);
LD_R(e);
LD_R(h);
LD_R(l);

CPU_INSTR(ldi_hl_a,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			mem_set(cpu->mem, cpu->regs.hl++, cpu->regs.a);
			break;
	}
	cpu->regs.pc += 1;
	return true;
},
{
	snprintf(data, size, "ldi (hl), a");
});

CPU_INSTR(ldi_a_hl,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			cpu->regs.a = mem_get(cpu->mem, cpu->regs.hl++);
			break;
	}
	cpu->regs.pc += 1;
	return true;
},
{
	snprintf(data, size, "ldi a, (hl)");
});

CPU_INSTR(ldd_hl_a,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			mem_set(cpu->mem, cpu->regs.hl--, cpu->regs.a);
			break;
	}
	cpu->regs.pc += 1;
	return true;
},
{
	snprintf(data, size, "ldd (hl), a");
});

CPU_INSTR(ldd_a_hl,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			cpu->regs.a = mem_get(cpu->mem, cpu->regs.hl--);
			break;
	}
	cpu->regs.pc += 1;
	return true;
},
{
	snprintf(data, size, "ldd a, (hl)");
});

CPU_INSTR(ld_hl_n,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			cpu->instr_tmp.u8[0] = mem_get(cpu->mem, cpu->regs.pc + 1);
			return false;
		case 2:
			mem_set(cpu->mem, cpu->regs.hl, cpu->instr_tmp.u8[0]);
			break;
	}
	cpu->regs.pc += 2;
	return true;
},
{
	snprintf(data, size, "ld (hl), $%02" PRIX8, mem_get(cpu->mem, cpu->regs.pc + 1));
});

CPU_INSTR(ld_ffn_a,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			cpu->instr_tmp.u8[0] = mem_get(cpu->mem, cpu->regs.pc + 1);
			return false;
		case 2:
			mem_set(cpu->mem, 0xFF00 + cpu->instr_tmp.u8[0], cpu->regs.a);
			break;
	}
	cpu->regs.pc += 2;
	return true;
},
{
	snprintf(data, size, "ld ($FF00 + $%02" PRIX8 "), a", mem_get(cpu->mem, cpu->regs.pc + 1));
});

CPU_INSTR(ld_ffc_a,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			mem_set(cpu->mem, 0xFF00 + cpu->regs.c, cpu->regs.a);
			break;
	}
	cpu->regs.pc += 1;
	return true;
},
{
	snprintf(data, size, "ld ($FF00 + c), a");
});

CPU_INSTR(ld_a_ffn,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			cpu->instr_tmp.u8[0] = mem_get(cpu->mem, cpu->regs.pc + 1);
			return false;
		case 2:
			cpu->regs.a = mem_get(cpu->mem, 0xFF00 + cpu->instr_tmp.u8[0]);
			break;
	}
	cpu->regs.pc += 2;
	return true;
},
{
	snprintf(data, size, "ld a, ($FF00 + $%02" PRIX8 ")", mem_get(cpu->mem, cpu->regs.pc + 1));
});

CPU_INSTR(ld_a_ffc,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			cpu->regs.a = mem_get(cpu->mem, 0xFF00 + cpu->regs.c);
			break;
	}
	cpu->regs.pc += 1;
	return true;
},
{
	snprintf(data, size, "ld a, ($FF00 + c)");
});

CPU_INSTR(ld_a_nn,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			cpu->instr_tmp.u16[0] = mem_get(cpu->mem, cpu->regs.pc + 1);
			return false;
		case 2:
			cpu->instr_tmp.u16[0] |= mem_get(cpu->mem, cpu->regs.pc + 2) << 8;
			return false;
		case 3:
			cpu->regs.a = mem_get(cpu->mem, cpu->instr_tmp.u16[0]);
			break;
	}
	cpu->regs.pc += 3;
	return true;
},
{
	snprintf(data, size, "ld a, ($%02" PRIX8 "%02" PRIX8 ")", mem_get(cpu->mem, cpu->regs.pc + 2), mem_get(cpu->mem, cpu->regs.pc + 1));
});

CPU_INSTR(ld_nn_a,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			cpu->instr_tmp.u16[0] = mem_get(cpu->mem, cpu->regs.pc + 1);
			return false;
		case 2:
			cpu->instr_tmp.u16[0] |= mem_get(cpu->mem, cpu->regs.pc + 2) << 8;
			return false;
		case 3:
			mem_set(cpu->mem, cpu->instr_tmp.u16[0], cpu->regs.a);
			break;
	}
	cpu->regs.pc += 3;
	return true;
},
{
	snprintf(data, size, "ld ($%02" PRIX8 "%02" PRIX8 "), a", mem_get(cpu->mem, cpu->regs.pc + 2), mem_get(cpu->mem, cpu->regs.pc + 1));
});

CPU_INSTR(rlca,
{
	switch (cpu->instr_cycle)
	{
		case 0:
		{
			uint8_t v = cpu->regs.a;
			uint8_t newv = (v << 1) | (v >> 7);
			cpu->regs.a = newv;
			CPU_SET_FLAG_Z(cpu, 0);
			CPU_SET_FLAG_N(cpu, 0);
			CPU_SET_FLAG_H(cpu, 0);
			CPU_SET_FLAG_C(cpu, v & 0x80);
			break;
		}
	}
	cpu->regs.pc++;
	return true;
},
{
	snprintf(data, size, "rlca");
});

CPU_INSTR(rrca,
{
	switch (cpu->instr_cycle)
	{
		case 0:
		{
			uint8_t v = cpu->regs.a;
			uint8_t newv = (v >> 1) | (v << 7);
			cpu->regs.a = newv;
			CPU_SET_FLAG_Z(cpu, 0);
			CPU_SET_FLAG_N(cpu, 0);
			CPU_SET_FLAG_H(cpu, 0);
			CPU_SET_FLAG_C(cpu, v & 0x1);
			break;
		}
	}
	cpu->regs.pc++;
	return true;
},
{
	snprintf(data, size, "rrca");
});

CPU_INSTR(rla,
{
	switch (cpu->instr_cycle)
	{
		case 0:
		{
			uint8_t v = cpu->regs.a;
			uint8_t newv = (v << 1) | CPU_GET_FLAG_C(cpu);
			cpu->regs.a = newv;
			CPU_SET_FLAG_Z(cpu, 0);
			CPU_SET_FLAG_N(cpu, 0);
			CPU_SET_FLAG_H(cpu, 0);
			CPU_SET_FLAG_C(cpu, v & 0x80);
			break;
		}
	}
	cpu->regs.pc++;
	return true;
},
{
	snprintf(data, size, "rla");
});

CPU_INSTR(rra,
{
	switch (cpu->instr_cycle)
	{
		case 0:
		{
			uint8_t v = cpu->regs.a;
			uint8_t newv = (v >> 1) | (CPU_GET_FLAG_C(cpu) << 7);
			cpu->regs.a = newv;
			CPU_SET_FLAG_Z(cpu, 0);
			CPU_SET_FLAG_N(cpu, 0);
			CPU_SET_FLAG_H(cpu, 0);
			CPU_SET_FLAG_C(cpu, v & 0x1);
			break;
		}
	}
	cpu->regs.pc++;
	return true;
},
{
	snprintf(data, size, "rra");
});

#define RLC_R(r) \
CPU_INSTR(rlc_##r, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
		{ \
			uint8_t v = cpu->regs.r; \
			uint8_t newv = (v << 1) | (v >> 7); \
			cpu->regs.r = newv; \
			CPU_SET_FLAG_Z(cpu, !newv); \
			CPU_SET_FLAG_N(cpu, 0); \
			CPU_SET_FLAG_H(cpu, 0); \
			CPU_SET_FLAG_C(cpu, v & 0x80); \
			break; \
		} \
	} \
	cpu->regs.pc += 2; \
	return true; \
}, \
{ \
	snprintf(data, size, "rlc " #r); \
})

RLC_R(a);
RLC_R(b);
RLC_R(c);
RLC_R(d);
RLC_R(e);
RLC_R(h);
RLC_R(l);

CPU_INSTR(rlc_hl,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			return false;
		case 2:
			cpu->instr_tmp.u8[0] = mem_get(cpu->mem, cpu->regs.hl);
			return false;
		case 3:
		{
			uint8_t v = cpu->instr_tmp.u8[0];
			uint8_t newv = (v << 1) | (v >> 7);
			mem_set(cpu->mem, cpu->regs.hl, newv);
			CPU_SET_FLAG_Z(cpu, !newv);
			CPU_SET_FLAG_N(cpu, 0);
			CPU_SET_FLAG_H(cpu, 0);
			CPU_SET_FLAG_C(cpu, v & 0x80);
			break;
		}
	}
	cpu->regs.pc += 2;
	return true;
},
{
	snprintf(data, size, "rlc (hl)");
});

#define RRC_R(r) \
CPU_INSTR(rrc_##r, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
		{ \
			uint8_t v = cpu->regs.r; \
			uint8_t newv = (v >> 1) | (v << 7); \
			cpu->regs.r = newv; \
			CPU_SET_FLAG_Z(cpu, !newv); \
			CPU_SET_FLAG_N(cpu, 0); \
			CPU_SET_FLAG_H(cpu, 0); \
			CPU_SET_FLAG_C(cpu, v & 0x1); \
			break; \
		} \
	} \
	cpu->regs.pc += 2; \
	return true; \
}, \
{ \
	snprintf(data, size, "rrc " #r); \
})

RRC_R(a);
RRC_R(b);
RRC_R(c);
RRC_R(d);
RRC_R(e);
RRC_R(h);
RRC_R(l);

CPU_INSTR(rrc_hl,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			return false;
		case 2:
			cpu->instr_tmp.u8[0] = mem_get(cpu->mem, cpu->regs.hl);
			return false;
		case 3:
		{
			uint8_t v = cpu->instr_tmp.u8[0];
			uint8_t newv = (v >> 1) | (v << 7);
			mem_set(cpu->mem, cpu->regs.hl, newv);
			CPU_SET_FLAG_Z(cpu, !newv);
			CPU_SET_FLAG_N(cpu, 0);
			CPU_SET_FLAG_H(cpu, 0);
			CPU_SET_FLAG_C(cpu, v & 0x1);
			break;
		}
	}
	cpu->regs.pc += 2;
	return true;
},
{
	snprintf(data, size, "rrc (hl)");
});

#define RL_R(r) \
CPU_INSTR(rl_##r, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
		{ \
			uint8_t v = cpu->regs.r; \
			uint8_t newv = (v << 1) | CPU_GET_FLAG_C(cpu); \
			cpu->regs.r = newv; \
			CPU_SET_FLAG_Z(cpu, !newv); \
			CPU_SET_FLAG_N(cpu, 0); \
			CPU_SET_FLAG_H(cpu, 0); \
			CPU_SET_FLAG_C(cpu, v & 0x80); \
			break; \
		} \
	} \
	cpu->regs.pc += 2; \
	return true; \
}, \
{ \
	snprintf(data, size, "rl " #r); \
})

RL_R(a);
RL_R(b);
RL_R(c);
RL_R(d);
RL_R(e);
RL_R(h);
RL_R(l);

CPU_INSTR(rl_hl,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			return false;
		case 2:
			cpu->instr_tmp.u8[0] = mem_get(cpu->mem, cpu->regs.hl);
			return false;
		case 3:
		{
			uint8_t v = cpu->instr_tmp.u8[0];
			uint8_t newv = (v << 1) | CPU_GET_FLAG_C(cpu);
			mem_set(cpu->mem, cpu->regs.hl, newv);
			CPU_SET_FLAG_Z(cpu, !newv);
			CPU_SET_FLAG_N(cpu, 0);
			CPU_SET_FLAG_H(cpu, 0);
			CPU_SET_FLAG_C(cpu, v & 0x80);
			break;
		}
	}
	cpu->regs.pc += 2;
	return true;
},
{
	snprintf(data, size, "rl (hl)");
});

#define RR_R(r) \
CPU_INSTR(rr_##r, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
		{ \
			uint8_t v = cpu->regs.r; \
			uint8_t newv = (v >> 1) | (CPU_GET_FLAG_C(cpu) << 7); \
			cpu->regs.r = newv; \
			CPU_SET_FLAG_Z(cpu, !newv); \
			CPU_SET_FLAG_N(cpu, 0); \
			CPU_SET_FLAG_H(cpu, 0); \
			CPU_SET_FLAG_C(cpu, v & 0x1); \
			break; \
		} \
	} \
	cpu->regs.pc += 2; \
	return true; \
}, \
{ \
	snprintf(data, size, "rr " #r); \
})

RR_R(a);
RR_R(b);
RR_R(c);
RR_R(d);
RR_R(e);
RR_R(h);
RR_R(l);

CPU_INSTR(rr_hl,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			return false;
		case 2:
			cpu->instr_tmp.u8[0] = mem_get(cpu->mem, cpu->regs.hl);
			return false;
		case 3:
		{
			uint8_t v = cpu->instr_tmp.u8[0];
			uint8_t newv = (v >> 1) | (CPU_GET_FLAG_C(cpu) << 7);
			mem_set(cpu->mem, cpu->regs.hl, newv);
			CPU_SET_FLAG_Z(cpu, !newv);
			CPU_SET_FLAG_N(cpu, 0);
			CPU_SET_FLAG_H(cpu, 0);
			CPU_SET_FLAG_C(cpu, v & 0x1);
			break;
		}
	}
	cpu->regs.pc += 2;
	return true;
},
{
	snprintf(data, size, "rl (hl)");
});

#define SLA_R(r) \
CPU_INSTR(sla_##r, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
		{ \
			uint8_t v = cpu->regs.r; \
			uint8_t newv = v << 1; \
			cpu->regs.r = newv; \
			CPU_SET_FLAG_Z(cpu, !newv); \
			CPU_SET_FLAG_N(cpu, 0); \
			CPU_SET_FLAG_H(cpu, 0); \
			CPU_SET_FLAG_C(cpu, v & 0x80); \
			break; \
		} \
	} \
	cpu->regs.pc += 2; \
	return true; \
}, \
{ \
	snprintf(data, size, "sla " #r); \
})

SLA_R(a);
SLA_R(b);
SLA_R(c);
SLA_R(d);
SLA_R(e);
SLA_R(h);
SLA_R(l);

CPU_INSTR(sla_hl,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			return false;
		case 2:
			cpu->instr_tmp.u8[0] = mem_get(cpu->mem, cpu->regs.hl);
			return false;
		case 3:
		{
			uint8_t v = cpu->instr_tmp.u8[0];
			uint8_t newv = v << 1;
			mem_set(cpu->mem, cpu->regs.hl, newv);
			CPU_SET_FLAG_Z(cpu, !newv);
			CPU_SET_FLAG_N(cpu, 0);
			CPU_SET_FLAG_H(cpu, 0);
			CPU_SET_FLAG_C(cpu, v & 0x80);
			break;
		}
	}
	cpu->regs.pc += 2;
	return true;
},
{
	snprintf(data, size, "sla (hl)");
});

#define SRA_R(r) \
CPU_INSTR(sra_##r, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
		{ \
			uint8_t v = cpu->regs.r; \
			uint8_t newv = (v >> 1) | (v & 0x80); \
			cpu->regs.r = newv; \
			CPU_SET_FLAG_Z(cpu, !newv); \
			CPU_SET_FLAG_N(cpu, 0); \
			CPU_SET_FLAG_H(cpu, 0); \
			CPU_SET_FLAG_C(cpu, v & 0x1); \
			break; \
		} \
	} \
	cpu->regs.pc += 2; \
	return true; \
}, \
{ \
	snprintf(data, size, "sra " #r); \
})

SRA_R(a);
SRA_R(b);
SRA_R(c);
SRA_R(d);
SRA_R(e);
SRA_R(h);
SRA_R(l);

CPU_INSTR(sra_hl,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			return false;
		case 2:
			cpu->instr_tmp.u8[0] = mem_get(cpu->mem, cpu->regs.hl);
			return false;
		case 3:
		{
			uint8_t v = cpu->instr_tmp.u8[0];
			uint8_t newv = (v >> 1) | (v & 0x80);;
			mem_set(cpu->mem, cpu->regs.hl, newv);
			CPU_SET_FLAG_Z(cpu, !newv);
			CPU_SET_FLAG_N(cpu, 0);
			CPU_SET_FLAG_H(cpu, 0);
			CPU_SET_FLAG_C(cpu, v & 0x1);
			break;
		}
	}
	cpu->regs.pc += 2;
	return true;
},
{
	snprintf(data, size, "sra (hl)");
});

#define SWAP_R(r) \
CPU_INSTR(swap_##r, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			cpu->regs.r = (cpu->regs.r >> 4) | (cpu->regs.r << 4); \
			CPU_SET_FLAG_Z(cpu, !cpu->regs.r); \
			CPU_SET_FLAG_N(cpu, 0); \
			CPU_SET_FLAG_H(cpu, 0); \
			CPU_SET_FLAG_C(cpu, 0); \
			break; \
	} \
	cpu->regs.pc += 2; \
	return true; \
}, \
{ \
	snprintf(data, size, "swap " #r); \
})

SWAP_R(a);
SWAP_R(b);
SWAP_R(c);
SWAP_R(d);
SWAP_R(e);
SWAP_R(h);
SWAP_R(l);

CPU_INSTR(swap_hl,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			return false;
		case 2:
			cpu->instr_tmp.u8[0] = mem_get(cpu->mem, cpu->regs.hl);
			return false;
		case 3:
		{
			uint8_t v = cpu->instr_tmp.u8[0];
			uint8_t newv = (v >> 4) | (v << 4);
			mem_set(cpu->mem, cpu->regs.hl, newv);
			CPU_SET_FLAG_Z(cpu, !newv);
			CPU_SET_FLAG_N(cpu, 0);
			CPU_SET_FLAG_H(cpu, 0);
			CPU_SET_FLAG_C(cpu, 0);
			break;
		}
	}
	cpu->regs.pc += 2;
	return true;
},
{
	snprintf(data, size, "swap (hl)");
});

#define SRL_R(r) \
CPU_INSTR(srl_##r, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
		{ \
			uint8_t v = cpu->regs.r; \
			uint8_t newv = v >> 1; \
			cpu->regs.r = newv; \
			CPU_SET_FLAG_Z(cpu, !newv); \
			CPU_SET_FLAG_N(cpu, 0); \
			CPU_SET_FLAG_H(cpu, 0); \
			CPU_SET_FLAG_C(cpu, v & 0x1); \
			break; \
		} \
	} \
	cpu->regs.pc += 2; \
	return true; \
}, \
{ \
	snprintf(data, size, "srl " #r); \
})

SRL_R(a);
SRL_R(b);
SRL_R(c);
SRL_R(d);
SRL_R(e);
SRL_R(h);
SRL_R(l);

CPU_INSTR(srl_hl,
{
	switch (cpu->instr_cycle)
	{
		case 0:
			return false;
		case 1:
			return false;
		case 2:
			cpu->instr_tmp.u8[0] = mem_get(cpu->mem, cpu->regs.hl);
			return false;
		case 3:
		{
			uint8_t v = cpu->instr_tmp.u8[0];
			uint8_t newv = v >> 1;
			mem_set(cpu->mem, cpu->regs.hl, newv);
			CPU_SET_FLAG_Z(cpu, !newv);
			CPU_SET_FLAG_N(cpu, 0);
			CPU_SET_FLAG_H(cpu, 0);
			CPU_SET_FLAG_C(cpu, v & 0x1);
			break;
		}
	}
	cpu->regs.pc += 2;
	return true;
},
{
	snprintf(data, size, "srl (hl)");
});

#define BIT_N_R(n, r) \
CPU_INSTR(bit_##n##_##r, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			CPU_SET_FLAG_Z(cpu, !(cpu->regs.r & (1 << n))); \
			CPU_SET_FLAG_N(cpu, 0); \
			CPU_SET_FLAG_H(cpu, 1); \
			break; \
	} \
	cpu->regs.pc += 2; \
	return true; \
}, \
{ \
	snprintf(data, size, "bit " #n ", " #r); \
})

#define BIT_N(n) \
BIT_N_R(n, a); \
BIT_N_R(n, b); \
BIT_N_R(n, c); \
BIT_N_R(n, d); \
BIT_N_R(n, e); \
BIT_N_R(n, h); \
BIT_N_R(n, l); \
CPU_INSTR(bit_##n##_hl, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			return false; \
		case 2: \
			CPU_SET_FLAG_Z(cpu, !(mem_get(cpu->mem, cpu->regs.hl) & (1 << n))); \
			CPU_SET_FLAG_N(cpu, 0); \
			CPU_SET_FLAG_H(cpu, 1); \
			break; \
	} \
	cpu->regs.pc += 2; \
	return true; \
}, \
{ \
	snprintf(data, size, "bit " #n ", (hl)"); \
})

BIT_N(0);
BIT_N(1);
BIT_N(2);
BIT_N(3);
BIT_N(4);
BIT_N(5);
BIT_N(6);
BIT_N(7);

#define RES_N_R(n, r) \
CPU_INSTR(res_##n##_##r, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			cpu->regs.r &= ~(1 << n); \
			break; \
	} \
	cpu->regs.pc += 2; \
	return true; \
}, \
{ \
	snprintf(data, size, "res " #n ", " #r); \
})

#define RES_N(n) \
RES_N_R(n, a); \
RES_N_R(n, b); \
RES_N_R(n, c); \
RES_N_R(n, d); \
RES_N_R(n, e); \
RES_N_R(n, h); \
RES_N_R(n, l); \
CPU_INSTR(res_##n##_hl, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			return false; \
		case 2: \
			cpu->instr_tmp.u8[0] = mem_get(cpu->mem, cpu->regs.hl); \
			return false; \
		case 3: \
			mem_set(cpu->mem, cpu->regs.hl, cpu->instr_tmp.u8[0] & ~(1 << n)); \
			break; \
	} \
	cpu->regs.pc += 2; \
	return true; \
}, \
{ \
	snprintf(data, size, "res " #n ", (hl)"); \
})

RES_N(0);
RES_N(1);
RES_N(2);
RES_N(3);
RES_N(4);
RES_N(5);
RES_N(6);
RES_N(7);

#define SET_N_R(n, r) \
CPU_INSTR(set_##n##_##r, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			cpu->regs.r |= 1 << n; \
			break; \
	} \
	cpu->regs.pc += 2; \
	return true; \
}, \
{ \
	snprintf(data, size, "set " #n ", " #r); \
})

#define SET_N(n) \
SET_N_R(n, a); \
SET_N_R(n, b); \
SET_N_R(n, c); \
SET_N_R(n, d); \
SET_N_R(n, e); \
SET_N_R(n, h); \
SET_N_R(n, l); \
CPU_INSTR(set_##n##_hl, \
{ \
	switch (cpu->instr_cycle) \
	{ \
		case 0: \
			return false; \
		case 1: \
			return false; \
		case 2: \
			cpu->instr_tmp.u8[0] = mem_get(cpu->mem, cpu->regs.hl); \
			return false; \
		case 3: \
			mem_set(cpu->mem, cpu->regs.hl, cpu->instr_tmp.u8[0] | (1 << n)); \
			break; \
	} \
	cpu->regs.pc += 2; \
	return true; \
}, \
{ \
	snprintf(data, size, "set " #n ", (hl)"); \
})

SET_N(0);
SET_N(1);
SET_N(2);
SET_N(3);
SET_N(4);
SET_N(5);
SET_N(6);
SET_N(7);

CPU_INSTR(unset,
{
	cpu->regs.pc++;
	return true;
},
{
	snprintf(data, size, "UNSET");
});

const struct cpu_instr *cpu_instr[256] =
{
	/* 0x00 */ &nop       , &ld_bc_nn , &ld_bc_a, &inc_bc,
	/* 0x04 */ &inc_b     , &dec_b    , &ld_b_n  , &rlca,
	/* 0x08 */ &ld_nn_sp  , &add_hl_bc, &ld_a_bc , &dec_bc,
	/* 0x0C */ &inc_c     , &dec_c    , &ld_c_n  , &rrca,
	/* 0x10 */ &stop      , &ld_de_nn , &ld_de_a , &inc_de,
	/* 0x14 */ &inc_d     , &dec_d    , &ld_d_n  , &rla,
	/* 0x18 */ &jr_dd     , &add_hl_de, &ld_a_de , &dec_de,
	/* 0x1C */ &inc_e     , &dec_e    , &ld_e_n  , &rra,
	/* 0x20 */ &jr_nz_n   , &ld_hl_nn , &ldi_hl_a, &inc_hl,
	/* 0x24 */ &inc_h     , &dec_h    , &ld_h_n  , &daa,
	/* 0x28 */ &jr_z_n    , &add_hl_hl, &ldi_a_hl, &dec_hl,
	/* 0x2C */ &inc_l     , &dec_l    , &ld_l_n  , &cpl,
	/* 0x30 */ &jr_nc_n   , &ld_sp_nn , &ldd_hl_a, &inc_sp,
	/* 0x34 */ &inc_rhl   , &dec_rhl  , &ld_hl_n , &scf,
	/* 0x38 */ &jr_c_n    , &add_hl_sp, &ldd_a_hl, &dec_sp,
	/* 0x3C */ &inc_a     , &dec_a    , &ld_a_n  , &ccf,
	/* 0x40 */ &ld_b_b    , &ld_b_c   , &ld_b_d  , &ld_b_e,
	/* 0x44 */ &ld_b_h    , &ld_b_l   , &ld_b_hl , &ld_b_a,
	/* 0x48 */ &ld_c_b    , &ld_c_c   , &ld_c_d  , &ld_c_e,
	/* 0x4C */ &ld_c_h    , &ld_c_l   , &ld_c_hl , &ld_c_a,
	/* 0x50 */ &ld_d_b    , &ld_d_c   , &ld_d_d  , &ld_d_e,
	/* 0x54 */ &ld_d_h    , &ld_d_l   , &ld_d_hl , &ld_d_a,
	/* 0x58 */ &ld_e_b    , &ld_e_c   , &ld_e_d  , &ld_e_e,
	/* 0x5C */ &ld_e_h    , &ld_e_l   , &ld_e_hl , &ld_e_a,
	/* 0x60 */ &ld_h_b    , &ld_h_c   , &ld_h_d  , &ld_h_e,
	/* 0x64 */ &ld_h_h    , &ld_h_l   , &ld_h_hl , &ld_h_a,
	/* 0x68 */ &ld_l_b    , &ld_l_c   , &ld_l_d  , &ld_l_e,
	/* 0x6C */ &ld_l_h    , &ld_l_l   , &ld_l_hl , &ld_l_a,
	/* 0x70 */ &ld_hl_b   , &ld_hl_c  , &ld_hl_d , &ld_hl_e,
	/* 0x74 */ &ld_hl_h   , &ld_hl_l  , &halt    , &ld_hl_a,
	/* 0x78 */ &ld_a_b    , &ld_a_c   , &ld_a_d  , &ld_a_e,
	/* 0x7C */ &ld_a_h    , &ld_a_l   , &ld_a_hl , &ld_a_a,
	/* 0x80 */ &add_a_b   , &add_a_c  , &add_a_d , &add_a_e,
	/* 0x84 */ &add_a_h   , &add_a_l  , &add_a_hl, &add_a_a,
	/* 0x88 */ &adc_a_b   , &adc_a_c  , &adc_a_d , &adc_a_e,
	/* 0x8C */ &adc_a_h   , &adc_a_l  , &adc_a_hl, &adc_a_a,
	/* 0x90 */ &sub_a_b   , &sub_a_c  , &sub_a_d , &sub_a_e,
	/* 0x94 */ &sub_a_h   , &sub_a_l  , &sub_a_hl, &sub_a_a,
	/* 0x98 */ &sbc_a_b   , &sbc_a_c  , &sbc_a_d , &sbc_a_e,
	/* 0x9C */ &sbc_a_h   , &sbc_a_l  , &sbc_a_hl, &sbc_a_a,
	/* 0xA0 */ &and_a_b   , &and_a_c  , &and_a_d , &and_a_e,
	/* 0xA4 */ &and_a_h   , &and_a_l  , &and_a_hl, &and_a_a,
	/* 0xA8 */ &xor_a_b   , &xor_a_c  , &xor_a_d , &xor_a_e,
	/* 0xAC */ &xor_a_h   , &xor_a_l  , &xor_a_hl, &xor_a_a,
	/* 0xB0 */ &or_a_b    , &or_a_c   , &or_a_d  , &or_a_e,
	/* 0xB4 */ &or_a_h    , &or_a_l   , &or_a_hl , &or_a_a,
	/* 0xB8 */ &cp_a_b    , &cp_a_c   , &cp_a_d  , &cp_a_e,
	/* 0xBC */ &cp_a_h    , &cp_a_l   , &cp_a_hl , &cp_a_a,
	/* 0xC0 */ &ret_nz    , &pop_bc   , &jp_nz_nn, &jp_nn,
	/* 0xC4 */ &call_nz_nn, &push_bc  , &add_a_n , &rst_00,
	/* 0xC8 */ &ret_z     , &ret      , &jp_z_nn, &unset,
	/* 0xCC */ &call_z_nn , &call_nn  , &adc_a_n , &rst_08,
	/* 0xD0 */ &ret_nc    , &pop_de   , &jp_nc_nn, &unset,
	/* 0xD4 */ &call_nc_nn, &push_de  , &sub_a_n , &rst_10,
	/* 0xD8 */ &ret_c     , &reti     , &jp_c_nn , &unset,
	/* 0xDC */ &call_c_nn , &unset    , &sbc_a_n , &rst_18,
	/* 0xE0 */ &ld_ffn_a  , &pop_hl   , &ld_ffc_a, &unset,
	/* 0xE4 */ &unset     , &push_hl  , &and_a_n , &rst_20,
	/* 0xE8 */ &add_sp_n  , &jp_hl    , &ld_nn_a , &unset,
	/* 0xEC */ &unset     , &unset    , &xor_a_n , &rst_28,
	/* 0xF0 */ &ld_a_ffn  , &pop_af   , &ld_a_ffc, &di,
	/* 0xF4 */ &unset     , &push_af  , &or_a_n  , &rst_30,
	/* 0xF8 */ &ld_hl_spn , &ld_sp_hl , &ld_a_nn , &ei,
	/* 0xFC */ &unset     , &unset    , &cp_a_n  , &rst_38,
};

const struct cpu_instr *cpu_instr_ext[256] =
{
	/* 0x00 */ &rlc_b  , &rlc_c  , &rlc_d   , &rlc_e,
	/* 0x04 */ &rlc_h  , &rlc_l  , &rlc_hl  , &rlc_a,
	/* 0x08 */ &rrc_b  , &rrc_c  , &rrc_d   , &rrc_e,
	/* 0x0C */ &rrc_h  , &rrc_l  , &rrc_hl  , &rrc_a,
	/* 0x10 */ &rl_b   , &rl_c   , &rl_d    , &rl_e,
	/* 0x14 */ &rl_h   , &rl_l   , &rl_hl   , &rl_a,
	/* 0x18 */ &rr_b   , &rr_c   , &rr_d    , &rr_e,
	/* 0x1C */ &rr_h   , &rr_l   , &rr_hl   , &rr_a,
	/* 0x20 */ &sla_b  , &sla_c  , &sla_d   , &sla_e,
	/* 0x24 */ &sla_h  , &sla_l  , &sla_hl  , &sla_a,
	/* 0x28 */ &sra_b  , &sra_c  , &sra_d   , &sra_e,
	/* 0x2C */ &sra_h  , &sra_l  , &sra_hl  , &sra_a,
	/* 0x30 */ &swap_b , &swap_c , &swap_d  , &swap_e,
	/* 0x34 */ &swap_h , &swap_l , &swap_hl , &swap_a,
	/* 0x38 */ &srl_b  , &srl_c  , &srl_d   , &srl_e,
	/* 0x3C */ &srl_h  , &srl_l  , &srl_hl  , &srl_a,
	/* 0x40 */ &bit_0_b, &bit_0_c, &bit_0_d , &bit_0_e,
	/* 0x44 */ &bit_0_h, &bit_0_l, &bit_0_hl, &bit_0_a,
	/* 0x48 */ &bit_1_b, &bit_1_c, &bit_1_d , &bit_1_e,
	/* 0x4C */ &bit_1_h, &bit_1_l, &bit_1_hl, &bit_1_a,
	/* 0x50 */ &bit_2_b, &bit_2_c, &bit_2_d , &bit_2_e,
	/* 0x54 */ &bit_2_h, &bit_2_l, &bit_2_hl, &bit_2_a,
	/* 0x58 */ &bit_3_b, &bit_3_c, &bit_3_d , &bit_3_e,
	/* 0x5C */ &bit_3_h, &bit_3_l, &bit_3_hl, &bit_3_a,
	/* 0x60 */ &bit_4_b, &bit_4_c, &bit_4_d , &bit_4_e,
	/* 0x64 */ &bit_4_h, &bit_4_l, &bit_4_hl, &bit_4_a,
	/* 0x68 */ &bit_5_b, &bit_5_c, &bit_5_d , &bit_5_e,
	/* 0x6C */ &bit_5_h, &bit_5_l, &bit_5_hl, &bit_5_a,
	/* 0x70 */ &bit_6_b, &bit_6_c, &bit_6_d , &bit_6_e,
	/* 0x74 */ &bit_6_h, &bit_6_l, &bit_6_hl, &bit_6_a,
	/* 0x78 */ &bit_7_b, &bit_7_c, &bit_7_d , &bit_7_e,
	/* 0x7C */ &bit_7_h, &bit_7_l, &bit_7_hl, &bit_7_a,
	/* 0x80 */ &res_0_b, &res_0_c, &res_0_d , &res_0_e,
	/* 0x84 */ &res_0_h, &res_0_l, &res_0_hl, &res_0_a,
	/* 0x88 */ &res_1_b, &res_1_c, &res_1_d , &res_1_e,
	/* 0x8C */ &res_1_h, &res_1_l, &res_1_hl, &res_1_a,
	/* 0x90 */ &res_2_b, &res_2_c, &res_2_d , &res_2_e,
	/* 0x94 */ &res_2_h, &res_2_l, &res_2_hl, &res_2_a,
	/* 0x98 */ &res_3_b, &res_3_c, &res_3_d , &res_3_e,
	/* 0x9C */ &res_3_h, &res_3_l, &res_3_hl, &res_3_a,
	/* 0xA0 */ &res_4_b, &res_4_c, &res_4_d , &res_4_e,
	/* 0xA4 */ &res_4_h, &res_4_l, &res_4_hl, &res_4_a,
	/* 0xA8 */ &res_5_b, &res_5_c, &res_5_d , &res_5_e,
	/* 0xAC */ &res_5_h, &res_5_l, &res_5_hl, &res_5_a,
	/* 0xB0 */ &res_6_b, &res_6_c, &res_6_d , &res_6_e,
	/* 0xB4 */ &res_6_h, &res_6_l, &res_6_hl, &res_6_a,
	/* 0xB8 */ &res_7_b, &res_7_c, &res_7_d , &res_7_e,
	/* 0xBC */ &res_7_h, &res_7_l, &res_7_hl, &res_7_a,
	/* 0xC0 */ &set_0_b, &set_0_c, &set_0_d , &set_0_e,
	/* 0xC4 */ &set_0_h, &set_0_l, &set_0_hl, &set_0_a,
	/* 0xC8 */ &set_1_b, &set_1_c, &set_1_d , &set_1_e,
	/* 0xCC */ &set_1_h, &set_1_l, &set_1_hl, &set_1_a,
	/* 0xD0 */ &set_2_b, &set_2_c, &set_2_d , &set_2_e,
	/* 0xD4 */ &set_2_h, &set_2_l, &set_2_hl, &set_2_a,
	/* 0xD8 */ &set_3_b, &set_3_c, &set_3_d , &set_3_e,
	/* 0xDC */ &set_3_h, &set_3_l, &set_3_hl, &set_3_a,
	/* 0xE0 */ &set_4_b, &set_4_c, &set_4_d , &set_4_e,
	/* 0xE4 */ &set_4_h, &set_4_l, &set_4_hl, &set_4_a,
	/* 0xE8 */ &set_5_b, &set_5_c, &set_5_d , &set_5_e,
	/* 0xEC */ &set_5_h, &set_5_l, &set_5_hl, &set_5_a,
	/* 0xF0 */ &set_6_b, &set_6_c, &set_6_d , &set_6_e,
	/* 0xF4 */ &set_6_h, &set_6_l, &set_6_hl, &set_6_a,
	/* 0xF8 */ &set_7_b, &set_7_c, &set_7_d , &set_7_e,
	/* 0xFC */ &set_7_h, &set_7_l, &set_7_hl, &set_7_a,
};

const struct cpu_instr *cpu_instr_int[5] =
{
	&int_40, &int_48, &int_50, &int_58, &int_60,
};
