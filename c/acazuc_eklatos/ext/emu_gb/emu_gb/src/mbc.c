#include "mbc.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

static void mbc1_update_rombank(struct mbc *mbc);
static void mbc1_update_rambank(struct mbc *mbc);
static void mbc2_update_rombank(struct mbc *mbc);
static void mbc2_update_rambank(struct mbc *mbc);
static void mbc3_update_rombank(struct mbc *mbc);
static void mbc3_update_rambank(struct mbc *mbc);
static void mbc5_update_rombank(struct mbc *mbc);
static void mbc5_update_rambank(struct mbc *mbc);

struct mbc *mbc_new(const void *data, size_t len)
{
	if (len < 0x150)
	{
		fprintf(stderr, "rom too short\n");
		return NULL;
	}

	if (len < (size_t)((32 * 1024) << ((uint8_t*)data)[0x148]))
	{
		fprintf(stderr, "rom has invalid size: got %u, expected %u\n", (unsigned)len, (unsigned)((32 * 1024) << ((uint8_t*)data)[0x148]));
		return NULL;
	}

#if 0
	uint16_t rombanks = (2 << ((uint8_t*)data)[0x148]) - 1;
#else
	uint16_t rombanks = len / (16 * 1024);
#endif
	uint8_t rambanks;
	switch (((uint8_t*)data)[0x149])
	{
		case 0x0:
		case 0x1:
			rambanks = 0;
			break;
		case 0x2:
			rambanks = 1;
			break;
		case 0x3:
			rambanks = 4;
			break;
		case 0x4:
			rambanks = 16;
			break;
		case 0x5:
			rambanks = 8;
			break;
		default:
			fprintf(stderr, "invalid rambanks count\n");
			return NULL;
	}

	enum mbc_type mbctype = MBC_ROM;
	uint8_t options = MBC_OPT_NONE;
	switch (((uint8_t*)data)[0x147])
	{
		case 0x0:
			mbctype = MBC_ROM;
			options = MBC_OPT_NONE;
			break;
		case 0x1:
			mbctype = MBC1;
			options = MBC_OPT_NONE;
			break;
		case 0x2:
			mbctype = MBC1;
			options = MBC_OPT_RAM;
			break;
		case 0x3:
			mbctype = MBC1;
			options = MBC_OPT_RAM | MBC_OPT_BATTERY;
			break;
		case 0x5:
			mbctype = MBC2;
			options = MBC_OPT_NONE;
			rambanks = 1;
			break;
		case 0x6:
			mbctype = MBC2;
			options = MBC_OPT_BATTERY;
			rambanks = 1;
			break;
		case 0x8:
			fprintf(stderr, "unsupported 0x8 rom type\n");
			return NULL;
		case 0x9:
			fprintf(stderr, "unsupported 0x9 rom type\n");
			return NULL;
		case 0xB:
			mbctype = MBC_MMM1;
			options = MBC_OPT_NONE;
			break;
		case 0xC:
			mbctype = MBC_MMM1;
			options = MBC_OPT_RAM;
			break;
		case 0xD:
			mbctype = MBC_MMM1;
			options = MBC_OPT_RAM | MBC_OPT_BATTERY;
			break;
		case 0xF:
			mbctype = MBC3;
			options = MBC_OPT_TIMER | MBC_OPT_BATTERY;
			break;
		case 0x10:
			mbctype = MBC3;
			options = MBC_OPT_TIMER | MBC_OPT_RAM | MBC_OPT_BATTERY;
			break;
		case 0x11:
			mbctype = MBC3;
			options = MBC_OPT_NONE;
			break;
		case 0x12:
			mbctype = MBC3;
			options = MBC_OPT_RAM;
			break;
		case 0x13:
			mbctype = MBC3;
			options = MBC_OPT_RAM | MBC_OPT_BATTERY;
			break;
		case 0x19:
			mbctype = MBC5;
			options = MBC_OPT_NONE;
			break;
		case 0x1A:
			mbctype = MBC5;
			options = MBC_OPT_RAM;
			break;
		case 0x1B:
			mbctype = MBC5;
			options = MBC_OPT_RAM | MBC_OPT_BATTERY;
			break;
		case 0x1C:
			mbctype = MBC5;
			options = MBC_OPT_RUMBLE;
			break;
		case 0x1D:
			mbctype = MBC5;
			options = MBC_OPT_RUMBLE | MBC_OPT_RAM;
			break;
		case 0x1E:
			mbctype = MBC5;
			options = MBC_OPT_RUMBLE | MBC_OPT_RAM | MBC_OPT_BATTERY;
			break;
		case 0x20:
			mbctype = MBC6;
			options = MBC_OPT_NONE;
			break;
		case 0x22:
			mbctype = MBC7;
			options = MBC_OPT_SENSOR | MBC_OPT_RUMBLE | MBC_OPT_RAM | MBC_OPT_BATTERY;
			break;
		case 0xFC:
			mbctype = MBC_CAM;
			options = MBC_OPT_NONE;
			break;
		case 0xFD:
			mbctype = MBC_TAMA5;
			options = MBC_OPT_NONE;
			break;
		case 0xFE:
			mbctype = MBC_HUC3;
			options = MBC_OPT_NONE;
			break;
		case 0xFF:
			mbctype = MBC_HUC1;
			options = MBC_OPT_RAM | MBC_OPT_BATTERY;
			break;
		default:
			fprintf(stderr, "unknown rom type: %d\n", ((uint8_t*)data)[0x147]);
			return NULL;
	}

	struct mbc *mbc = calloc(sizeof(*mbc), 1);
	if (mbc == NULL)
	{
		fprintf(stderr, "allocation failed\n");
		return NULL;
	}

	mbc->data = calloc(len, 1);
	if (mbc->data == NULL)
	{
		fprintf(stderr, "allocation failed\n");
		free(mbc);
		return NULL;
	}

	memcpy(mbc->data, data, len);
	mbc->size = len;
	mbc->rambanksnb = rambanks;
	mbc->rombanksnb = rombanks;
	mbc->type = mbctype;
	mbc->options = options;
	if (rambanks)
	{
		mbc->rambanks = calloc(1, 0x2000 * rambanks);
		if (!mbc->rambanks)
		{
			fprintf(stderr, "allocation failed\n");
			return NULL;
		}
	}

	switch (mbc->type)
	{
		case MBC1:
			mbc1_update_rombank(mbc);
			mbc1_update_rambank(mbc);
			break;
		case MBC2:
			mbc2_update_rombank(mbc);
			mbc2_update_rambank(mbc);
			break;
		case MBC3:
			mbc3_update_rombank(mbc);
			mbc3_update_rambank(mbc);
			break;
		case MBC5:
			mbc->rombank = 1;
			mbc5_update_rombank(mbc);
			mbc5_update_rambank(mbc);
			break;
		default:
			break;
	}

	return mbc;
}

void mbc_del(struct mbc *mbc)
{
	if (mbc == NULL)
		return;

	free(mbc->rambanks);
	free(mbc->data);
	free(mbc);
}

static void mbc1_update_rombank(struct mbc *mbc)
{
	uint8_t rombank;

	if (!mbc->bankmode)
		rombank = mbc->rombank | ((mbc->rambank & 3) << 5);
	else
		rombank = mbc->rombank;

	if (!(rombank & 0x1f))
		rombank++;

	if (rombank > mbc->rombanksnb)
	{
		fprintf(stderr, "invalid mbc1 rombank: %x / %x\n", rombank, mbc->rombanksnb);
		rombank %= mbc->rombanksnb + 1;
	}

	mbc->rombankptr = &mbc->data[0x4000 * rombank];
}

static void mbc1_update_rambank(struct mbc *mbc)
{
	if (!mbc->ramenabled)
	{
		mbc->rambankptr = NULL;
		return;
	}

	if (!mbc->bankmode)
	{
		mbc->rambankptr = mbc->rambanks;
		return;
	}

	if (mbc->rambank >= mbc->rambanksnb)
	{
		fprintf(stderr, "invalid mbc1 rambank: %x / %x\n", mbc->rambank, mbc->rambanksnb);
		mbc->rambankptr = NULL;
		return;
	}

	mbc->rambankptr = &mbc->rambanks[0x2000 * mbc->rambank];
}

static void mbc2_update_rombank(struct mbc *mbc)
{
	uint16_t rombank = mbc->rombank;

	if (!rombank)
		rombank++;

	if (rombank > mbc->rombanksnb)
	{
		fprintf(stderr, "invalid mbc2 rombank: %x / %x\n", rombank, mbc->rombanksnb);
		rombank %= mbc->rombanksnb + 1;
	}

	mbc->rombankptr = &mbc->data[0x4000 * rombank];
}

static void mbc2_update_rambank(struct mbc *mbc)
{
	if (!mbc->ramenabled)
	{
		mbc->rambankptr = NULL;
		return;
	}

	mbc->rambankptr = &mbc->rambanks[0];
}

static void mbc3_update_rombank(struct mbc *mbc)
{
	uint16_t rombank = mbc->rombank;
	if (rombank > mbc->rombanksnb)
	{
		fprintf(stderr, "invalid mbc3 rombank: %x / %x\n", mbc->rombank, mbc->rombanksnb);
		rombank %= mbc->rombanksnb + 1;
	}

	if (mbc->rombank)
		mbc->rombankptr = &mbc->data[0x4000 * rombank];
	else
		mbc->rombankptr = &mbc->data[0x4000];
}

static void mbc3_update_rambank(struct mbc *mbc)
{
	if (!mbc->ramenabled)
	{
		mbc->rambankptr = NULL;
		return;
	}

	if (mbc->rambank < 0x4 && mbc->rambank >= mbc->rambanksnb)
	{
		fprintf(stderr, "invalid mbc3 rambank: %x / %x\n", mbc->rambank, mbc->rambanksnb);
		mbc->rambankptr = NULL;
		return;
	}

	mbc->rambankptr = &mbc->rambanks[0x2000 * mbc->rambank];
}

static void mbc5_update_rombank(struct mbc *mbc)
{
	uint16_t rombank = mbc->rombank;
	if (rombank > mbc->rombanksnb)
	{
		fprintf(stderr, "invalid mbc5 rombank: %x / %x\n", mbc->rombank, mbc->rombanksnb);
		rombank %= mbc->rombanksnb + 1;
	}

	mbc->rombankptr = &mbc->data[0x4000 * rombank];
}

static void mbc5_update_rambank(struct mbc *mbc)
{
	if (!mbc->ramenabled)
	{
		mbc->rambankptr = NULL;
		return;
	}

	if (mbc->rambank >= mbc->rambanksnb)
	{
		fprintf(stderr, "invalid mbc5 rambank: %x / %x\n", mbc->rambank, mbc->rambanksnb);
		mbc->rambankptr = NULL;
		return;
	}

	mbc->rambankptr = &mbc->rambanks[0x2000 * mbc->rambank];
}

static uint8_t rtc_read(struct mbc *mbc, uint8_t reg)
{
	time_t base = time(NULL); /* XXX halt bit */
	switch (reg)
	{
		case 0x8:
		{
			time_t t = base + mbc->rtc.offset;
			struct tm *tm = localtime(&t);
			return tm->tm_sec;
		}
		case 0x9:
		{
			time_t t = base + mbc->rtc.offset;
			struct tm *tm = localtime(&t);
			return tm->tm_min;
		}
		case 0xA:
		{
			time_t t = base + mbc->rtc.offset;
			struct tm *tm = localtime(&t);
			return tm->tm_hour;
		}
		case 0xB:
		{
			int64_t delta = base - mbc->rtc.day_base;
			int64_t days = delta / (60 * 60 * 24);
			if (days < 0)
				return 0;
			return days;
		}
		case 0xC:
		{
			int64_t delta = base - mbc->rtc.day_base;
			int64_t days = delta / (60 * 60 * 24);
			if (days < 0)
				return 0;
			uint8_t ret = ((days % 512) >> 8) & 1;
			if (days > 511)
				ret |= (1 << 7);
			return ret;
		}
		default:
			fprintf(stderr, "invalid rtc read register: 0x%02" PRIx8 "\n", reg);
			return 0;
	}
}

static void rtc_write(struct mbc *mbc, uint8_t reg, uint8_t v)
{
	time_t base = time(NULL); /* XXX halt bit */
	switch (reg)
	{
		case 0x8:
		{
			time_t t = base + mbc->rtc.offset;
			struct tm *tm = localtime(&t);
			tm->tm_sec = v;
			mbc->rtc.offset = mktime(tm) - base;
			break;
		}
		case 0x9:
		{
			time_t t = base + mbc->rtc.offset;
			struct tm *tm = localtime(&t);
			tm->tm_min = v;
			mbc->rtc.offset = mktime(tm) - base;
			break;
		}
		case 0xA:
		{
			time_t t = base + mbc->rtc.offset;
			struct tm *tm = localtime(&t);
			tm->tm_hour = v;
			mbc->rtc.offset = mktime(tm) - base;
			break;
		}
		case 0xB:
		{
			int64_t delta = base - mbc->rtc.day_base;
			int64_t days = delta / (60 * 60 * 24);
			if (days < 0)
				days = 0;
			days -= (days % 256);
			days += v;
			mbc->rtc.day_base = (base - (base % (60 * 60 * 24))) - (days * (60 * 60 * 24));
			break;
		}
		case 0xC:
		{
			int64_t delta = base - mbc->rtc.day_base;
			int64_t days = delta / (60 * 60 * 24);
			if (days < 0)
				days = 0;
			if (!(v & (1 << 7)))
				days %= 512;
			days = (days & ~0x100) | (((uint32_t)v & 1) << 8);
			mbc->rtc.day_base = (base - (base % (60 * 60 * 24))) - (days * (60 * 60 * 24));
			break;
		}
		default:
			fprintf(stderr, "invalid rtc write register: 0x%02" PRIx8 "\n", reg);
			return;
	}
}

uint8_t mbc_get(struct mbc *mbc, uint16_t addr)
{
	if (addr < 0x4000)
	{
		if (addr < mbc->size)
			return mbc->data[addr];
		fprintf(stderr, "rom too short: %x / %x\n", addr, (unsigned)mbc->size);
		return 0;
	}

	switch (mbc->type)
	{
		case MBC_ROM:
			if (addr < 0x8000)
			{
				if (addr < mbc->size)
					return mbc->data[addr];
				fprintf(stderr, "rom too short: %x / %x\n", addr, (unsigned)mbc->size);
			}
			return 0xFF;
		case MBC1:
			if (addr < 0x8000)
			{
				if (!mbc->rombankptr)
					return 0xFF;
				return mbc->rombankptr[addr - 0x4000];
			}
			if (addr >= 0xA000 && addr < 0xC000)
			{
				if (!mbc->rambankptr)
					return 0xFF;
				return mbc->rambankptr[addr - 0xA000];
			}
			fprintf(stderr, "mbc1 read from unknown addr: %04x\n", addr);
			return 0xFF;
		case MBC2:
			if (addr < 0x8000)
			{
				if (!mbc->rombankptr)
					return 0xFF;
				return mbc->rombankptr[addr - 0x4000];
			}
			if (addr >= 0xA000 && addr < 0xC000)
			{
				if (!mbc->rambankptr)
					return 0xFF;
				return mbc->rambankptr[addr - 0xA000] & 0xf;
			}
			fprintf(stderr, "mbc2 read from unknown addr: %04x\n", addr);
			return 0xFF;
		case MBC3:
			if (addr < 0x8000)
			{
				if (!mbc->rombankptr)
					return 0xFF;
				return mbc->rombankptr[addr - 0x4000];
			}
			if (addr >= 0xA000 && addr < 0xC000)
			{
				if (mbc->rambank > 0x3)
				{
					if (mbc->rambank >= 0x8 && mbc->rambank <= 0xC)
						return rtc_read(mbc, mbc->rambank);
					return 0xFF;
				}
				if (!mbc->rambankptr)
					return 0xFF;
				return mbc->rambankptr[addr - 0xA000];
			}
			fprintf(stderr, "mbc3 read from unknown addr: %04x\n", addr);
			return 0xFF;
		case MBC5:
			if (addr < 0x8000)
			{
				if (!mbc->rombankptr)
					return 0xFF;
				return mbc->rombankptr[addr - 0x4000];
			}
			if (addr >= 0xA000 && addr < 0xC000)
			{
				if (!mbc->rambankptr)
					return 0xFF;
				return mbc->rambankptr[addr - 0xA000];
			}
			fprintf(stderr, "mbc5 read from unknown addr: %04x\n", addr);
			return 0xFF;
		case MBC6:
		case MBC7:
		case MBC_HUC1:
		case MBC_HUC3:
		case MBC_MMM1:
		case MBC_CAM:
		case MBC_TAMA5:
			return 0;
	}
	return 0;
}

void mbc_set(struct mbc *mbc, uint16_t addr, uint8_t v)
{
	switch (mbc->type)
	{
		case MBC_ROM:
			return;
		case MBC1:
			if (addr < 0x2000)
			{
				mbc->ramenabled = ((v & 0x0F) == 0xA);
				mbc1_update_rambank(mbc);
				return;
			}
			if (addr < 0x4000)
			{
				mbc->rombank = v & 0x1F;
				mbc1_update_rombank(mbc);
				return;
			}
			if (addr < 0x6000)
			{
				mbc->rambank = v & 0x3;
				mbc1_update_rombank(mbc);
				mbc1_update_rambank(mbc);
				return;
			}
			if (addr < 0x8000)
			{
				mbc->bankmode = v & 1;
				mbc1_update_rombank(mbc);
				mbc1_update_rambank(mbc);
				return;
			}
			if (addr >= 0xA000 && addr < 0xC000)
			{
				if (!mbc->rambankptr)
					return;
				mbc->rambankptr[addr - 0xA000] = v;
				return;
			}
			fprintf(stderr, "mbc1 write to unknown addr: %04x\n", addr);
			return;
		case MBC2:
			if (addr < 0x2000)
			{
				mbc->ramenabled = ((v & 0x0F) == 0xA);
				mbc2_update_rambank(mbc);
				return;
			}
			if (addr < 0x4000)
			{
				mbc->rombank = v;
				mbc2_update_rombank(mbc);
				return;
			}
			if (addr >= 0xA000 && addr < 0xC000)
			{
				if (!mbc->rambankptr)
					return;
				addr -= 0xA000;
				if (addr > 0x1FF)
				{
					fprintf(stderr, "invalid RAM write: %x\n", 0xA000 + addr);
					return;
				}
				mbc->rambankptr[addr] = v & 0xf;
				return;
			}
			fprintf(stderr, "mbc2 write to unknown addr: %04x\n", addr);
			return;
		case MBC3:
			if (addr < 0x2000)
			{
				mbc->ramenabled = ((v & 0x0F) == 0xA);
				mbc3_update_rambank(mbc);
				return;
			}
			if (addr < 0x4000)
			{
				mbc->rombank = v & 0x7F;
				mbc3_update_rombank(mbc);
				return;
			}
			if (addr < 0x6000)
			{
				mbc->rambank = v;
				mbc3_update_rambank(mbc);
				return;
			}
			if (addr < 0x8000)
			{
				//XXX latch clock
				return;
			}
			if (addr >= 0xA000 && addr < 0xC000)
			{
				if (mbc->rambank > 0x3)
				{
					if (mbc->rambank >= 0x8 && mbc->rambank <= 0xC)
						rtc_write(mbc, mbc->rambank, v);
					return;
				}
				if (!mbc->rambankptr)
					return;
				mbc->rambankptr[addr - 0xA000] = v;
				return;
			}
			fprintf(stderr, "mbc3 write to unknown addr: %04x\n", addr);
			return;
		case MBC5:
			if (addr < 0x2000)
			{
				mbc->ramenabled = (v == 0xA);
				mbc5_update_rambank(mbc);
				return;
			}
			if (addr < 0x3000)
			{
				mbc->rombank = (mbc->rombank & 0xFF00) | v;
				mbc5_update_rombank(mbc);
				return;
			}
			if (addr < 0x4000)
			{
				mbc->rombank = (mbc->rombank & 0x00FF) | ((v << 8) & 0x100);
				mbc5_update_rombank(mbc);
				return;
			}
			if (addr < 0x6000)
			{
				mbc->rambank = v;
				mbc5_update_rambank(mbc);
				return;
			}
			if (addr >= 0xA000 && addr < 0xC000)
			{
				if (!mbc->rambankptr)
					return;
				mbc->rambankptr[addr - 0xA000] = v;
				return;
			}
			fprintf(stderr, "mbc5 write to unknown addr: %04x\n", addr);
			return;
		case MBC6:
			return;
		case MBC7:
			return;
		case MBC_HUC1:
			return;
		case MBC_HUC3:
			return;
		case MBC_MMM1:
			return;
		case MBC_CAM:
			return;
		case MBC_TAMA5:
			return;
	}
}
