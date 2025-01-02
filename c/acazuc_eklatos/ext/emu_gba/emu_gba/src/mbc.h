#ifndef MBC_H
#define MBC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

enum mbc_backup_type
{
	MBC_EEPROM4,
	MBC_EEPROM64,
	MBC_SRAM,
	MBC_FLASH64,
	MBC_FLASH128,
};

enum mbc_gpio_type
{
	MBC_NONE,
	MBC_RTC,
};

struct eeprom
{
	uint32_t addr;
	uint8_t cmd[11];
	uint8_t cmd_len;
	uint8_t cmd_pos;
	uint8_t cmd_wr;
};

struct flash
{
	uint8_t cmdphase;
	uint8_t bankid;
	bool chipid;
	bool erase;
	bool write;
	bool bank;
};

struct rtc
{
	int cmd_flip;
	uint8_t cmd;
	uint8_t inbuf;
	uint8_t inlen; /* in bits */
	uint8_t outbuf[8];
	uint8_t outlen; /* in bits */
	uint8_t outpos; /* in bits */
	uint8_t outbyte;
	uint8_t wpos;
	uint8_t sr;
	uint8_t int1_steady_freq;
	int64_t offset;
	struct tm tm; /* for date / time set */
};

struct mbc
{
	uint8_t *data;
	size_t data_size;
	uint8_t backup[0x20000];
	enum mbc_backup_type backup_type;
	union
	{
		struct eeprom eeprom;
		struct flash flash;
	};
	enum mbc_gpio_type gpio_type;
	struct
	{
		uint8_t dir;
		struct rtc rtc;
	} gpio;
};

struct mbc *mbc_new(const void *data, size_t size);
void mbc_del(struct mbc *mbc);

uint8_t  mbc_get8 (struct mbc *mbc, uint32_t addr);
uint16_t mbc_get16(struct mbc *mbc, uint32_t addr);
uint32_t mbc_get32(struct mbc *mbc, uint32_t addr);

void mbc_set8 (struct mbc *mbc, uint32_t addr, uint8_t v);
void mbc_set16(struct mbc *mbc, uint32_t addr, uint16_t v);
void mbc_set32(struct mbc *mbc, uint32_t addr, uint32_t v);

#endif
