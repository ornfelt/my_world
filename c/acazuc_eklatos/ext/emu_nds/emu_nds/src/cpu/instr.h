#ifndef CPU_INSTR_H
#define CPU_INSTR_H

#include <stdbool.h>
#include <stddef.h>

struct cpu;

struct cpu_instr
{
	void (*exec)(struct cpu *cpu);
	void (*print)(struct cpu *cpu, char *data, size_t size);
};

extern const struct cpu_instr *cpu_instr_thumb[0x400];
extern const struct cpu_instr *cpu_instr_arm[0x1000];

extern const struct cpu_instr *cpu_instr_blx_imm;

#endif
