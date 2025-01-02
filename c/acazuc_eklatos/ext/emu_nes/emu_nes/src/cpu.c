#include "internal.h"
#include "cpu/instr.h"
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

void cpu_init(struct cpu *cpu, struct nes *nes)
{
	cpu->nes = nes;
	cpu->reset = 1;
}

static void print_instr(struct cpu *cpu)
{
	char tmp[1024] = "";

	if ((cpu->debug & CPU_DEBUG_INSTR) && cpu->instr->print)
	{
		tmp[0] = ' ';
		cpu->instr->print(cpu, tmp + 1, sizeof(tmp) - 1);
	}

	printf("[OP=%02" PRIx8 " A=%02" PRIx8 " X=%02" PRIx8 " Y=%02" PRIx8
	       " S=%02" PRIx8 " PC=%04" PRIx16 " P=%02" PRIx8 " %c%c%c%c%c%c%c]%s\n",
	       cpu->instr_opcode, cpu->regs.a, cpu->regs.x, cpu->regs.y, cpu->regs.s,
	       (uint16_t)(cpu->regs.pc - 1), cpu->regs.p,
	       CPU_GET_FLAG_C(cpu) ? 'C' : '-',
	       CPU_GET_FLAG_Z(cpu) ? 'Z' : '-',
	       CPU_GET_FLAG_I(cpu) ? 'I' : '-',
	       CPU_GET_FLAG_D(cpu) ? 'D' : '-',
	       CPU_GET_FLAG_B(cpu) ? 'B' : '-',
	       CPU_GET_FLAG_V(cpu) ? 'V' : '-',
	       CPU_GET_FLAG_N(cpu) ? 'N' : '-',
	       tmp
	       );
}

static void cpu_cycle(struct cpu *cpu)
{
	if (cpu->instr_delay)
	{
		cpu->instr_delay--;
		return;
	}
	if (cpu->reset)
	{
		cpu->instr = special_instr[2];
		cpu->reset = 0;
		cpu->instr_opcode = 0;
	}
	else if (cpu->nmi)
	{
		cpu->instr = special_instr[1];
		cpu->nmi = 0;
		cpu->instr_opcode = 0;
	}
	else if (cpu->irq)
	{
		cpu->instr = special_instr[0];
		cpu->irq = 0;
		cpu->instr_opcode = 0;
	}
	else
	{
		cpu->instr_opcode = cpu_fetch8(cpu);
		cpu->instr = cpu_instr[cpu->instr_opcode];
		if (!cpu->instr)
		{
			printf("unknown instruction %" PRIx8 "\n", cpu->instr_opcode);
			return;
		}
	}
	if (cpu->debug)
		print_instr(cpu);
	cpu->instr->exec(cpu);
}

void cpu_clock(struct cpu *cpu)
{
	/* 16 in pal, 12 in NTSC */
	if (++cpu->clock_count == 12)
	{
		cpu_cycle(cpu);
		cpu->clock_count = 0;
	}
}
