#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>

struct cpu_instr;
struct mem;

#define CPU_DEBUG_BASE    (1 << 0) /* print instr name */
#define CPU_DEBUG_INSTR   (1 << 1) /* print disassembled instruction */
#define CPU_DEBUG_REGS    (1 << 2) /* print registers */
#define CPU_DEBUG_REGS_ML (1 << 3) /* multiline registers dump */
#define CPU_DEBUG_ALL     (CPU_DEBUG_BASE | CPU_DEBUG_INSTR | CPU_DEBUG_REGS)
#define CPU_DEBUG_ALL_ML  (CPU_DEBUG_ALL | CPU_DEBUG_REGS_ML)

#define CPU_FLAG_N (1UL << 31)
#define CPU_FLAG_Z (1UL << 30)
#define CPU_FLAG_C (1UL << 29)
#define CPU_FLAG_V (1UL << 28)
#define CPU_FLAG_Q (1UL << 27)
#define CPU_FLAG_I (1UL << 7)
#define CPU_FLAG_F (1UL << 6)
#define CPU_FLAG_T (1UL << 5)

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
#define CPU_SET_MODE(cpu, mode) (cpu)->regs.cpsr = ((cpu)->regs.cpsr & 0xFFFFFFE0UL) | (mode)

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

enum mem_type;

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

struct cp15
{
	uint32_t midr;
	uint32_t ctr;
	uint32_t tcmsr;
	uint32_t cr;
	uint8_t dpr;
	uint8_t ipr;
	uint8_t wdpr;
	uint16_t apdpr;
	uint16_t apipr;
	uint32_t eapdpr;
	uint32_t eapipr;
	uint32_t pudr[8];
	uint32_t puir[8];
	uint32_t dcl;
	uint32_t icl;
	uint32_t dtcm;
	uint32_t itcm;
};

struct cpu
{
	uint8_t  (*get8 )(struct mem *mem, uint32_t addr, enum mem_type type);
	uint16_t (*get16)(struct mem *mem, uint32_t addr, enum mem_type type);
	uint32_t (*get32)(struct mem *mem, uint32_t addr, enum mem_type type);
	void (*set8 )(struct mem *mem, uint32_t addr, uint8_t val, enum mem_type type);
	void (*set16)(struct mem *mem, uint32_t addr, uint16_t val, enum mem_type type);
	void (*set32)(struct mem *mem, uint32_t addr, uint32_t val, enum mem_type type);
	struct cpu_regs regs;
	struct cp15 cp15;
	struct mem *mem;
	const struct cpu_instr *instr;
	uint32_t instr_opcode;
	int32_t instr_delay;
	uint8_t debug;
	enum cpu_state state;
	int arm9;
	int irq_wait;
	uint32_t irq_line;
	uint16_t next_thumb;
	int has_next_thumb;
};

struct cpu *cpu_new(struct mem *mem, int arm9);
void cpu_del(struct cpu *cpu);

void cpu_cycle(struct cpu *cpu);
void cpu_update_mode(struct cpu *cpu);
void cpu_update_irq_state(struct cpu *cpu);

uint32_t cp15_read(struct cpu *cpu, uint8_t cn, uint8_t cm, uint8_t cp);
void cp15_write(struct cpu *cpu, uint8_t cn, uint8_t cm, uint8_t cp, uint32_t v);

static inline uint32_t cpu_get_reg(struct cpu *cpu, uint32_t reg)
{
	return *cpu->regs.rptr[reg];
}

static inline void cpu_set_reg(struct cpu *cpu, uint32_t reg, uint32_t v)
{
	*cpu->regs.rptr[reg] = v;
	if (reg == CPU_REG_PC)
		cpu->has_next_thumb = 0;
}

static inline void cpu_inc_pc(struct cpu *cpu, uint32_t v)
{
	*cpu->regs.rptr[CPU_REG_PC] += v;
}

#endif
