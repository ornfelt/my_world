#include "cpu.h"
#include "mem.h"
#include "nds.h"
#include "cpu/instr.h"

#include <inttypes.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

struct cpu *cpu_new(struct mem *mem, int arm9)
{
	struct cpu *cpu = calloc(sizeof(*cpu), 1);
	if (!cpu)
		return NULL;

	cpu->arm9 = arm9;
	if (arm9)
	{
		cpu->get8 = mem_arm9_get8;
		cpu->get16 = mem_arm9_get16;
		cpu->get32 = mem_arm9_get32;
		cpu->set8 = mem_arm9_set8;
		cpu->set16 = mem_arm9_set16;
		cpu->set32 = mem_arm9_set32;
		cpu->cp15.midr = 0x41059461;
		cpu->cp15.ctr = 0x0F0D2112;
		cpu->cp15.tcmsr = 0x00140180;
		cpu->cp15.cr = 0x00002078;
	}
	else
	{
		cpu->get8 = mem_arm7_get8;
		cpu->get16 = mem_arm7_get16;
		cpu->get32 = mem_arm7_get32;
		cpu->set8 = mem_arm7_set8;
		cpu->set16 = mem_arm7_set16;
		cpu->set32 = mem_arm7_set32;
	}
	cpu->mem = mem;
	cpu->regs.cpsr = 0xD3;
	cpu_update_mode(cpu);
	cpu_set_reg(cpu, CPU_REG_PC, (cpu->cp15.cr & 0x2000) ? 0xFFFF0000UL : 0);
	return cpu;
}

void cpu_del(struct cpu *cpu)
{
	if (!cpu)
		return;
	free(cpu);
}

static bool check_arm_cond(struct cpu *cpu, uint32_t cond)
{
	switch (cond & 0xF)
	{
		case 0x0:
			return CPU_GET_FLAG_Z(cpu);
		case 0x1:
			return !CPU_GET_FLAG_Z(cpu);
		case 0x2:
			return CPU_GET_FLAG_C(cpu);
		case 0x3:
			return !CPU_GET_FLAG_C(cpu);
		case 0x4:
			return CPU_GET_FLAG_N(cpu);
		case 0x5:
			return !CPU_GET_FLAG_N(cpu);
		case 0x6:
			return CPU_GET_FLAG_V(cpu);
		case 0x7:
			return !CPU_GET_FLAG_V(cpu);
		case 0x8:
			return CPU_GET_FLAG_C(cpu) && !CPU_GET_FLAG_Z(cpu);
		case 0x9:
			return !CPU_GET_FLAG_C(cpu) || CPU_GET_FLAG_Z(cpu);
		case 0xA:
			return CPU_GET_FLAG_N(cpu) == CPU_GET_FLAG_V(cpu);
		case 0xB:
			return CPU_GET_FLAG_N(cpu) != CPU_GET_FLAG_V(cpu);
		case 0xC:
			return !CPU_GET_FLAG_Z(cpu) && CPU_GET_FLAG_N(cpu) == CPU_GET_FLAG_V(cpu);
		case 0xD:
			return CPU_GET_FLAG_Z(cpu) || CPU_GET_FLAG_N(cpu) != CPU_GET_FLAG_V(cpu);
		case 0xE:
			return true;
		case 0xF:
			return false;
	}
	/* unreachable */
	return false;
}

static void print_regs(struct cpu *cpu)
{
	printf("r00=%08" PRIx32 " r01=%08" PRIx32 " r02=%08" PRIx32 " r03=%08" PRIx32 " "
	       "r04=%08" PRIx32 " r05=%08" PRIx32 " r06=%08" PRIx32 " r07=%08" PRIx32 " "
	       "r08=%08" PRIx32 " r09=%08" PRIx32 " r10=%08" PRIx32 " r11=%08" PRIx32 " "
	       "r12=%08" PRIx32 " r13=%08" PRIx32 " r14=%08" PRIx32 " r15=%08" PRIx32 "\n",
	       cpu_get_reg(cpu, 0x0),
	       cpu_get_reg(cpu, 0x1),
	       cpu_get_reg(cpu, 0x2),
	       cpu_get_reg(cpu, 0x3),
	       cpu_get_reg(cpu, 0x4),
	       cpu_get_reg(cpu, 0x5),
	       cpu_get_reg(cpu, 0x6),
	       cpu_get_reg(cpu, 0x7),
	       cpu_get_reg(cpu, 0x8),
	       cpu_get_reg(cpu, 0x9),
	       cpu_get_reg(cpu, 0xA),
	       cpu_get_reg(cpu, 0xB),
	       cpu_get_reg(cpu, 0xC),
	       cpu_get_reg(cpu, 0xD),
	       cpu_get_reg(cpu, 0xE),
	       cpu_get_reg(cpu, 0xF));
}

static void print_regs_ml(struct cpu *cpu)
{
	printf("r00=%08" PRIx32 " r01=%08" PRIx32 " r02=%08" PRIx32 " r03=%08" PRIx32 "\n"
	       "r04=%08" PRIx32 " r05=%08" PRIx32 " r06=%08" PRIx32 " r07=%08" PRIx32 "\n"
	       "r08=%08" PRIx32 " r09=%08" PRIx32 " r10=%08" PRIx32 " r11=%08" PRIx32 "\n"
	       "r12=%08" PRIx32 " r13=%08" PRIx32 " r14=%08" PRIx32 " r15=%08" PRIx32 "\n",
	       cpu_get_reg(cpu, 0x0),
	       cpu_get_reg(cpu, 0x1),
	       cpu_get_reg(cpu, 0x2),
	       cpu_get_reg(cpu, 0x3),
	       cpu_get_reg(cpu, 0x4),
	       cpu_get_reg(cpu, 0x5),
	       cpu_get_reg(cpu, 0x6),
	       cpu_get_reg(cpu, 0x7),
	       cpu_get_reg(cpu, 0x8),
	       cpu_get_reg(cpu, 0x9),
	       cpu_get_reg(cpu, 0xA),
	       cpu_get_reg(cpu, 0xB),
	       cpu_get_reg(cpu, 0xC),
	       cpu_get_reg(cpu, 0xD),
	       cpu_get_reg(cpu, 0xE),
	       cpu_get_reg(cpu, 0xF));
}

static void print_instr(struct cpu *cpu, const char *msg, const struct cpu_instr *instr)
{
	char tmp[1024] = "";

	if ((cpu->debug & CPU_DEBUG_INSTR) && instr->print)
	{
		tmp[0] = ' ';
		instr->print(cpu, tmp + 1, sizeof(tmp) - 1);
	}

	printf("[%s] [%-4s] [%08" PRIx32 "] [%08" PRIx32 "] [%08" PRIx32 "]%s\n",
	        cpu->arm9 ? "ARM9" : "ARM7",
	        msg,
	        cpu->regs.cpsr,
	        *cpu->regs.spsr,
	        cpu->instr_opcode,
	        tmp
	        );

	if (cpu->debug & CPU_DEBUG_REGS)
	{
		if (cpu->debug & CPU_DEBUG_REGS_ML)
			print_regs_ml(cpu);
		else
			print_regs(cpu);
		fflush(stdout);
	}
}

static bool handle_interrupt(struct cpu *cpu)
{
	if (!cpu->irq_line)
		return false;
	if (cpu->state == CPU_STATE_RUN && CPU_GET_FLAG_I(cpu))
		return false;
	if (cpu->arm9)
	{
		if (!mem_arm9_get_reg16(cpu->mem, MEM_ARM9_REG_IME))
			return false;
	}
	else
	{
		if (cpu->state == CPU_STATE_RUN
		 && !mem_arm7_get_reg16(cpu->mem, MEM_ARM7_REG_IME))
			return false;
	}
	cpu->state = CPU_STATE_RUN;
	for (uint8_t i = 0; i < 32; ++i)
	{
		if (!(cpu->irq_line & (1 << i)))
			continue;
#if 0
		printf("[ARM%c] IRQ %" PRIu8 "\n", cpu->arm9 ? '9' : '7', i);
#endif
		cpu->regs.spsr_modes[3] = cpu->regs.cpsr;
		CPU_SET_MODE(cpu, CPU_MODE_IRQ);
		cpu_update_mode(cpu);
		CPU_SET_FLAG_I(cpu, 1);
		if (CPU_GET_FLAG_T(cpu))
		{
			CPU_SET_FLAG_T(cpu, 0);
			cpu_set_reg(cpu, CPU_REG_LR, cpu_get_reg(cpu, CPU_REG_PC) + 4);
		}
		else
		{
			cpu_set_reg(cpu, CPU_REG_LR, cpu_get_reg(cpu, CPU_REG_PC) + 4);
		}
		if (cpu->cp15.cr & 0x2000)
			cpu_set_reg(cpu, CPU_REG_PC, 0xFFFF0018UL);
		else
			cpu_set_reg(cpu, CPU_REG_PC, 0x18);
		return true;
	}
	return false;
}

void cpu_update_irq_state(struct cpu *cpu)
{
	uint32_t reg_if;
	uint32_t reg_ie;
	if (cpu->arm9)
	{
		reg_if = mem_arm9_get_reg32(cpu->mem, MEM_ARM9_REG_IF);
		reg_ie = mem_arm9_get_reg32(cpu->mem, MEM_ARM9_REG_IE);
	}
	else
	{
		reg_if = mem_arm7_get_reg32(cpu->mem, MEM_ARM7_REG_IF);
		reg_ie = mem_arm7_get_reg32(cpu->mem, MEM_ARM7_REG_IE);
	}
	cpu->irq_line = reg_ie & reg_if;
	if (cpu->state == CPU_STATE_RUN)
		cpu->irq_wait = 0;
	else
		cpu->irq_wait = !cpu->irq_line;
#if 0
	printf("[ARM%c] updated IRQ state: IE=%08" PRIx32 " IF=%08" PRIx32 " -> LINE=%08" PRIx32 " WAIT=%d\n",
	       cpu->arm9 ? '9' : '7', reg_ie, reg_if, cpu->irq_line, cpu->irq_wait);
#endif
}

static bool decode_instruction(struct cpu *cpu)
{
	if (CPU_GET_FLAG_T(cpu))
	{
		if (cpu->has_next_thumb)
		{
			cpu->instr_opcode = cpu->next_thumb;
			cpu->has_next_thumb = 0;
		}
		else
		{
			uint32_t pc = cpu_get_reg(cpu, CPU_REG_PC);
			if (pc & 2)
			{
				cpu->instr_opcode = cpu->get16(cpu->mem, pc, cpu->arm9 ? MEM_CODE_NSEQ : MEM_CODE_SEQ);
			}
			else
			{
				uint32_t fetch = cpu->get32(cpu->mem, pc, cpu->arm9 ? MEM_CODE_NSEQ : MEM_CODE_SEQ);
				cpu->instr_opcode = fetch & 0xFFFF;
				cpu->has_next_thumb = 1;
				cpu->next_thumb = (fetch >> 16) & 0xFFFF;
			}
		}
		cpu->instr = cpu_instr_thumb[cpu->instr_opcode >> 6];
	}
	else
	{
		cpu->has_next_thumb = 0;
		uint32_t pc = cpu_get_reg(cpu, CPU_REG_PC);
		cpu->instr_opcode = cpu->get32(cpu->mem, pc, cpu->arm9 ? MEM_CODE_NSEQ : MEM_CODE_SEQ);
		if (cpu->instr_opcode >> 25 == 0x7D) /* come on arm ISA.... wtf ? */
		{
			cpu->instr = cpu_instr_blx_imm;
		}
		else
		{
			if (!check_arm_cond(cpu, cpu->instr_opcode >> 28))
			{
				if (cpu->debug)
					print_instr(cpu, "SKIP", cpu_instr_arm[((cpu->instr_opcode >> 16) & 0xFF0) | ((cpu->instr_opcode >> 4) & 0xF)]);
				cpu_inc_pc(cpu, 4);
				cpu->instr = NULL;
				return false;
			}
			cpu->instr = cpu_instr_arm[((cpu->instr_opcode >> 16) & 0xFF0) | ((cpu->instr_opcode >> 4) & 0xF)];
		}
	}

	return true;
}

void cpu_cycle(struct cpu *cpu)
{
#if 0
	if (cpu_get_reg(cpu, CPU_REG_PC) == 0x02400020)
		cpu->debug = CPU_DEBUG_ALL_ML;
#endif

	handle_interrupt(cpu);
	if (cpu->state != CPU_STATE_RUN)
		return;
	if (!decode_instruction(cpu))
		return;
	if (cpu->debug)
		print_instr(cpu, "EXEC", cpu->instr);
	cpu->instr->exec(cpu);
}

void cpu_update_mode(struct cpu *cpu)
{
	for (size_t i = 0; i < 16; ++i)
		cpu->regs.rptr[i] = &cpu->regs.r[i];
	switch (CPU_GET_MODE(cpu))
	{
		case CPU_MODE_USR:
		case CPU_MODE_SYS:
			cpu->regs.spsr = &cpu->regs.cpsr;
			break;
		case CPU_MODE_FIQ:
			for (size_t i = 8; i < 15; ++i)
				cpu->regs.rptr[i] = &cpu->regs.r_fiq[i - 8];
			cpu->regs.spsr = &cpu->regs.spsr_modes[0];
			break;
		case CPU_MODE_SVC:
			for (size_t i = 13; i < 15; ++i)
				cpu->regs.rptr[i] = &cpu->regs.r_svc[i - 13];
			cpu->regs.spsr = &cpu->regs.spsr_modes[1];
			break;
		case CPU_MODE_ABT:
			for (size_t i = 13; i < 15; ++i)
				cpu->regs.rptr[i] = &cpu->regs.r_abt[i - 13];
			cpu->regs.spsr = &cpu->regs.spsr_modes[2];
			break;
		case CPU_MODE_IRQ:
			for (size_t i = 13; i < 15; ++i)
				cpu->regs.rptr[i] = &cpu->regs.r_irq[i - 13];
			cpu->regs.spsr = &cpu->regs.spsr_modes[3];
			break;
		case CPU_MODE_UND:
			for (size_t i = 13; i < 15; ++i)
				cpu->regs.rptr[i] = &cpu->regs.r_und[i - 13];
			cpu->regs.spsr = &cpu->regs.spsr_modes[4];
			break;
		default:
			printf("unknown mode: %" PRIx32 "\n", CPU_GET_MODE(cpu));
			assert(!"invalid mode");
	}
}

uint32_t cp15_read(struct cpu *cpu, uint8_t cn, uint8_t cm, uint8_t cp)
{
#if 0
	printf("cp15[%" PRIx8 "%" PRIx8 "%" PRIx8 "] read\n", cn, cm, cp);
#endif
	switch ((cn << 8) | (cm << 4) | cp)
	{
		case 0x000:
			return cpu->cp15.midr;
		case 0x001:
			return cpu->cp15.ctr;
		case 0x002:
			return cpu->cp15.tcmsr;
		case 0x100:
			return cpu->cp15.cr;
		case 0x200:
			return cpu->cp15.dpr;
		case 0x201:
			return cpu->cp15.ipr;
		case 0x300:
			return cpu->cp15.wdpr;
		case 0x500:
			return cpu->cp15.apdpr;
		case 0x501:
			return cpu->cp15.apipr;
		case 0x502:
			return cpu->cp15.eapdpr;
		case 0x503:
			return cpu->cp15.eapipr;
		case 0x600:
			return cpu->cp15.pudr[0];
		case 0x610:
			return cpu->cp15.pudr[1];
		case 0x620:
			return cpu->cp15.pudr[2];
		case 0x630:
			return cpu->cp15.pudr[3];
		case 0x640:
			return cpu->cp15.pudr[4];
		case 0x650:
			return cpu->cp15.pudr[5];
		case 0x660:
			return cpu->cp15.pudr[6];
		case 0x670:
			return cpu->cp15.pudr[7];
		case 0x601:
			return cpu->cp15.puir[0];
		case 0x611:
			return cpu->cp15.puir[1];
		case 0x621:
			return cpu->cp15.puir[2];
		case 0x631:
			return cpu->cp15.puir[3];
		case 0x641:
			return cpu->cp15.puir[4];
		case 0x651:
			return cpu->cp15.puir[5];
		case 0x661:
			return cpu->cp15.puir[6];
		case 0x671:
			return cpu->cp15.puir[7];
		case 0x900:
			return cpu->cp15.dcl;
		case 0x901:
			return cpu->cp15.icl;
		case 0x910:
			return cpu->cp15.dtcm;
		case 0x911:
			return cpu->cp15.itcm;
		default:
			printf("unknown cp15 read reg: %" PRIx8 "%" PRIx8 "%" PRIx8 "\n", cn, cm, cp);
	}
	return 0;
}

static void update_itcm(struct cpu *cpu)
{
	if (!(cpu->cp15.cr & (1 << 18)))
	{
		cpu->mem->itcm_base = 0xFFFFFFFF;
		cpu->mem->itcm_mask = 0;
		return;
	}
	cpu->mem->itcm_base = 0;
	uint8_t size = (cpu->cp15.itcm & 0x3E) >> 1;
	if (size < 3)
		size = 3;
	if (size > 23)
		size = 23;
	cpu->mem->itcm_mask = (0x200 << size) - 1;
#if 0
	printf("itcm: 0x%08" PRIx32 " / 0x%08" PRIx32 "\n",
	       cpu->mem->itcm_base, cpu->mem->itcm_mask);
#endif
}

static void update_dtcm(struct cpu *cpu)
{
	if (!(cpu->cp15.cr & (1 << 16)))
	{
		cpu->mem->dtcm_base = 0xFFFFFFFF;
		cpu->mem->dtcm_mask = 0;
		return;
	}
	cpu->mem->dtcm_base = cpu->cp15.dtcm & 0xFFFFF000;
	uint8_t size = (cpu->cp15.dtcm & 0x3E) >> 1;
	if (size < 3)
		size = 3;
	if (size > 23)
		size = 23;
	cpu->mem->dtcm_mask = (0x200 << size) - 1;
#if 0
	printf("dtcm: 0x%08" PRIx32 " / 0x%08" PRIx32 "\n",
	       cpu->mem->dtcm_base, cpu->mem->dtcm_mask);
#endif
}

void cp15_write(struct cpu *cpu, uint8_t cn, uint8_t cm, uint8_t cp, uint32_t v)
{
#if 0
	printf("cp15[%" PRIx8 "%" PRIx8 "%" PRIx8 "] = %08" PRIx32 "\n", cn, cm, cp, v);
#endif
	switch ((cn << 8) | (cm << 4) | cp)
	{
		case 0x100:
			cpu->cp15.cr = (cpu->cp15.cr & ~0xFF085) | (v & 0xFF085);
			update_itcm(cpu);
			update_dtcm(cpu);
			break;
		case 0x200:
			cpu->cp15.dpr = v;
			break;
		case 0x201:
			cpu->cp15.ipr = v;
			break;
		case 0x300:
			cpu->cp15.wdpr = v;
			break;
		case 0x500:
			cpu->cp15.apdpr = v;
			break;
		case 0x501:
			cpu->cp15.apipr = v;
			break;
		case 0x502:
			cpu->cp15.eapdpr = v;
			break;
		case 0x503:
			cpu->cp15.eapipr = v;
			break;
		case 0x600:
			cpu->cp15.pudr[0] = v;
			break;
		case 0x610:
			cpu->cp15.pudr[1] = v;
			break;
		case 0x620:
			cpu->cp15.pudr[2] = v;
			break;
		case 0x630:
			cpu->cp15.pudr[3] = v;
			break;
		case 0x640:
			cpu->cp15.pudr[4] = v;
			break;
		case 0x650:
			cpu->cp15.pudr[5] = v;
			break;
		case 0x660:
			cpu->cp15.pudr[6] = v;
			break;
		case 0x670:
			cpu->cp15.pudr[7] = v;
			break;
		case 0x601:
			cpu->cp15.puir[0] = v;
			break;
		case 0x611:
			cpu->cp15.puir[1] = v;
			break;
		case 0x621:
			cpu->cp15.puir[2] = v;
			break;
		case 0x631:
			cpu->cp15.puir[3] = v;
			break;
		case 0x641:
			cpu->cp15.puir[4] = v;
			break;
		case 0x651:
			cpu->cp15.puir[5] = v;
			break;
		case 0x661:
			cpu->cp15.puir[6] = v;
			break;
		case 0x671:
			cpu->cp15.puir[7] = v;
			break;
		case 0x704:
		case 0x782:
			cpu->state = CPU_STATE_HALT;
			cpu_update_irq_state(cpu);
			return;
		case 0x750: /* invalidate instruction cache */
			return;
		case 0x751: /* invalidate instruction cache line VA */
			return;
		case 0x752: /* invalidate instruction cache line S/I */
			return;
		case 0x760: /* invalidate data cache */
			return;
		case 0x761: /* invalidate data cache line VA */
			return;
		case 0x762: /* invalidate data cache line S/I */
			return;
		case 0x7A1: /* clean data cache line VA */
			return;
		case 0x7A2: /* clean data cache line S/I */
			return;
		case 0x7A4: /* drain write buffer */
			return;
		case 0x7E1: /* clean and invalidate cache line VA */
			return;
		case 0x7E2: /* clean and invalidate cache line S/I */
			return;
		case 0x900:
			cpu->cp15.dcl = v;
			break;
		case 0x901:
			cpu->cp15.icl = v;
			break;
		case 0x910:
			cpu->cp15.dtcm = v;
			update_dtcm(cpu);
#if 0
			printf("DTCM 0x%08" PRIx32 " @ 0x%08" PRIx32 "\n",
			       (uint32_t)(0x200 << ((cpu->cp15.dtcm & 0x3E) >> 1)),
			       (uint32_t)(cpu->cp15.dtcm & 0xFFFFF000));
#endif
			break;
		case 0x911:
			cpu->cp15.itcm = v;
			update_itcm(cpu);
#if 0
			printf("ITCM 0x%08" PRIx32 " @ 0x%08" PRIx32 "\n",
			       (uint32_t)(0x200 << ((cpu->cp15.itcm & 0x3E) >> 1)),
			       (uint32_t)(cpu->cp15.itcm & 0xFFFFF000));
#endif
			break;
		default:
			printf("unknown cp15 write reg: %" PRIx8 "%" PRIx8 "%" PRIx8 ": %" PRIx32 "\n", cn, cm, cp, v);
	}
}
