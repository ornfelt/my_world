#ifndef MBC_H
#define MBC_H

#include <stdint.h>
#include <stddef.h>

#define MBC_SPI_CMD_NONE 0x00
#define MBC_SPI_CMD_WRSR 0x01
#define MBC_SPI_CMD_WRLO 0x02
#define MBC_SPI_CMD_RDLO 0x03
#define MBC_SPI_CMD_WRDI 0x04
#define MBC_SPI_CMD_RDSR 0x05
#define MBC_SPI_CMD_WREN 0x06
#define MBC_SPI_CMD_WRHI 0x0A
#define MBC_SPI_CMD_RDHI 0x0B
#define MBC_SPI_CMD_RDID 0x9F

struct nds;

enum mbc_backup_type
{
	MBC_BACKUP_UNKNOWN,
	MBC_EEPROM_512,
	MBC_EEPROM_8K,
	MBC_EEPROM_64K,
	MBC_EEPROM_128K,
	MBC_FLASH_256K,
	MBC_FLASH_512K,
	MBC_FLASH_1024K,
	MBC_FLASH_2048K,
	MBC_FRAM_8K,
	MBC_FRAM_32K,
};

enum mbc_cmd
{
	MBC_CMD_NONE,
	MBC_CMD_DUMMY,
	MBC_CMD_GETHDR,
	MBC_CMD_ROMID1,
	MBC_CMD_ROMID2,
	MBC_CMD_SECBLK,
	MBC_CMD_ENCREAD,
};

struct mbc_spi
{
	uint8_t cmd;
	uint8_t read_latch;
	uint8_t posb;
	uint8_t write;
	uint32_t addr;
};

struct mbc
{
	struct nds *nds;
	uint8_t *data;
	size_t data_size;
	enum mbc_cmd cmd;
	uint8_t enc;
	uint32_t keybuf[0x412];
	uint64_t key2_x;
	uint64_t key2_y;
	uint32_t cmd_count;
	uint32_t cmd_off;
	uint8_t secure_area[0x800];
	uint8_t chipid[4];
	struct mbc_spi spi;
	enum mbc_backup_type backup_type;
	uint32_t backup_size;
	uint8_t *backup;
};

struct mbc *mbc_new(struct nds *nds, const void *data, size_t size);
void mbc_del(struct mbc *mbc);

void mbc_cmd(struct mbc *mbc);
uint8_t mbc_read(struct mbc *mbc);
void mbc_write(struct mbc *mbc, uint8_t v);

uint8_t mbc_spi_read(struct mbc *mbc);
void mbc_spi_write(struct mbc *mbc, uint8_t v);
void mbc_spi_reset(struct mbc *mbc);

#endif
