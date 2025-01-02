#include "cpu/instr.h"
#include "cpu.h"
#include "mem.h"

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct cpu *cpu_new(struct mem *mem)
{
	struct cpu *cpu = calloc(sizeof(*cpu), 1);
	if (!cpu)
	{
		fprintf(stderr, "allocation failed\n");
		return NULL;
	}

	cpu->mem = mem;
	return cpu;
}

void cpu_del(struct cpu *cpu)
{
	if (!cpu)
		return;

	free(cpu);
}

static void print_instr(struct cpu *cpu)
{
	char tmp[1024] = "";

	if (cpu->instr->print)
	{
		tmp[0] = ' ';
		cpu->instr->print(cpu, tmp + 1, sizeof(tmp) - 1);
	}

	fprintf(stderr, "[A=%02x B=%02x C=%02x D=%02x E=%02x H=%02x L=%02x PC=%04x SP=%04x IME=%d %c%c%c%c]%s\n",
	        cpu->regs.a,
	        cpu->regs.b,
	        cpu->regs.c,
	        cpu->regs.d,
	        cpu->regs.e,
	        cpu->regs.h,
	        cpu->regs.l,
	        cpu->regs.pc,
	        cpu->regs.sp,
	        cpu->ime ? 1 : 0,
	        cpu->regs.f & CPU_FLAG_Z ? 'Z' : '-',
	        cpu->regs.f & CPU_FLAG_N ? 'N' : '-',
	        cpu->regs.f & CPU_FLAG_H ? 'H' : '-',
	        cpu->regs.f & CPU_FLAG_C ? 'C' : '-',
	        tmp);
}

static void next_instruction(struct cpu *cpu)
{
	uint8_t opcode = mem_get(cpu->mem, cpu->regs.pc);
	if (opcode == 0xCB)
		cpu->instr = cpu_instr_ext[mem_get(cpu->mem, cpu->regs.pc + 1)];
	else
		cpu->instr = cpu_instr[opcode];

	cpu->instr_cycle = 0;
	if (cpu->debug)
		print_instr(cpu);
}

static bool handle_interrupt(struct cpu *cpu)
{
	uint8_t reg_if = mem_get_reg(cpu->mem, MEM_REG_IF);
	if (!reg_if)
		return false;
	uint8_t ie = mem_get_reg(cpu->mem, MEM_REG_IE);
	uint8_t v = ie & reg_if;
	if (!v)
		return false;
	cpu->state = CPU_RUN;
	if (!cpu->ime)
		return false;
	for (uint8_t i = 0; i < 5; ++i)
	{
		uint8_t mask = 1 << i;
		if (!(v & mask))
			continue;
		mem_set_reg(cpu->mem, MEM_REG_IF, reg_if & ~mask);
		cpu->instr = cpu_instr_int[i];
		cpu->instr_cycle = 0;
		return true;
	}
	return false;
}

static void cpu_cycle(struct cpu *cpu)
{
	if (!cpu->instr)
		next_instruction(cpu);

	if (cpu->hdma_wait)
	{
		cpu->hdma_wait--;
		return;
	}

	if (cpu->state == CPU_HALT)
	{
		if (!handle_interrupt(cpu))
			return;
		cpu->state = CPU_RUN;
	}

	if (cpu->state == CPU_STOP)
		return;

	bool end = cpu->instr->exec(cpu);
	if (end)
	{
		switch (cpu->ei)
		{
			case 0:
				break;
			case 1:
				cpu->ime = true;
				/* FALLTHROUGH */
			default:
				cpu->ei--;
				break;
		}
		if (!handle_interrupt(cpu))
			next_instruction(cpu);
	}
	else
	{
		cpu->instr_cycle++;
	}

	if (cpu->mem->dmatransfer != 0xFF)
	{
		mem_dmatransfer(cpu->mem);
		cpu->mem->dmatransfer--;
	}
}

void cpu_clock(struct cpu *cpu)
{
	if (++cpu->clock_count == 4)
	{
		cpu_cycle(cpu);
		cpu->clock_count = 0;
	}
}
