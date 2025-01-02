#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include <stdint.h>

struct cpu_instr;
struct mem;

#define CPU_DEBUG_BASE    (1 << 0) /* print instr name */
#define CPU_DEBUG_INSTR   (1 << 1) /* print disassembled instruction */
#define CPU_DEBUG_REGS    (1 << 2) /* print registers */
#define CPU_DEBUG_REGS_ML (1 << 3) /* multiline registers dump */
#define CPU_DEBUG_ALL     (CPU_DEBUG_BASE | CPU_DEBUG_INSTR | CPU_DEBUG_REGS)

#define CPU_FLAG_N (1 << 31)
#define CPU_FLAG_Z (1 << 30)
#define CPU_FLAG_C (1 << 29)
#define CPU_FLAG_V (1 << 28)
#define CPU_FLAG_Q (1 << 27)
#define CPU_FLAG_I (1 << 7)
#define CPU_FLAG_F (1 << 6)
#define CPU_FLAG_T (1 << 5)

#define CPU_GET_FLAG(cpu, f) (((cpu)->regs.cpsr & (f)) ? 1 : 0)
#define CPU_GET_FLAG_N(cpu) CPU_GET_FLAG(cpu, CPU_FLAG_N)
#define CPU_GET_FLAG_Z(cpu) CPU_GET_FLAG(cpu, CPU_FLAG_Z)
#define CPU_GET_FLAG_C(cpu) CPU_GET_FLAG(cpu, CPU_FLAG_C)
#define CPU_GET_FLAG_V(cpu) CPU_GET_FLAG(cpu, CPU_FLAG_V)
#define CPU_GET_FLAG_Q(cpu) CPU_GET_FLAG(cpu, CPU_FLAG_Q)
#define CPU_GET_FLAG_I(cpu) CPU_GET_FLAG(cpu, CPU_FLAG_I)
#define CPU_GET_FLAG_F(cpu) CPU_GET_FLAG(cpu, CPU_FLAG_F)
#define CPU_GET_FLAG_T(cpu) CPU_GET_FLAG(cpu, CPU_FLAG_T)

#define CPU_SET_FLAG(cpu, f, v) \
do \
{ \
	if (v) \
		(cpu)->regs.cpsr |= f; \
	else \
		(cpu)->regs.cpsr &= ~f; \
} while (0)
#define CPU_SET_FLAG_N(cpu, v) CPU_SET_FLAG(cpu, CPU_FLAG_N, v)
#define CPU_SET_FLAG_Z(cpu, v) CPU_SET_FLAG(cpu, CPU_FLAG_Z, v)
#define CPU_SET_FLAG_C(cpu, v) CPU_SET_FLAG(cpu, CPU_FLAG_C, v)
#define CPU_SET_FLAG_V(cpu, v) CPU_SET_FLAG(cpu, CPU_FLAG_V, v)
#define CPU_SET_FLAG_Q(cpu, v) CPU_SET_FLAG(cpu, CPU_FLAG_Q, v)
#define CPU_SET_FLAG_I(cpu, v) CPU_SET_FLAG(cpu, CPU_FLAG_I, v)
#define CPU_SET_FLAG_F(cpu, v) CPU_SET_FLAG(cpu, CPU_FLAG_F, v)
#define CPU_SET_FLAG_T(cpu, v) CPU_SET_FLAG(cpu, CPU_FLAG_T, v)

#define CPU_GET_MODE(cpu) ((cpu)->regs.cpsr & 0x1F)
#define CPU_SET_MODE(cpu, mode) (cpu)->regs.cpsr = ((cpu)->regs.cpsr & 0xFFFFFFE0) | (mode)

#define CPU_MODE_USR 0x10
#define CPU_MODE_FIQ 0x11
#define CPU_MODE_IRQ 0x12
#define CPU_MODE_SVC 0x13
#define CPU_MODE_ABT 0x17
#define CPU_MODE_UND 0x1B
#define CPU_MODE_SYS 0x1F

#define CPU_REG_SP 0xD
#define CPU_REG_LR 0xE
#define CPU_REG_PC 0xF

struct cpu_regs
{
	uint32_t r[16];
	uint32_t r_fiq[7];
	uint32_t r_svc[2];
	uint32_t r_abt[2];
	uint32_t r_irq[2];
	uint32_t r_und[2];
	uint32_t cpsr;
	uint32_t spsr_modes[5];
	uint32_t *rptr[16];
	uint32_t *spsr;
};

enum cpu_state
{
	CPU_STATE_RUN,
	CPU_STATE_HALT,
	CPU_STATE_STOP,
};

struct cpu
{
	struct cpu_regs regs;
	struct mem *mem;
	const struct cpu_instr *instr;
	uint32_t last_bios_decode;
	uint32_t instr_opcode;
	uint32_t instr_delay;
	uint8_t debug;
	enum cpu_state state;
};

struct cpu *cpu_new(struct mem *mem);
void cpu_del(struct cpu *cpu);

void cpu_cycle(struct cpu *cpu);
void cpu_update_mode(struct cpu *cpu);

static inline uint32_t cpu_get_reg(struct cpu *cpu, uint32_t reg)
{
	return *cpu->regs.rptr[reg];
}

static inline void cpu_set_reg(struct cpu *cpu, uint32_t reg, uint32_t v)
{
	*cpu->regs.rptr[reg] = v;
}

static inline void cpu_inc_pc(struct cpu *cpu, uint32_t v)
{
	*cpu->regs.rptr[15] += v;
}

#endif
