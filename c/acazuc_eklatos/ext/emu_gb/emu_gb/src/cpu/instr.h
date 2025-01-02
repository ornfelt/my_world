#ifndef CPU_INSTR_H
#define CPU_INSTR_H

#include <stdbool.h>
#include <stddef.h>

struct cpu;

struct cpu_instr
{
	bool (*exec)(struct cpu *cpu);
	void (*print)(struct cpu *cpu, char *data, size_t size);
};

extern const struct cpu_instr *cpu_instr[256];
extern const struct cpu_instr *cpu_instr_ext[256];
extern const struct cpu_instr *cpu_instr_int[5];

#endif
