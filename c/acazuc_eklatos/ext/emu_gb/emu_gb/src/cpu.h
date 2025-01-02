#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include <stdint.h>

#define CPU_GET_FLAG(cpu, flag) (((cpu)->regs.f & flag) ? 1 : 0)
#define CPU_GET_FLAG_Z(cpu) CPU_GET_FLAG(cpu, CPU_FLAG_Z)
#define CPU_GET_FLAG_N(cpu) CPU_GET_FLAG(cpu, CPU_FLAG_N)
#define CPU_GET_FLAG_H(cpu) CPU_GET_FLAG(cpu, CPU_FLAG_H)
#define CPU_GET_FLAG_C(cpu) CPU_GET_FLAG(cpu, CPU_FLAG_C)

#define CPU_SET_FLAG(cpu, flag, v) \
do \
{ \
	if (v) \
		cpu->regs.f |=  flag; \
	else \
		cpu->regs.f &= ~flag; \
} while (0)

#define CPU_SET_FLAG_Z(cpu, v) CPU_SET_FLAG(cpu, CPU_FLAG_Z, v)
#define CPU_SET_FLAG_N(cpu, v) CPU_SET_FLAG(cpu, CPU_FLAG_N, v)
#define CPU_SET_FLAG_H(cpu, v) CPU_SET_FLAG(cpu, CPU_FLAG_H, v)
#define CPU_SET_FLAG_C(cpu, v) CPU_SET_FLAG(cpu, CPU_FLAG_C, v)

enum cpu_flag
{
	CPU_FLAG_Z = (1 << 7),
	CPU_FLAG_N = (1 << 6),
	CPU_FLAG_H = (1 << 5),
	CPU_FLAG_C = (1 << 4),
};

union cpu_regs
{
	struct
	{
		uint16_t pc;
		uint16_t sp;
		uint16_t af;
		uint16_t bc;
		uint16_t de;
		uint16_t hl;
	};
	struct
	{
		uint8_t pcl;
		uint8_t pch;
		uint8_t p;
		uint8_t s;
		uint8_t f;
		uint8_t a;
		uint8_t c;
		uint8_t b;
		uint8_t e;
		uint8_t d;
		uint8_t l;
		uint8_t h;
	};
};

struct cpu_instr;

enum cpu_state
{
	CPU_RUN,
	CPU_HALT,
	CPU_STOP,
};

struct cpu
{
	enum cpu_state state;
	union cpu_regs regs;
	uint32_t instr_cycle;
	uint32_t clock_count;
	union
	{
		uint8_t  u8[16];
		 int8_t  i8[16];
		uint16_t u16[8];
	} instr_tmp;
	const struct cpu_instr *instr;
	struct mem *mem;
	bool ime;
	uint8_t ei;
	uint8_t debug;
	uint16_t hdma_wait;
};

struct cpu *cpu_new(struct mem *mem);
void cpu_del(struct cpu *cpu);

void cpu_clock(struct cpu *cpu);

#endif
