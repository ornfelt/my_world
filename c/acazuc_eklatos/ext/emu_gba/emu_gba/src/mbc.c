#include "mbc.h"
#include "mem.h"
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

struct mbc *mbc_new(const void *data, size_t size)
{
	struct mbc *mbc = calloc(sizeof(*mbc), 1);
	if (!mbc)
		return NULL;

	mbc->data = malloc(size);
	if (!mbc->data)
		return NULL;

	memcpy(mbc->data, data, size);
	mbc->data_size = size;

	if (memmem(data, size, "EEPROM_V", 8))
		mbc->backup_type = MBC_EEPROM4;
	else if (memmem(data, size, "SRAM_V", 6))
		mbc->backup_type = MBC_SRAM;
	else if (memmem(data, size, "FLASH_V", 7))
		mbc->backup_type = MBC_FLASH64;
	else if (memmem(data, size, "FLASH512_V", 10))
		mbc->backup_type = MBC_FLASH64;
	else if (memmem(data, size, "FLASH1M_V", 9))
		mbc->backup_type = MBC_FLASH128;
	else
		printf("unknown backup type\n");

	if (memmem(data, size, "RTC_V", 5))
		mbc->gpio_type = MBC_RTC;
	else
		mbc->gpio_type = MBC_NONE;

	if (mbc->backup_type == MBC_EEPROM64)
		memset(mbc->backup, 0, sizeof(mbc->backup));
	else
		memset(mbc->backup, 0xFF, sizeof(mbc->backup));

	printf("backup type: %d\n", mbc->backup_type);
	mbc->gpio.rtc.cmd = 0xFF;
	return mbc;
}

void mbc_del(struct mbc *mbc)
{
	if (!mbc)
		return;
	free(mbc->data);
	free(mbc);
}

static uint8_t gpio_get_data(struct mbc *mbc)
{
#if 0
	printf("gpio get data 0x%02" PRIx8 "\n", mbc->gpio.rtc.outbyte);
#endif
	return mbc->gpio.rtc.outbyte;
}

static uint8_t gpio_get_dir(struct mbc *mbc)
{
#if 0
	printf("gpio get dir\n");
#endif
	(void)mbc;
	return 0;
}

static uint8_t gpio_get_ctrl(struct mbc *mbc)
{
#if 0
	printf("gpio get ctrl\n");
#endif
	(void)mbc;
	return 0;
}

#define MBC_GET(size) \
static uint##size##_t eeprom_get##size(struct mbc *mbc, uint32_t addr) \
{ \
	(void)addr; \
	/* printf("eeprom get" #size " %" PRIx32 "\n", addr); */ \
	if (mbc->eeprom.cmd_wr) \
	{ \
		/* printf("EEPROM get while writing\n"); */ \
		return 1; \
	} \
	if (mbc->eeprom.cmd_pos >= 68) \
	{ \
		/* printf("EEPROM past read command\n"); */ \
		return 1; \
	} \
	uint8_t pos = mbc->eeprom.cmd_pos++; \
	if (pos < 4) \
	{ \
		/* printf("EEPROM get fake 0\n"); */ \
		return 0; \
	} \
	uint32_t a = mbc->eeprom.addr * 8 + pos - 4; \
	int ret = (mbc->backup[a / 8] >> (7 - (a % 8))) & 1; \
	/* printf("EEPROM get [%" PRIx32 ",%" PRIx32 "] = %d (%" PRIx8 ")\n", a / 8, a % 8, ret, mbc->backup[a / 8]); */ \
	return ret; \
} \
static uint##size##_t sram_get##size(struct mbc *mbc, uint32_t addr) \
{ \
	addr &= 0x7FFF; \
	uint##size##_t ret = mbc->backup[addr]; \
	if (size == 32) \
		ret = ret | (ret << 8) | (ret << 16) | (ret << 24); \
	else if (size == 16) \
		ret = ret | (ret << 8); \
	/* printf("sram get" #size " %" PRIx32 " = 0x%" PRIx##size "\n", addr, ret); */ \
	return ret; \
} \
static uint##size##_t flash_get##size(struct mbc *mbc, uint32_t addr) \
{ \
	/* printf("flash get" #size " %" PRIx32 "\n", addr); */ \
	addr &= 0xFFFF; \
	if (mbc->flash.chipid) \
	{ \
		if (mbc->backup_type == MBC_FLASH64) \
		{ \
			if (addr == 0) \
				return 0xC2; \
			if (addr == 1) \
				return 0x1C; \
		} \
		else \
		{ \
			if (addr == 0) \
				return 0xC2; \
			if (addr == 1) \
				return 0x09; \
			} \
	} \
	if (mbc->flash.bankid >= 2) \
		return 0xFF; \
	uint8_t ret = mbc->backup[addr + mbc->flash.bankid * 0x10000]; \
	/* printf("ret: %02" PRIx8 "\n", ret); */ \
	if (size == 32) \
		return ret | (ret << 8) | (ret << 16) | (ret << 24); \
	if (size == 16) \
		return ret | (ret << 8); \
	return ret; \
} \
uint##size##_t mbc_get##size(struct mbc *mbc, uint32_t addr) \
{ \
	switch ((addr >> 24) & 0xF) \
	{ \
		case 0x8: \
			if ((addr & ~1) == 0x80000C4) \
			{ \
				if (size == 32) \
					return (gpio_get_data(mbc) << 0) \
					    |  (gpio_get_dir(mbc) << 16); \
				return gpio_get_data(mbc); \
			} \
			if ((addr & ~1) == 0x80000C6) \
			{ \
				if (size == 32) \
					return (gpio_get_dir(mbc) << 0) \
					     | (gpio_get_ctrl(mbc) << 16); \
				return gpio_get_dir(mbc); \
			} \
			if ((addr & ~1) == 0x80000C8) \
				return gpio_get_ctrl(mbc); \
			/* FALLTHROUGH */ \
		case 0x9: /* rom0 */ \
		case 0xA: \
		case 0xB: /* rom1 */ \
		case 0xC: \
		case 0xD: /* rom2 */ \
		{ \
			if ((mbc->backup_type == MBC_EEPROM4 || mbc->backup_type == MBC_EEPROM64) \
			 && ((mbc->data_size <= 0x1000000 && addr >= 0xD000000) || addr >= 0xDFFFF00)) \
				return eeprom_get##size(mbc, addr); \
			uint32_t a = addr & 0x1FFFFFF; \
			if (a < mbc->data_size) \
				return *(uint##size##_t*)&mbc->data[a]; \
			if (size == 16) \
				return addr >> 1; \
			if (size == 32) \
			{ \
				uint16_t lo = addr >> 1; \
				uint16_t hi = lo + 1; \
				return (hi << 16) | lo; \
			} \
			break; \
		} \
		case 0xE: /* backup */ \
		{ \
			switch (mbc->backup_type) \
			{ \
				case MBC_EEPROM4: \
				case MBC_EEPROM64: \
					return 0; \
				case MBC_SRAM: \
					return sram_get##size(mbc, addr); \
				case MBC_FLASH64: \
				case MBC_FLASH128: \
					return flash_get##size(mbc, addr); \
			} \
			break; \
		} \
		case 0xF: /* unused */ \
		{ \
			switch (mbc->backup_type) \
			{ \
				case MBC_EEPROM4: \
				case MBC_EEPROM64: \
					return 0; \
				case MBC_SRAM: \
					return sram_get##size(mbc, addr); \
				case MBC_FLASH64: \
				case MBC_FLASH128: \
					return flash_get##size(mbc, addr); \
			} \
			break; \
		} \
	} \
	printf("unknown get" #size " mbc addr: 0x%08" PRIx32 "\n", addr); \
	return 0; \
}

MBC_GET(8);
MBC_GET(16);
MBC_GET(32);

static void eeprom_set(struct mbc *mbc, uint32_t addr, uint8_t v)
{
	(void)addr;
	v &= 1;
#if 0
	printf("eeprom %" PRIu8 " set %" PRIx32 " = %" PRIx8 "\n", mbc->eeprom.cmd_len, addr, v);
#endif
	mbc->eeprom.cmd[mbc->eeprom.cmd_len / 8] |= v << (7 - (mbc->eeprom.cmd_len % 8));
	mbc->eeprom.cmd_len++;
	if (mbc->eeprom.cmd_len < 2)
		return;
	if (mbc->backup_type == MBC_EEPROM64)
	{
		switch ((mbc->eeprom.cmd[0] & 0xC0))
		{
			case 0xC0:
				if (mbc->eeprom.cmd_len != 17)
					break;
				mbc->eeprom.addr = (mbc->eeprom.cmd[0] & 0x3F) << 7
				                 | (mbc->eeprom.cmd[1] & 0xF0) >> 1;
				mbc->eeprom.cmd_len = 0;
				mbc->eeprom.cmd_pos = 0;
				mbc->eeprom.cmd_wr = 0;
#if 0
				printf("end read cmd @ 0x%" PRIx32 "\n", mbc->eeprom.addr);
#endif
				memset(mbc->eeprom.cmd, 0, sizeof(mbc->eeprom.cmd));
				return;
			case 0x80:
				if (mbc->eeprom.cmd_len != 81)
					break;
				mbc->eeprom.addr = (mbc->eeprom.cmd[0] & 0x3F) << 7
				                 | (mbc->eeprom.cmd[1] & 0xF0) >> 1;
				mbc->eeprom.cmd_len = 0;
				mbc->eeprom.cmd_pos = 0;
				mbc->eeprom.cmd_wr = 1;
#if 0
				printf("end write cmd @ 0x%" PRIx32 "\n", mbc->eeprom.addr);
#endif
				memcpy(&mbc->backup[mbc->eeprom.addr], &mbc->eeprom.cmd[2], 8);
#if 0
				printf("{");
				for (int i = 0; i < 8; ++i)
					printf("%02x, ", mbc->eeprom.cmd[2 + i]);
				printf("}\n");
#endif
				memset(mbc->eeprom.cmd, 0, sizeof(mbc->eeprom.cmd));
				return;
			case 0x00:
				assert(!"unsupported 0x00\n");
				return;
			case 0x40:
				assert(!"unsupported 0x40\n");
				return;
		}
	}
	else
	{
		switch ((mbc->eeprom.cmd[0] & 0xC0))
		{
			case 0xC0:
				if (mbc->eeprom.cmd_len != 9)
					break;
				mbc->eeprom.addr = (mbc->eeprom.cmd[0] & 0x3F) << 3;
				mbc->eeprom.cmd_len = 0;
				mbc->eeprom.cmd_pos = 0;
				mbc->eeprom.cmd_wr = 0;
#if 0
				printf("end read cmd @ 0x%" PRIx32 "\n", mbc->eeprom.addr);
#endif
				memset(mbc->eeprom.cmd, 0, sizeof(mbc->eeprom.cmd));
				return;
			case 0x80:
				if (mbc->eeprom.cmd_len != 73)
					break;
				mbc->eeprom.addr = (mbc->eeprom.cmd[0] & 0x3F) << 3;
				mbc->eeprom.cmd_len = 0;
				mbc->eeprom.cmd_pos = 0;
				mbc->eeprom.cmd_wr = 1;
#if 0
				printf("end write cmd @ 0x%" PRIx32 "\n", mbc->eeprom.addr);
#endif
				memcpy(&mbc->backup[mbc->eeprom.addr], &mbc->eeprom.cmd[2], 8);
#if 0
				printf("{");
				for (int i = 0; i < 8; ++i)
					printf("%02x, ", mbc->eeprom.cmd[2 + i]);
				printf("}\n");
#endif
				memset(mbc->eeprom.cmd, 0, sizeof(mbc->eeprom.cmd));
				return;
			case 0x00:
				assert(!"unsupported 0x00\n");
				return;
			case 0x40:
				assert(!"unsupported 0x40\n");
				return;
		}
	}
}

/* the fact that every single RTC on earth uses BCD scares me */
#define BCD(n) (((n) % 10) + (((n) / 10) * 16))
#define DAA(n) (((n) % 16) + (((n) / 16) * 10))

/* 0: clock
 * 1: data
 * 2: select
 */
static void gpio_set_data(struct mbc *mbc, uint8_t v)
{
#if 0
	printf("gpio set data 0x%02" PRIx8 "\n", v);
#endif
	if (!(v & (1 << 2)))
	{
#if 0
		printf("RTC reset\n");
#endif
		mbc->gpio.rtc.inbuf = 0;
		mbc->gpio.rtc.inlen = 0;
		mbc->gpio.rtc.cmd_flip = 1;
		mbc->gpio.rtc.cmd = 0xFF;
		mbc->gpio.rtc.wpos = 0;
		return;
	}
	if (!(mbc->gpio.dir & (1 << 1)))
	{
		uint8_t b = 0;
#if 0
		printf("RTC read %u / %u\n", mbc->gpio.rtc.outpos, mbc->gpio.rtc.outlen);
#endif
		if (mbc->gpio.rtc.outpos < mbc->gpio.rtc.outlen)
		{
			b = mbc->gpio.rtc.outbuf[mbc->gpio.rtc.outpos / 8];
			b >>= mbc->gpio.rtc.outpos % 8;
			b &= 1;
			if (v & 1)
				mbc->gpio.rtc.outpos++;
		}
		mbc->gpio.rtc.outbyte = 0x5 | (b << 1);
		return;
	}
	if (mbc->gpio.rtc.cmd_flip)
	{
		mbc->gpio.rtc.cmd_flip = 0;
		return;
	}
	if (!(v & 1))
		return;
	mbc->gpio.rtc.inbuf |= ((v >> 1) & 1) << (mbc->gpio.rtc.inlen % 8);
	mbc->gpio.rtc.inlen++;
	if (mbc->gpio.rtc.inlen != 8)
		return;
	mbc->gpio.rtc.inlen = 0;
	if (mbc->gpio.rtc.cmd == 0xFF)
	{
		mbc->gpio.rtc.cmd = mbc->gpio.rtc.inbuf;
#if 0
		printf("RTC cmd %02" PRIx8 "\n", mbc->gpio.rtc.cmd);
#endif
		if (mbc->gpio.rtc.cmd & (1 << 7))
		{
			switch (mbc->gpio.rtc.cmd)
			{
				case 0xC6:
					mbc->gpio.rtc.outbuf[0] = mbc->gpio.rtc.sr;
					mbc->gpio.rtc.outpos = 0;
					mbc->gpio.rtc.outlen = 8;
					break;
				case 0xA6:
				{
					time_t t = time(NULL) + mbc->gpio.rtc.offset;
					struct tm *tm = localtime(&t);
					mbc->gpio.rtc.outbuf[0] = BCD(tm->tm_year - 100);
					mbc->gpio.rtc.outbuf[1] = BCD(tm->tm_mon + 1);
					mbc->gpio.rtc.outbuf[2] = BCD(tm->tm_mday);
					mbc->gpio.rtc.outbuf[3] = BCD(tm->tm_wday);
					mbc->gpio.rtc.outbuf[4] = BCD(tm->tm_hour);
					mbc->gpio.rtc.outbuf[5] = BCD(tm->tm_min);
					mbc->gpio.rtc.outbuf[6] = BCD(tm->tm_sec);
#if 0
					printf("RTC read %x %x %x %x %x %x %x\n",
					       mbc->gpio.rtc.outbuf[0],
					       mbc->gpio.rtc.outbuf[1],
					       mbc->gpio.rtc.outbuf[2],
					       mbc->gpio.rtc.outbuf[3],
					       mbc->gpio.rtc.outbuf[4],
					       mbc->gpio.rtc.outbuf[5],
					       mbc->gpio.rtc.outbuf[6]);
#endif
					mbc->gpio.rtc.outpos = 0;
					mbc->gpio.rtc.outlen = 8 * 7;
					break;
				}
				case 0xE6:
				{
					time_t t = time(NULL) + mbc->gpio.rtc.offset;
					struct tm *tm = localtime(&t);
					mbc->gpio.rtc.outbuf[0] = BCD(tm->tm_hour);
					mbc->gpio.rtc.outbuf[1] = BCD(tm->tm_min);
					mbc->gpio.rtc.outbuf[2] = BCD(tm->tm_sec);
					mbc->gpio.rtc.outpos = 0;
					mbc->gpio.rtc.outlen = 8 * 3;
					break;
				}
				default:
					printf("unknown RTC read cmd: %02" PRIx8 "\n",
					       mbc->gpio.rtc.cmd);
					break;
			}
		}
		else if (mbc->gpio.rtc.cmd == 0x06)
		{
			mbc->gpio.rtc.sr = 0;
			mbc->gpio.rtc.cmd = 0xFF;
			mbc->gpio.rtc.inbuf = 0;
		}
		return;
	}
	if (mbc->gpio.rtc.cmd & (1 << 7))
	{
		printf("RTC write data on read cmd!\n");
		mbc->gpio.rtc.inbuf = 0;
		return;
	}
#if 0
	printf("RTC byte %02" PRIx8 " for cmd %02" PRIx8 "\n",
	       mbc->gpio.rtc.inbuf, mbc->gpio.rtc.cmd);
#endif
	switch (mbc->gpio.rtc.cmd)
	{
		case 0x46:
			mbc->gpio.rtc.sr = mbc->gpio.rtc.inbuf;
			break;
		case 0x26:
			switch (mbc->gpio.rtc.wpos)
			{
				case 0:
					/* XXX for unknown reasons, firmware sends something not ok for year (not bcd?)*/
					mbc->gpio.rtc.tm.tm_year = DAA(mbc->gpio.rtc.inbuf) + 100;
					break;
				case 1:
					mbc->gpio.rtc.tm.tm_mon = DAA(mbc->gpio.rtc.inbuf) - 1;
					break;
				case 2:
					mbc->gpio.rtc.tm.tm_mday = DAA(mbc->gpio.rtc.inbuf);
					break;
				case 3:
					mbc->gpio.rtc.tm.tm_wday = DAA(mbc->gpio.rtc.inbuf);
					break;
				case 4:
					mbc->gpio.rtc.tm.tm_hour = DAA(mbc->gpio.rtc.inbuf);
					break;
				case 5:
					mbc->gpio.rtc.tm.tm_min = DAA(mbc->gpio.rtc.inbuf);
					break;
				case 6:
				{
					mbc->gpio.rtc.tm.tm_sec = DAA(mbc->gpio.rtc.inbuf);
					time_t cur = time(NULL);
					time_t t = mktime(&mbc->gpio.rtc.tm);
					mbc->gpio.rtc.offset = t - cur;
					break;
				}
			}
			mbc->gpio.rtc.wpos++;
			break;
		case 0x66:
			switch (mbc->gpio.rtc.wpos)
			{
				case 0:
					mbc->gpio.rtc.tm.tm_hour = DAA(mbc->gpio.rtc.inbuf);
					break;
				case 1:
					mbc->gpio.rtc.tm.tm_min = DAA(mbc->gpio.rtc.inbuf);
					break;
				case 2:
				{
					mbc->gpio.rtc.tm.tm_sec = DAA(mbc->gpio.rtc.inbuf);
					time_t cur = time(NULL);
					time_t rtc_cur = cur + mbc->gpio.rtc.offset;
					struct tm *cur_tm = localtime(&rtc_cur);
					mbc->gpio.rtc.tm.tm_year = cur_tm->tm_year;
					mbc->gpio.rtc.tm.tm_mon = cur_tm->tm_mon;
					mbc->gpio.rtc.tm.tm_mday = cur_tm->tm_mday;
					mbc->gpio.rtc.tm.tm_wday = cur_tm->tm_wday; /* not required */
					time_t t = mktime(&mbc->gpio.rtc.tm);
					mbc->gpio.rtc.offset = t - cur;
					break;
				}
			}
			break;
		default:
			printf("unknown RTC write cmd: %02" PRIx8 "\n",
			       mbc->gpio.rtc.cmd);
			break;
	}
	mbc->gpio.rtc.inbuf = 0;
}

static void gpio_set_dir(struct mbc *mbc, uint8_t v)
{
#if 0
	printf("gpio set dir 0x%02" PRIx8 "\n", v);
#endif
	mbc->gpio.dir = v;
}

static void gpio_set_ctrl(struct mbc *mbc, uint8_t v)
{
#if 0
	printf("gpio set ctrl 0x%02" PRIx8 "\n", v);
#endif
	(void)mbc;
	(void)v;
}

#define MBC_SET(size) \
static void sram_set##size(struct mbc *mbc, uint32_t addr, uint##size##_t v) \
{ \
	addr &= 0x7FFF; \
	if (size == 32) \
		v >>= 8 * (addr & 3); \
	else if (size == 16) \
		v >>= 8 * (addr & 1); \
	/* printf("sram set" #size " %" PRIx32 " = %" PRIx##size "\n", addr, v); */ \
	mbc->backup[addr] = v; \
} \
static void flash_set##size(struct mbc *mbc, uint32_t addr, uint##size##_t v) \
{ \
	/* printf("flash phase %" PRIu8 " set" #size " %" PRIx32 " = %" PRIx##size "\n", mbc->flash.cmdphase, addr, v); */ \
	addr &= 0xFFFF; \
	if (mbc->flash.cmdphase == 0 && addr == 0x5555 && v == 0xAA) \
	{ \
		mbc->flash.cmdphase++; \
		return;  \
	} \
	if (mbc->flash.cmdphase == 1 && addr == 0x2AAA && v == 0x55) \
	{ \
		mbc->flash.cmdphase++; \
		return; \
	} \
	if (mbc->flash.cmdphase == 2) \
	{ \
		if (addr == 0x5555) \
		{ \
			switch (v) \
			{ \
				case 0x10: \
					if (!mbc->flash.erase) \
						break; \
					memset(mbc->backup, 0xFF, sizeof(mbc->backup)); \
					mbc->flash.erase = false; \
					break; \
				case 0x90: \
					mbc->flash.chipid = true; \
					break; \
				case 0xF0: \
					mbc->flash.chipid = false; \
					break; \
				case 0x80: \
					mbc->flash.erase = true; \
					break; \
				case 0xA0: \
					mbc->flash.write = true; \
					break; \
				case 0xB0: \
					mbc->flash.bank = true; \
					break; \
			} \
		} \
		else if (!(addr & 0xFFF) && mbc->flash.erase) \
		{ \
			uint8_t sector = (addr >> 12) & 0xF; \
			memset(&mbc->backup[sector * 0x1000], 0xFF, 0x1000); \
		} \
		mbc->flash.cmdphase = 0; \
		return; \
	} \
	if (mbc->flash.write) \
	{ \
		if (mbc->flash.bankid < 2) \
		{ \
			if (size == 32) \
				v >>= 8 * (addr & 3); \
			else if (size == 16) \
				v >>= 8 * (addr & 1); \
			mbc->backup[addr + mbc->flash.bankid * 0x10000] = v; \
			mbc->flash.write = false; \
		} \
		return; \
	} \
	if (mbc->flash.bank && !addr) \
	{ \
		mbc->flash.bankid = v; \
		mbc->flash.bank = false; \
		return; \
	} \
} \
void mbc_set##size(struct mbc *mbc, uint32_t addr, uint##size##_t v) \
{ \
	switch ((addr >> 24) & 0xF) \
	{ \
		case 0x8: \
			if (addr == 0x80000C4) \
			{ \
				gpio_set_data(mbc, v); \
				if (size == 32) \
					gpio_set_dir(mbc, v >> 16); \
				return; \
			} \
			if (addr == 0x80000C6) \
			{ \
				gpio_set_dir(mbc, v); \
				if (size == 32) \
					gpio_set_ctrl(mbc, v >> 16); \
				return; \
			} \
			if (addr == 0x80000C8) \
			{ \
				gpio_set_ctrl(mbc, v); \
				return; \
			} \
			/* FALLTHROUGH */ \
		case 0x9: /* rom0 */ \
		case 0xA: \
		case 0xB: /* rom1 */ \
		case 0xC: \
		case 0xD: /* rom2 */ \
			if ((mbc->backup_type == MBC_EEPROM4 || mbc->backup_type == MBC_EEPROM64) \
			 && ((mbc->data_size <= 0x1000000 && addr >= 0xD000000) || addr >= 0xDFFFF00)) \
			{ \
				eeprom_set(mbc, addr, v); \
				return; \
			} \
			break; \
		case 0xE: /* backup */ \
		{ \
			switch (mbc->backup_type) \
			{ \
				case MBC_EEPROM4: \
				case MBC_EEPROM64: \
					return; \
				case MBC_SRAM: \
					sram_set##size(mbc, addr, v); \
					return; \
				case MBC_FLASH64: \
				case MBC_FLASH128: \
					flash_set##size(mbc, addr, v); \
					return; \
			} \
		} \
		case 0xF: /* unused */ \
			break; \
	} \
	printf("unknown set" #size " mbc addr: 0x%08" PRIx32 "\n", addr); \
}

MBC_SET(8);
MBC_SET(16);
MBC_SET(32);
