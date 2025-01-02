#ifndef MEM_H
#define MEM_H

#include <stdbool.h>
#include <stdint.h>

#define MEM_REG_JOYP 0xFF00
#define MEM_REG_SB   0xFF01
#define MEM_REG_SC   0xFF02
#define MEM_REG_DIV  0xFF04
#define MEM_REG_TIMA 0xFF05
#define MEM_REG_TMA  0xFF06
#define MEM_REG_TAC  0xFF07
#define MEM_REG_IF   0xFF0F
#define MEM_REG_NR10 0xFF10
#define MEM_REG_NR11 0xFF11
#define MEM_REG_NR12 0xFF12
#define MEM_REG_NR13 0xFF13
#define MEM_REG_NR14 0xFF14
#define MEM_REG_NR21 0xFF16
#define MEM_REG_NR22 0xFF17
#define MEM_REG_NR23 0xFF18
#define MEM_REG_NR24 0xFF19
#define MEM_REG_NR30 0xFF1A
#define MEM_REG_NR31 0xFF1B
#define MEM_REG_NR32 0xFF1C
#define MEM_REG_NR33 0xFF1D
#define MEM_REG_NR34 0xFF1E
#define MEM_REG_NR41 0xFF20
#define MEM_REG_NR42 0xFF21
#define MEM_REG_NR43 0xFF22
#define MEM_REG_NR44 0xFF23
#define MEM_REG_NR50 0xFF24
#define MEM_REG_NR51 0xFF25
#define MEM_REG_NR52 0xFF26
#define MEM_REG_WAVE 0xFF30
#define MEM_REG_LCDC 0xFF40
#define MEM_REG_STAT 0xFF41
#define MEM_REG_SCY  0xFF42
#define MEM_REG_SCX  0xFF43
#define MEM_REG_LY   0xFF44
#define MEM_REG_LYC  0xFF45
#define MEM_REG_DMA  0xFF46
#define MEM_REG_BGP  0xFF47
#define MEM_REG_OBP0 0xFF48
#define MEM_REG_OBP1 0xFF49
#define MEM_REG_WY   0xFF4A
#define MEM_REG_WX   0xFF4B
#define MEM_REG_KEY0 0xFF4C
#define MEM_REG_KEY1 0xFF4D
#define MEM_REG_VBK  0xFF4F
#define MEM_REG_BOOT 0xFF50
#define MEM_REG_HDM1 0xFF51
#define MEM_REG_HDM2 0xFF52
#define MEM_REG_HDM3 0xFF53
#define MEM_REG_HDM4 0xFF54
#define MEM_REG_HDM5 0xFF55
#define MEM_REG_RP   0xFF56
#define MEM_REG_BCPS 0xFF68
#define MEM_REG_BCPD 0xFF69
#define MEM_REG_OCPS 0xFF6A
#define MEM_REG_OCPD 0xFF6B
#define MEM_REG_OPRI 0xFF6C
#define MEM_REG_SVBK 0xFF70
#define MEM_REG_PCM1 0xFF76
#define MEM_REG_PCM2 0xFF77
#define MEM_REG_IE   0xFFFF

struct mbc;
struct gb;

enum cgb_mode
{
	CGB_NO,
	CGB_YES,
	CGB_FORCE,
};

struct mem
{
	uint8_t dmg_bios[0x100];
	uint8_t cgb_bios[0x900];
	uint8_t vram[0x4000];
	uint8_t workram0[0x1000];
	uint8_t workram1[0x7000];
	uint8_t oam[0xA0];
	uint8_t highram[0x100];
	uint8_t objpalette[64];
	uint8_t bgpalette[64];
	uint8_t svbk;
	uint8_t vbk;
	uint8_t joyp;
	uint8_t dmatransfer;
	uint16_t timer;
	uint16_t hdma_src;
	uint16_t hdma_dst;
	uint16_t hdma_len;
	bool doublespeed;
	enum cgb_mode cgb;
	struct mbc *mbc;
	struct gb *gb;
};

struct mem *mem_new(struct gb *gb, struct mbc *mbc);
void mem_del(struct mem *mem);

void mem_dmatransfer(struct mem *mem);
void mem_hdmatransfer(struct mem *mem);

static inline uint8_t mem_get_reg(struct mem *mem, uint16_t addr)
{
	return mem->highram[addr - 0xFF00];
}

static inline void mem_set_reg(struct mem *mem, uint16_t addr, uint8_t v)
{
	mem->highram[addr - 0xFF00] = v;
}

static inline uint8_t mem_get_vram0(struct mem *mem, uint16_t addr)
{
	return mem->vram[addr - 0x8000];
}

static inline uint8_t mem_get_vram1(struct mem *mem, uint16_t addr)
{
	return mem->vram[addr - 0x8000 + 0x2000];
}

static inline uint8_t mem_get_oam(struct mem *mem, uint16_t addr)
{
	return mem->oam[addr - 0xFE00];
}

uint8_t mem_get(struct mem *mem, uint16_t addr);
void    mem_set(struct mem *mem, uint16_t addr, uint8_t v);

#endif
