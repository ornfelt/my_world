#ifndef MEM_H
#define MEM_H

#include <stdbool.h>
#include <stdint.h>

#define MEM_REG_DISPCNT     0x000
#define MEM_REG_DISPSTAT    0x004
#define MEM_REG_VCOUNT      0x006
#define MEM_REG_BG0CNT      0x008
#define MEM_REG_BG1CNT      0x00A
#define MEM_REG_BG2CNT      0x00C
#define MEM_REG_BG3CNT      0x00E
#define MEM_REG_BG0HOFS     0x010
#define MEM_REG_BG0VOFS     0x012
#define MEM_REG_BG1HOFS     0x014
#define MEM_REG_BG1VOFS     0x016
#define MEM_REG_BG2HOFS     0x018
#define MEM_REG_BG2VOFS     0x01A
#define MEM_REG_BG3HOFS     0x01C
#define MEM_REG_BG3VOFS     0x01E
#define MEM_REG_BG2PA       0x020
#define MEM_REG_BG2PB       0x022
#define MEM_REG_BG2PC       0x024
#define MEM_REG_BG2PD       0x026
#define MEM_REG_BG2X        0x028
#define MEM_REG_BG2Y        0x02C
#define MEM_REG_BG3PA       0x030
#define MEM_REG_BG3PB       0x032
#define MEM_REG_BG3PC       0x034
#define MEM_REG_BG3PD       0x036
#define MEM_REG_BG3X        0x038
#define MEM_REG_BG3Y        0x03C
#define MEM_REG_WIN0H       0x040
#define MEM_REG_WIN1H       0x042
#define MEM_REG_WIN0V       0x044
#define MEM_REG_WIN1V       0x046
#define MEM_REG_WININ       0x048
#define MEM_REG_WINOUT      0x04A
#define MEM_REG_MOSAIC      0x04C
#define MEM_REG_BLDCNT      0x050
#define MEM_REG_BLDALPHA    0x052
#define MEM_REG_BLDY        0x054

#define MEM_REG_SOUND1CNT_L 0x060
#define MEM_REG_SOUND1CNT_H 0x062
#define MEM_REG_SOUND1CNT_X 0x064
#define MEM_REG_SOUND2CNT_L 0x068
#define MEM_REG_SOUND2CNT_H 0x06C
#define MEM_REG_SOUND3CNT_L 0x070
#define MEM_REG_SOUND3CNT_H 0x072
#define MEM_REG_SOUND3CNT_X 0x074
#define MEM_REG_SOUND4CNT_L 0x078
#define MEM_REG_SOUND4CNT_H 0x07C
#define MEM_REG_SOUNDCNT_L  0x080
#define MEM_REG_SOUNDCNT_H  0x082
#define MEM_REG_SOUNDCNT_X  0x084
#define MEM_REG_SOUNDBIAS   0x088
#define MEM_REG_WAVE_RAM0_L 0x090
#define MEM_REG_WAVE_RAM0_H 0x092
#define MEM_REG_WAVE_RAM1_L 0x094
#define MEM_REG_WAVE_RAM1_H 0x096
#define MEM_REG_WAVE_RAM2_L 0x098
#define MEM_REG_WAVE_RAM2_H 0x09A
#define MEM_REG_WAVE_RAM3_L 0x09C
#define MEM_REG_WAVE_RAM3_H 0x09E
#define MEM_REG_FIFO_A      0x0A0
#define MEM_REG_FIFO_B      0x0A4

#define MEM_REG_DMA0SAD     0x0B0
#define MEM_REG_DMA0DAD     0x0B4
#define MEM_REG_DMA0CNT_L   0x0B8
#define MEM_REG_DMA0CNT_H   0x0BA
#define MEM_REG_DMA1SAD     0x0BC
#define MEM_REG_DMA1DAD     0x0C0
#define MEM_REG_DMA1CNT_L   0x0C4
#define MEM_REG_DMA1CNT_H   0x0C6
#define MEM_REG_DMA2SAD     0x0C8
#define MEM_REG_DMA2DAD     0x0CC
#define MEM_REG_DMA2CNT_L   0x0D0
#define MEM_REG_DMA2CNT_H   0x0D2
#define MEM_REG_DMA3SAD     0x0D4
#define MEM_REG_DMA3DAD     0x0D8
#define MEM_REG_DMA3CNT_L   0x0DC
#define MEM_REG_DMA3CNT_H   0x0DE

#define MEM_REG_TM0CNT_L    0x100
#define MEM_REG_TM0CNT_H    0x102
#define MEM_REG_TM1CNT_L    0x104
#define MEM_REG_TM1CNT_H    0x106
#define MEM_REG_TM2CNT_L    0x108
#define MEM_REG_TM2CNT_H    0x10A
#define MEM_REG_TM3CNT_L    0x10C
#define MEM_REG_TM3CNT_H    0x10E

#define MEM_REG_SIODATA32   0x120
#define MEM_REG_SIOMULTI0   0x120
#define MEM_REG_SIOMULTI1   0x122
#define MEM_REG_SIOMULTI2   0x124
#define MEM_REG_SIOMULTI3   0x126
#define MEM_REG_SIOCNT      0x128
#define MEM_REG_SIOMLT_SEND 0x12A
#define MEM_REG_SIODATA8    0x12A

#define MEM_REG_KEYINPUT    0x130
#define MEM_REG_KEYCNT      0x132

#define MEM_REG_RCNT        0x134
#define MEM_REG_JOYCNT      0x140
#define MEM_REG_JOY_RECV    0x150
#define MEM_REG_JOY_TRANS   0x154
#define MEM_REG_JOYSTAT     0x158

#define MEM_REG_IE          0x200
#define MEM_REG_IF          0x202
#define MEM_REG_WAITCNT     0x204
#define MEM_REG_IME         0x208
#define MEM_REG_POSTFLG     0x300
#define MEM_REG_HALTCNT     0x301

struct mbc;
struct gba;

struct mem_dma
{
	bool enabled;
	bool active;
	uint32_t src;
	uint32_t dst;
	uint32_t len;
	uint32_t cnt;
};

struct mem_timer
{
	uint16_t v;
};

struct mem
{
	struct gba *gba;
	struct mbc *mbc;
	struct mem_timer timers[4];
	struct mem_dma dma[4];
	uint8_t bios[0x4000];
	uint8_t board_wram[0x40000];
	uint8_t chip_wram[0x8000];
	uint8_t io_regs[0x400];
	uint8_t palette[0x400];
	uint8_t vram[0x18000];
	uint8_t oam[0x400];
	uint8_t wave[0x20];
	uint8_t fifo[2][0x20];
	uint8_t fifo_nb[2];
};

struct mem *mem_new(struct gba *gba, struct mbc *mbc);
void mem_del(struct mem *mem);

void mem_timers(struct mem *mem);
bool mem_dma(struct mem *mem);
void mem_hblank(struct mem *mem);
void mem_vblank(struct mem *mem);
void mem_fifo(struct mem *mem, uint8_t fifo);

uint8_t  mem_get8 (struct mem *mem, uint32_t addr);
uint16_t mem_get16(struct mem *mem, uint32_t addr);
uint32_t mem_get32(struct mem *mem, uint32_t addr);
void mem_set8 (struct mem *mem, uint32_t addr, uint8_t val);
void mem_set16(struct mem *mem, uint32_t addr, uint16_t val);
void mem_set32(struct mem *mem, uint32_t addr, uint32_t val);

static inline uint32_t mem_get_reg32(struct mem *mem, uint32_t reg)
{
	return *(uint32_t*)&mem->io_regs[reg];
}

static inline void mem_set_reg32(struct mem *mem, uint32_t reg, uint32_t v)
{
	*(uint32_t*)&mem->io_regs[reg] = v;
}

static inline uint16_t mem_get_reg16(struct mem *mem, uint32_t reg)
{
	return *(uint16_t*)&mem->io_regs[reg];
}

static inline void mem_set_reg16(struct mem *mem, uint32_t reg, uint16_t v)
{
	*(uint16_t*)&mem->io_regs[reg] = v;
}

static inline uint8_t mem_get_reg8(struct mem *mem, uint32_t reg)
{
	return mem->io_regs[reg];
}

static inline void mem_set_reg8(struct mem *mem, uint32_t reg, uint8_t v)
{
	mem->io_regs[reg] = v;
}

static inline uint16_t mem_get_oam16(struct mem *mem, uint32_t addr)
{
	return *(uint16_t*)&mem->oam[addr];
}

static inline uint8_t mem_get_vram8(struct mem *mem, uint32_t addr)
{
	return mem->vram[addr];
}

static inline uint16_t mem_get_vram16(struct mem *mem, uint32_t addr)
{
	return *(uint16_t*)&mem->vram[addr & ~1];
}

static inline uint16_t mem_get_bg_palette(struct mem *mem, uint32_t addr)
{
	return *(uint16_t*)&mem->palette[addr];
}

static inline uint16_t mem_get_obj_palette(struct mem *mem, uint32_t addr)
{
	return *(uint16_t*)&mem->palette[0x200 + addr];
}

static inline uint8_t mem_get_wave4(struct mem *mem, uint8_t offset)
{
	uint8_t v = mem->wave[offset / 2];
	if (offset & 1)
		return v & 0xF;
	return v >> 4;
}

#endif
