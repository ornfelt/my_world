#ifndef INTERNAL_H
#define INTERNAL_H

#include <stdint.h>
#include <stdbool.h>

#include "nes.h"

#define MEM_REG_GPU_CR1          0x2000
#define MEM_REG_GPU_CR2          0x2001
#define MEM_REG_GPU_STATUS       0x2002
#define MEM_REG_GPU_SPR_RAM_ADDR 0x2003
#define MEM_REG_GPU_SPR_RAM_DATA 0x2004
#define MEM_REG_GPU_SCROLL       0x2005
#define MEM_REG_GPU_VRAM_ADDR    0x2006
#define MEM_REG_GPU_VRAM_DATA    0x2007

#define CPU_GET_FLAG(cpu, f) (((cpu)->regs.p & (f)) ? 1 : 0)
#define CPU_GET_FLAG_C(cpu) CPU_GET_FLAG(cpu, CPU_FLAG_C)
#define CPU_GET_FLAG_Z(cpu) CPU_GET_FLAG(cpu, CPU_FLAG_Z)
#define CPU_GET_FLAG_I(cpu) CPU_GET_FLAG(cpu, CPU_FLAG_I)
#define CPU_GET_FLAG_D(cpu) CPU_GET_FLAG(cpu, CPU_FLAG_D)
#define CPU_GET_FLAG_B(cpu) CPU_GET_FLAG(cpu, CPU_FLAG_B)
#define CPU_GET_FLAG_V(cpu) CPU_GET_FLAG(cpu, CPU_FLAG_V)
#define CPU_GET_FLAG_N(cpu) CPU_GET_FLAG(cpu, CPU_FLAG_N)

#define CPU_SET_FLAG(cpu, f, v) \
do \
{ \
	if (v) \
		(cpu)->regs.p |= (f); \
	else \
		(cpu)->regs.p &= ~(f); \
} while (0)

#define CPU_SET_FLAG_C(cpu, v) CPU_SET_FLAG(cpu, CPU_FLAG_C, v)
#define CPU_SET_FLAG_Z(cpu, v) CPU_SET_FLAG(cpu, CPU_FLAG_Z, v)
#define CPU_SET_FLAG_I(cpu, v) CPU_SET_FLAG(cpu, CPU_FLAG_I, v)
#define CPU_SET_FLAG_D(cpu, v) CPU_SET_FLAG(cpu, CPU_FLAG_D, v)
#define CPU_SET_FLAG_B(cpu, v) CPU_SET_FLAG(cpu, CPU_FLAG_B, v)
#define CPU_SET_FLAG_V(cpu, v) CPU_SET_FLAG(cpu, CPU_FLAG_V, v)
#define CPU_SET_FLAG_N(cpu, v) CPU_SET_FLAG(cpu, CPU_FLAG_N, v)

#define CPU_DEBUG_BASE    (1 << 0) /* print instr name */
#define CPU_DEBUG_INSTR   (1 << 1) /* print disassembled instruction */
#define CPU_DEBUG_REGS    (1 << 2) /* print registers */
#define CPU_DEBUG_ALL     (CPU_DEBUG_BASE | CPU_DEBUG_INSTR | CPU_DEBUG_REGS)

enum cpu_flag
{
	CPU_FLAG_C = (1 << 0),
	CPU_FLAG_Z = (1 << 1),
	CPU_FLAG_I = (1 << 2),
	CPU_FLAG_D = (1 << 3),
	CPU_FLAG_B = (1 << 4),
	CPU_FLAG_V = (1 << 6),
	CPU_FLAG_N = (1 << 7),
};

static const uint8_t palette_table[0x20] =
{
	0x00, 0x01, 0x02, 0x03,
	0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B,
	0x0C, 0x0D, 0x0E, 0x0F,
	0x00, 0x11, 0x12, 0x13,
	0x04, 0x15, 0x16, 0x17,
	0x08, 0x19, 0x1C, 0x1B,
	0x0C, 0x1D, 0x1E, 0x1F,
};

struct apu_wave
{
	uint8_t duty;
	uint32_t frequency;
	uint32_t counter;
	uint8_t value;
	uint8_t status;
	uint8_t length;
	uint8_t decay;
	uint8_t decay_count;
	uint8_t sweep;
	uint8_t sweep_count;
};

struct apu_triangle
{
	uint32_t frequency;
	uint32_t counter;
	uint8_t value;
	uint8_t length;
	uint8_t status;
	uint8_t linear;
	uint8_t reload;
};

struct apu_noise
{
	uint32_t counter;
	uint16_t value;
	uint8_t frequency;
	uint8_t status;
	uint8_t length;
	uint8_t decay;
	uint8_t decay_count;
};

struct apu
{
	struct nes *nes;
	uint8_t data[800];
	uint16_t sample_clock;
	uint16_t sample_count;
	uint16_t frame_clock;
	uint8_t clock_count;
	uint8_t status;
	uint8_t control;
	struct apu_wave wave1;
	struct apu_wave wave2;
	struct apu_triangle triangle;
	struct apu_noise noise;
};

struct cpu_regs
{
	uint8_t a;
	uint8_t x;
	uint8_t y;
	uint8_t s;
	uint16_t pc;
	uint8_t p;
};

struct cpu
{
	struct nes *nes;
	struct cpu_regs regs;
	uint8_t clock_count;
	uint16_t instr_delay;
	const struct cpu_instr *instr;
	uint8_t instr_opcode;
	uint8_t debug;
	uint8_t nmi;
	uint8_t irq;
	uint8_t reset;
};

struct gpu
{
	struct nes *nes;
	uint8_t data[256 * 240 * 4];
	uint8_t clock_count;
	uint16_t x;
	uint16_t y;
	uint8_t chr;
	uint8_t bg_v1;
	uint8_t bg_v2;
};

struct ines
{
	uint8_t magic[4];
	uint8_t prg_rom_lsb;
	uint8_t chr_rom_lsb;
	uint8_t flags6;
	uint8_t flags7;
	uint8_t mapper;
	uint8_t prg_chr_rom_msb;
	uint8_t prg_ram_size;
	uint8_t chr_ram_size;
	uint8_t timing;
	uint8_t ext_type;
	uint8_t misc_roms;
	uint8_t ded;
};

struct mbc
{
	struct nes *nes;
	uint8_t *data;
	size_t size;
	struct ines *ines;
	uint8_t *trainer;
	uint8_t *prg_rom_data;
	uint32_t prg_rom_size;
	uint8_t *chr_rom_data;
	uint32_t chr_rom_size;
	uint8_t *prg_ram_data;
	uint32_t prg_ram_size;
	uint8_t *chr_ram_data;
	uint32_t chr_ram_size;
	uint16_t mapper;
	uint8_t banks[11];
	uint8_t irq_latch;
	uint8_t irq_state;
	uint8_t irq_reload;
	uint8_t irq_value;
	uint8_t regs[8];
	uint8_t mapping;
};

struct mem
{
	struct nes *nes;
	uint8_t gpu_regs[7];
	uint8_t wram[0x800];
	uint8_t gpu_names[0x800];
	uint8_t gpu_palettes[0x20];
	uint8_t spram[0x100];
	uint8_t spram_addr;
	uint16_t vram_t;
	uint16_t vram_v;
	uint8_t vram_x;
	uint8_t vram_w;
	uint8_t vram_latch;
	uint8_t joypad_state;
	uint8_t joypad_shift;
	uint8_t joypad_latch;
};

struct nes
{
	struct mbc mbc;
	struct mem mem;
	struct apu apu;
	struct cpu cpu;
	struct gpu gpu;
};

void apu_init(struct apu *apu, struct nes *nes);
void apu_clock(struct apu *apu);

void cpu_init(struct cpu *cpu, struct nes *nes);
void cpu_clock(struct cpu *cpu);

void gpu_init(struct gpu *gpu, struct nes *nes);
void gpu_clock(struct gpu *gpu);

bool mbc_init(struct mbc *mbc, struct nes *nes, const void *data, size_t size);
void mbc_destroy(struct mbc *mbc);

uint8_t mbc_get(struct mbc *mbc, uint16_t addr);
void mbc_set(struct mbc *mbc, uint16_t addr, uint8_t v);

uint8_t mbc_gpu_get(struct mbc *mbc, uint16_t addr);
void mbc_gpu_set(struct mbc *mbc, uint16_t addr, uint8_t v);

void mem_init(struct mem *mem, struct nes *nes);

uint8_t mem_get(struct mem *mem, uint16_t addr);
void mem_set(struct mem *mem, uint16_t addr, uint8_t v);

uint8_t mem_gpu_get(struct mem *mem, uint16_t addr);
void mem_gpu_set(struct mem *mem, uint16_t addr, uint8_t v);

static inline void mem_set_gpu_reg(struct mem *mem, uint16_t r, uint8_t v)
{
	mem->gpu_regs[r - 0x2000] = v;
}

static inline uint8_t mem_get_gpu_reg(struct mem *mem, uint16_t r)
{
	return mem->gpu_regs[r - 0x2000];
}

static inline uint8_t mem_get_spr(struct mem *mem, uint8_t addr)
{
	return mem->spram[addr];
}

static inline uint8_t mem_get_palette(struct mem *mem, uint8_t addr)
{
	return mem->gpu_palettes[palette_table[addr & 0x1F]];
}

static inline uint8_t cpu_peek8(struct cpu *cpu)
{
	return mem_get(&cpu->nes->mem, cpu->regs.pc);
}

static inline uint16_t cpu_peek16(struct cpu *cpu)
{
	uint16_t lo = mem_get(&cpu->nes->mem, cpu->regs.pc + 0);
	uint16_t hi = mem_get(&cpu->nes->mem, cpu->regs.pc + 1);
	return lo | (hi << 8);
}

static inline uint8_t cpu_fetch8(struct cpu *cpu)
{
	return mem_get(&cpu->nes->mem, cpu->regs.pc++);
}

static inline uint16_t cpu_fetch16(struct cpu *cpu)
{
	uint16_t lo = cpu_fetch8(cpu);
	uint16_t hi = cpu_fetch8(cpu);
	return lo | (hi << 8);
}

#endif
