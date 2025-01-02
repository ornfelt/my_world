#include "internal.h"
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

bool mbc_init(struct mbc *mbc, struct nes *nes, const void *data, size_t size)
{
	mbc->nes = nes;
	if (size < sizeof(struct ines))
	{
		fprintf(stderr, "invalid iNES header\n");
		return false;
	}
	if (memcmp(data, "NES\x1A", 4))
	{
		fprintf(stderr, "invalid iNES magic\n");
		return false;
	}
	mbc->data = malloc(size);
	if (!mbc->data)
	{
		fprintf(stderr, "allocation failed\n");
		return false;
	}
	memcpy(mbc->data, data, size);
	mbc->size = size;
	mbc->ines = (struct ines*)mbc->data;
	if (mbc->ines->flags6 & (1 << 2))
		mbc->trainer = &mbc->data[16];
	else
		mbc->trainer = NULL;

	mbc->prg_rom_data = mbc->trainer ? &mbc->data[528] : &mbc->data[16];
	if ((mbc->ines->prg_chr_rom_msb & 0xF) != 0xF)
		mbc->prg_rom_size = 16384 * (mbc->ines->prg_rom_lsb | ((mbc->ines->prg_chr_rom_msb & 0xF) << 8));
	else
		mbc->prg_rom_size = (1 << (mbc->ines->prg_rom_lsb >> 2)) * (1 + 2 * (mbc->ines->prg_rom_lsb & 0x3));
	if ((mbc->ines->prg_chr_rom_msb & 0xF0) != 0xF0)
		mbc->chr_rom_size = 8192 * (mbc->ines->chr_rom_lsb | ((mbc->ines->prg_chr_rom_msb & 0xF0) << 4));
	else
		mbc->chr_rom_size = (1 << (mbc->ines->chr_rom_lsb >> 2)) * (1 + 2 * (mbc->ines->prg_rom_lsb & 0x3));
	if (mbc->chr_rom_size)
		mbc->chr_rom_data = &mbc->prg_rom_data[mbc->prg_rom_size];
	else
		mbc->chr_rom_data = NULL;
	if ((mbc->ines->flags7 & 0x0C) == 0x08)
	{
		if (mbc->ines->prg_ram_size)
			mbc->prg_ram_size = 64 << (mbc->ines->prg_ram_size & 0xF);
		else
			mbc->prg_ram_size = 0;
	}
	else
	{
		mbc->prg_ram_size = 8192;
		if (!mbc->chr_rom_size)
		{
			mbc->chr_ram_size = 0x4000;
			mbc->chr_ram_data = calloc(mbc->chr_ram_size, 1);
			if (!mbc->chr_ram_data)
			{
				fprintf(stderr, "chr ram allocation failed\n");
				free(mbc->data);
				return false;
			}
		}
	}
	if (mbc->prg_ram_size)
	{
		mbc->prg_ram_data = malloc(mbc->prg_ram_size);
		if (!mbc->prg_ram_data)
		{
			fprintf(stderr, "prg ram allocation failed\n");
			free(mbc->data);
			return false;
		}
	}
	mbc->mapper = (mbc->ines->flags6 >> 4) | (mbc->ines->flags7 & 0xF0) | ((mbc->ines->mapper & 0xF) << 8);
	printf("mapper %d\n", mbc->mapper);
	printf("prg_rom_size: %" PRIx32 "\n", (uint32_t)mbc->prg_rom_size);
	printf("prg_rom_data: %" PRIx32 "\n", (uint32_t)(mbc->prg_rom_data - mbc->data));
	printf("chr_rom_size: %" PRIx32 "\n", (uint32_t)mbc->chr_rom_size);
	printf("chr_rom_data: %" PRIx32 "\n", (uint32_t)(mbc->chr_rom_data - mbc->data));
	printf("prg_ram_size: %" PRIx32 "\n", (uint32_t)mbc->prg_ram_size);
	switch (mbc->mapper)
	{
		case 1:
			mbc->banks[1] = (mbc->prg_rom_size / 0x4000) - 1;
			break;
		case 4:
			mbc->banks[10] = (mbc->prg_rom_size / 0x2000) - 2;
			break;
	}
	mbc->mapping = mbc->ines->flags6 & 1;
	return true;
}

void mbc_destroy(struct mbc *mbc)
{
	free(mbc->prg_ram_data);
	free(mbc->data);
}

static uint8_t mapper0_get(struct mbc *mbc, uint16_t addr)
{
#if 0
	printf("mapper0 get [0x%04" PRIx16 "]\n", addr);
#endif
	if (addr < 0x6000)
		return 0;
	if (addr < 0x8000)
		return 0;
	if (addr < 0xC000)
	{
		addr -= 0x8000;
		if (addr >= mbc->prg_rom_size)
			return 0;
		return mbc->prg_rom_data[addr];
	}
	if (mbc->ines->prg_rom_lsb > 1)
		addr -= 0x8000;
	else
		addr -= 0xC000;
	if (addr >= mbc->prg_rom_size)
		return 0;
	return mbc->prg_rom_data[addr];
}

static void mapper0_set(struct mbc *mbc, uint16_t addr, uint8_t v)
{
	(void)mbc;
	(void)addr;
	(void)v;
}

static uint8_t mapper0_gpu_get(struct mbc *mbc, uint16_t addr)
{
	if (addr < 0x2000)
	{
		if (addr >= mbc->chr_rom_size)
			return 0;
		return mbc->chr_rom_data[addr];
	}
	return 0;
}

static void mapper0_gpu_set(struct mbc *mbc, uint16_t addr, uint8_t v)
{
	(void)mbc;
	(void)addr;
	(void)v;
}

static uint8_t mapper1_get(struct mbc *mbc, uint16_t addr)
{
#if 0
	printf("mapper1 get [0x%04" PRIx16 "]\n", addr);
#endif
	if (addr < 0x6000)
		return 0;
	if (addr < 0x8000)
	{
		addr -= 0x6000;
		if (addr >= mbc->prg_ram_size)
			return 0;
		return mbc->prg_ram_data[addr];
	}
	if (addr < 0xC000)
		return mbc->prg_rom_data[addr - 0x8000 + 0x4000 * mbc->banks[0]];
	return mbc->prg_rom_data[addr - 0xC000 + 0x4000 * mbc->banks[1]];
}

static void mapper1_set(struct mbc *mbc, uint16_t addr, uint8_t v)
{
#if 0
	printf("mapper1 set [0x%04" PRIx16 "] = %02" PRIx8 "\n", addr, v);
#endif
	if (addr < 0x6000)
		return;
	if (addr < 0x8000)
	{
		addr -= 0x6000;
		if (addr >= mbc->prg_ram_size)
			return;
		mbc->prg_ram_data[addr] = v;
		return;
	}
	if (v & (1 << 7))
	{
		mbc->regs[0] = 0;
		mbc->regs[1] = 0;
		return;
	}
	mbc->regs[0] |= (v & 1) << mbc->regs[1];
	if (mbc->regs[1] != 4)
	{
		mbc->regs[1]++;
		return;
	}
	switch ((addr >> 13) & 0x3)
	{
		case 0x0:
			mbc->regs[2] = mbc->regs[0];
			switch (mbc->regs[2] & 3)
			{
				case 0:
					/* XXX */
					break;
				case 1:
					/* XXX */
					break;
				case 2:
					mbc->mapping = 1;
					break;
				case 3:
					mbc->mapping = 0;
					break;
			}
			break;
		case 0x1:
			if (mbc->regs[2] & (1 << 4))
			{
				mbc->banks[2] = mbc->regs[0];
			}
			else
			{
				mbc->banks[2] = mbc->regs[0] & ~1;
				mbc->banks[3] = mbc->regs[0] | 1;
			}
			break;
		case 0x2:
			if (mbc->regs[2] & (1 << 4))
				mbc->banks[3] = mbc->regs[0];
			break;
		case 0x3:
			switch ((mbc->regs[2] >> 2) & 3)
			{
				case 0:
				case 1:
					mbc->banks[0] = (mbc->regs[0] & 0xF) & ~1;
					mbc->banks[1] = (mbc->regs[0] & 0xF) | 1;
					break;
				case 2:
					mbc->banks[0] = 0;
					mbc->banks[1] = mbc->regs[0] & 0xF;
					break;
				case 3:
					mbc->banks[0] = mbc->regs[0] & 0xF;
					mbc->banks[1] = (mbc->prg_rom_size / 0x4000) - 1;
					break;
			}
			break;
	}
	mbc->regs[0] = 0;
	mbc->regs[1] = 0;
}

static uint8_t mapper1_gpu_get(struct mbc *mbc, uint16_t addr)
{
	if (addr < 0x1000)
		return mbc->chr_rom_data[addr - 0x0000 + 0x1000 * mbc->banks[2]];
	if (addr < 0x2000)
		return mbc->chr_rom_data[addr - 0x1000 + 0x1000 * mbc->banks[3]];
	return 0;
}

static void mapper1_gpu_set(struct mbc *mbc, uint16_t addr, uint8_t v)
{
	(void)mbc;
	(void)addr;
	(void)v;
}

static uint8_t mapper2_get(struct mbc *mbc, uint16_t addr)
{
#if 0
	printf("mapper2 get [0x%04" PRIx16 "]\n", addr);
#endif
	if (addr < 0x6000)
		return 0;
	if (addr < 0x8000)
		return 0;
	if (addr < 0xC000)
	{
		uint32_t a = addr - 0x8000;
		a += mbc->banks[0] * 0x4000;
		if (a >= mbc->prg_rom_size)
			return 0;
		return mbc->prg_rom_data[a];
	}
	return mbc->prg_rom_data[addr - 0xC000 + (mbc->prg_rom_size - 0x4000)];
}

static void mapper2_set(struct mbc *mbc, uint16_t addr, uint8_t v)
{
#if 0
	printf("mapper2 set [0x%04" PRIx16 "] = %02" PRIx8 "\n", addr, v);
#endif
	if (addr >= 0x8000)
		mbc->banks[0] = v;
}

static uint8_t mapper2_gpu_get(struct mbc *mbc, uint16_t addr)
{
#if 0
	printf("mapper2 gpu get [0x%04" PRIx16 "]\n", addr);
#endif
	if (addr < 0x2000)
	{
		if (mbc->chr_ram_data)
		{
			if (addr >= mbc->chr_ram_size)
				return 0;
			return mbc->chr_ram_data[addr];
		}
		else
		{
			if (addr >= mbc->chr_rom_size)
				return 0;
			return mbc->chr_rom_data[addr];
		}
	}
	return 0;
}

static void mapper2_gpu_set(struct mbc *mbc, uint16_t addr, uint8_t v)
{
#if 0
	printf("mapper2 gpu set [0x%04" PRIx16 "] = %02" PRIx8 "\n", addr, v);
#endif
	if (addr < 0x2000)
	{
		if (mbc->chr_ram_data)
		{
			if (addr >= mbc->chr_ram_size)
				return;
			mbc->chr_ram_data[addr] = v;
			return;
		}
	}
}

static uint8_t mapper3_get(struct mbc *mbc, uint16_t addr)
{
#if 0
	printf("mapper3 get [0x%04" PRIx16 "]\n", addr);
#endif
	if (addr < 0x6000)
		return 0;
	if (addr < 0x8000)
		return 0;
	if (addr < 0xC000)
	{
		addr -= 0x8000;
		if (addr >= mbc->prg_rom_size)
			return 0;
		return mbc->prg_rom_data[addr];
	}
	if (mbc->ines->prg_rom_lsb > 1)
		addr -= 0x8000;
	else
		addr -= 0xC000;
	if (addr >= mbc->prg_rom_size)
		return 0;
	return mbc->prg_rom_data[addr];
}

static void mapper3_set(struct mbc *mbc, uint16_t addr, uint8_t v)
{
	if (addr >= 0x8000)
		mbc->banks[0] = v;
}

static uint8_t mapper3_gpu_get(struct mbc *mbc, uint16_t addr)
{
	if (addr < 0x2000)
	{
		uint32_t a = addr + 0x2000 * mbc->banks[0];
		if (a >= mbc->chr_rom_size)
			return 0;
		return mbc->chr_rom_data[a];
	}
	return 0;
}

static void mapper3_gpu_set(struct mbc *mbc, uint16_t addr, uint8_t v)
{
	(void)mbc;
	(void)addr;
	(void)v;
}

static uint8_t mapper4_get(struct mbc *mbc, uint16_t addr)
{
#if 0
	printf("mapper4 get [0x%04" PRIx16 "]\n", addr);
#endif
	if (addr < 0x6000)
		return 0;
	if (addr < 0x8000)
	{
		addr -= 0x6000;
		if (addr >= mbc->prg_ram_size)
			return 0;
		return mbc->prg_ram_data[addr];
	}
	if (addr < 0xA000)
		return mbc->prg_rom_data[addr - 0x8000 + 0x2000 * mbc->banks[8]];
	if (addr < 0xC000)
		return mbc->prg_rom_data[addr - 0xA000 + 0x2000 * mbc->banks[9]];
	if (addr < 0xE000)
		return mbc->prg_rom_data[addr - 0xC000 + 0x2000 * mbc->banks[10]];
	return mbc->prg_rom_data[addr - 0xE000 + (mbc->prg_rom_size - 0x2000)];
}

static void mapper4_set(struct mbc *mbc, uint16_t addr, uint8_t v)
{
#if 0
	printf("mapper4 set [0x%04" PRIx16 "] = %02" PRIx8 "\n", addr, v);
#endif
	if (addr < 0x6000)
		return;
	if (addr < 0x8000)
	{
		addr -= 0x6000;
		if (addr >= mbc->prg_ram_size)
			return;
		mbc->prg_ram_data[addr] = v;
		return;
	}
	if (addr < 0xA000)
	{
		if (addr & 1)
		{
			switch (mbc->regs[0] & 0x7)
			{
				case 0:
					if (mbc->regs[0] & (1 << 7))
					{
						mbc->banks[4] = v & ~1;
						mbc->banks[5] = v | 1;
					}
					else
					{
						mbc->banks[0] = v & ~1;
						mbc->banks[1] = v | 1;
					}
					break;
				case 1:
					if (mbc->regs[0] & (1 << 7))
					{
						mbc->banks[6] = v & ~1;
						mbc->banks[7] = v | 1;
					}
					else
					{
						mbc->banks[2] = v & ~1;
						mbc->banks[3] = v | 1;
					}
					break;
				case 2:
					if (mbc->regs[0] & (1 << 7))
						mbc->banks[0] = v;
					else
						mbc->banks[4] = v;
					break;
				case 3:
					if (mbc->regs[0] & (1 << 7))
						mbc->banks[1] = v;
					else
						mbc->banks[5] = v;
					break;
				case 4:
					if (mbc->regs[0] & (1 << 7))
						mbc->banks[2] = v;
					else
						mbc->banks[6] = v;
					break;
				case 5:
					if (mbc->regs[0] & (1 << 7))
						mbc->banks[3] = v;
					else
						mbc->banks[7] = v;
					break;
				case 6:
					if (mbc->regs[0] & (1 << 6))
						mbc->banks[10] = v & 0x3F;
					else
						mbc->banks[8] = v & 0x3F;
					break;
				case 7:
					mbc->banks[9] = v & 0x3F;
					break;
			}
		}
		else
		{
			if ((mbc->regs[0] & (1 << 6)) != (v & (1 << 6)))
			{
				uint8_t tmp = mbc->banks[8];
				mbc->banks[8] = mbc->banks[10];
				mbc->banks[10] = tmp;
			}
			mbc->regs[0] = v;
		}
		return;
	}
	if (addr < 0xC000)
	{
		if (addr & 1)
		{
			/* XXX PRG RAM protect */
		}
		else
		{
			mbc->mapping = (~v) & 1;
		}
		return;
	}
	if (addr < 0xE000)
	{
		if (addr & 1)
		{
			mbc->irq_value = 0;
			mbc->irq_reload = v;
		}
		else
		{
			mbc->irq_latch = 1;
		}
		return;
	}
	if (addr & 1)
	{
		mbc->irq_state = 1;
	}
	else
	{
		mbc->irq_state = 0;
	}
}

static uint8_t mapper4_gpu_get(struct mbc *mbc, uint16_t addr)
{
	if (addr < 0x0400)
		return mbc->chr_rom_data[addr - 0x0000 + 0x400 * mbc->banks[0]];
	if (addr < 0x0800)
		return mbc->chr_rom_data[addr - 0x0400 + 0x400 * mbc->banks[1]];
	if (addr < 0x0C00)
		return mbc->chr_rom_data[addr - 0x0800 + 0x400 * mbc->banks[2]];
	if (addr < 0x1000)
		return mbc->chr_rom_data[addr - 0x0C00 + 0x400 * mbc->banks[3]];
	if (addr < 0x1400)
		return mbc->chr_rom_data[addr - 0x1000 + 0x400 * mbc->banks[4]];
	if (addr < 0x1800)
		return mbc->chr_rom_data[addr - 0x1400 + 0x400 * mbc->banks[5]];
	if (addr < 0x1C00)
		return mbc->chr_rom_data[addr - 0x1800 + 0x400 * mbc->banks[6]];
	if (addr < 0x2000)
		return mbc->chr_rom_data[addr - 0x1C00 + 0x400 * mbc->banks[7]];
	return 0;
}

static void mapper4_gpu_set(struct mbc *mbc, uint16_t addr, uint8_t v)
{
	(void)mbc;
	(void)addr;
	(void)v;
}

uint8_t mbc_get(struct mbc *mbc, uint16_t addr)
{
	switch (mbc->mapper)
	{
		case 0:
			return mapper0_get(mbc, addr);
		case 1:
			return mapper1_get(mbc, addr);
		case 2:
			return mapper2_get(mbc, addr);
		case 3:
			return mapper3_get(mbc, addr);
		case 4:
			return mapper4_get(mbc, addr);
		default:
			return 0;
	}
}

void mbc_set(struct mbc *mbc, uint16_t addr, uint8_t v)
{
	switch (mbc->mapper)
	{
		case 0:
			mapper0_set(mbc, addr, v);
			break;
		case 1:
			mapper1_set(mbc, addr, v);
			break;
		case 2:
			mapper2_set(mbc, addr, v);
			break;
		case 3:
			mapper3_set(mbc, addr, v);
			break;
		case 4:
			mapper4_set(mbc, addr, v);
			break;
		default:
			return;
	}
}

uint8_t mbc_gpu_get(struct mbc *mbc, uint16_t addr)
{
	switch (mbc->mapper)
	{
		case 0:
			return mapper0_gpu_get(mbc, addr);
		case 1:
			return mapper1_gpu_get(mbc, addr);
		case 2:
			return mapper2_gpu_get(mbc, addr);
		case 3:
			return mapper3_gpu_get(mbc, addr);
		case 4:
			return mapper4_gpu_get(mbc, addr);
		default:
			return 0;
	}
}

void mbc_gpu_set(struct mbc *mbc, uint16_t addr, uint8_t v)
{
	switch (mbc->mapper)
	{
		case 0:
			mapper0_gpu_set(mbc, addr, v);
			break;
		case 1:
			mapper1_gpu_set(mbc, addr, v);
			break;
		case 2:
			mapper2_gpu_set(mbc, addr, v);
			break;
		case 3:
			mapper3_gpu_set(mbc, addr, v);
			break;
		case 4:
			mapper4_gpu_set(mbc, addr, v);
			break;
		default:
			return;
	}
}
