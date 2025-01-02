#ifndef MBC_H
#define MBC_H

#include <stdint.h>
#include <stddef.h>

enum mbc_type
{
	MBC_ROM,
	MBC1,
	MBC2,
	MBC3,
	MBC5,
	MBC6,
	MBC7,
	MBC_HUC1,
	MBC_HUC3,
	MBC_MMM1,
	MBC_CAM,
	MBC_TAMA5,
};

enum mbc_option
{
	MBC_OPT_NONE    = 0x00,
	MBC_OPT_RAM     = 0x01,
	MBC_OPT_BATTERY = 0x02,
	MBC_OPT_TIMER   = 0x04,
	MBC_OPT_RUMBLE  = 0x08,
	MBC_OPT_SENSOR  = 0x10,
};

struct mbc
{
	uint8_t *data;
	size_t size;
	enum mbc_type type;
	uint8_t *rambankptr;
	uint8_t *rombankptr;
	uint8_t *rambanks;
	uint8_t rambanksnb;
	uint8_t ramenabled;
	uint16_t rombanksnb;
	uint16_t rambank;
	uint16_t rombank;
	uint8_t options;
	uint8_t bankmode;
	struct
	{
		int64_t offset; /* for sec, min, hour */
		int64_t day_base; /* for days counter */
	} rtc;
};

struct mbc *mbc_new(const void *data, size_t len);
void mbc_del(struct mbc *mbc);

uint8_t mbc_get(struct mbc *mbc, uint16_t addr);
void    mbc_set(struct mbc *mbc, uint16_t addr, uint8_t v);

#endif
