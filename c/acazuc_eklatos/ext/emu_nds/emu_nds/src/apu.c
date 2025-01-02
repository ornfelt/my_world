#include "apu.h"
#include "mem.h"

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

static const int adpcm_index_table[] = {-1, -1, -1, -1, 2, 4, 6, 8};
static const uint16_t adpcm_table[] =
{
	0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E,
	0x0010, 0x0011, 0x0013, 0x0015, 0x0017, 0x0019, 0x001C, 0x001F,
	0x0022, 0x0025, 0x0029, 0x002D, 0x0032, 0x0037, 0x003C, 0x0042,
	0x0049, 0x0050, 0x0058, 0x0061, 0x006B, 0x0076, 0x0082, 0x008F,
	0x009D, 0x00AD, 0x00BE, 0x00D1, 0x00E6, 0x00FD, 0x0117, 0x0133,
	0x0151, 0x0173, 0x0198, 0x01C1, 0x01EE, 0x0220, 0x0256, 0x0292,
	0x02D4, 0x031C, 0x036C, 0x03C3, 0x0424, 0x048E, 0x0502, 0x0583,
	0x0610, 0x06AB, 0x0756, 0x0812, 0x08E0, 0x09C3, 0x0ABD, 0x0BD0,
	0x0CFF, 0x0E4C, 0x0FBA, 0x114C, 0x1307, 0x14EE, 0x1706, 0x1954,
	0x1BDC, 0x1EA5, 0x21B6, 0x2515, 0x28CA, 0x2CDF, 0x315B, 0x364B,
	0x3BB9, 0x41B2, 0x4844, 0x4F7E, 0x5771, 0x602F, 0x69CE, 0x7462,
	0x7FFF
};

struct apu *apu_new(struct mem *mem)
{
	struct apu *apu = calloc(sizeof(*apu), 1);
	if (!apu)
		return NULL;

	apu->mem = mem;
	return apu;
}

void apu_del(struct apu *apu)
{
	if (!apu)
		return;
	free(apu);
}

static void gen_sample(struct apu *apu, int16_t *dst)
{
	uint32_t soundcnt = mem_arm7_get_reg32(apu->mem, MEM_ARM7_REG_SOUNDCNT);
	if (!(soundcnt & (1 << 15)))
	{
		dst[0] = 0;
		dst[1] = 0;
		return;
	}
	uint32_t powcnt2 = mem_arm7_get_reg32(apu->mem, MEM_ARM7_REG_POWCNT2);
	if (!(powcnt2 & (1 << 0)))
	{
		dst[0] = 0;
		dst[1] = 0;
		return;
	}
	if (!(apu->mem->spi_powerman.regs[0] & (1 << 0)))
	{
		dst[0] = 0;
		dst[1] = 0;
		return;
	}
	int32_t l = 0;
	int32_t r = 0;
	for (size_t i = 0; i < 16; ++i)
	{
		uint32_t cnt = mem_arm7_get_reg32(apu->mem, MEM_ARM7_REG_SOUNDXCNT(i));
		if (!(cnt & (1 << 31)))
			continue;
		struct apu_channel *channel = &apu->channels[i];
		int32_t sample = channel->sample;
		uint8_t volume = cnt & 0x7F;
		sample = (sample * volume) / 128 / 2; /* divide by two to avoid being too loud */
		static const uint8_t dividers[4] = {0, 1, 2, 4};
		uint8_t divider = dividers[(cnt >> 8) & 0x3];
		sample >>= divider;
		uint8_t pan = (cnt >> 16) & 0x7F;
		l += (sample * (127 - pan)) / 128;
		r += (sample * pan) / 128;
	}
	uint8_t volume = soundcnt & 0x7F;
	l = (l * volume) / 128;
	r = (r * volume) / 128;
	if (l < INT16_MIN)
		l = INT16_MIN;
	else if (l > INT16_MAX)
		l = INT16_MAX;
	if (r < INT16_MIN)
		r = INT16_MIN;
	else if (r > INT16_MAX)
		r = INT16_MAX;
	dst[0] = l;
	dst[1] = r;
}

void apu_sample(struct apu *apu, uint32_t cycles)
{
	apu->clock += cycles;
	if (apu->clock < apu->next_sample)
		return;
#if 0
	printf("sample %u\n", apu->sample);
#endif
	gen_sample(apu, &apu->data[apu->sample * 2]);
	apu->sample++;
	apu->next_sample = (1120380 * apu->sample) / (APU_FRAME_SAMPLES - 1);
}

void apu_cycles(struct apu *apu, uint32_t cycles)
{
	uint32_t powcnt2 = mem_arm7_get_reg32(apu->mem, MEM_ARM7_REG_POWCNT2);
	if (!(powcnt2 & (1 << 0)))
		return;
	for (size_t i = 0; i < 16; ++i)
	{
		uint32_t cnt = mem_arm7_get_reg32(apu->mem, MEM_ARM7_REG_SOUNDXCNT(i));
		if (!(cnt & (1 << 31)))
			continue;
		struct apu_channel *channel = &apu->channels[i];
		channel->clock += cycles;
		while (channel->clock >= 0x10000)
		{
			channel->clock -= 0x10000;
			channel->clock += channel->tmr;
			switch ((cnt >> 29) & 0x3)
			{
				case 0:
				{
					uint8_t v = mem_arm7_get16(apu->mem,
					                            channel->sad + channel->pos / 2,
					                            MEM_DIRECT);
#if 0
					printf("8bit: [0x%08" PRIx32 "] = 0x%02" PRIx8 "\n",
					       channel->sad + channel->pos, v);
#endif
					channel->sample = (int8_t)v * 256;
					channel->pos += 2;
					break;
				}
				case 1:
				{
					uint16_t v = mem_arm7_get16(apu->mem,
					                            channel->sad + channel->pos / 2,
					                            MEM_DIRECT);
#if 0
					printf("16bit: [0x%08" PRIx32 "] = 0x%04" PRIx16 "\n",
					       channel->sad + channel->pos, v);
#endif
					channel->sample = (int16_t)v;
					channel->pos += 4;
					break;
				}
				case 2:
				{
					if (channel->pos == channel->pnt)
					{
						channel->adpcm_init_idx = channel->adpcm_idx;
						channel->adpcm_init_sample = channel->sample;
					}
					uint8_t v = mem_arm7_get8(apu->mem,
					                          channel->sad + channel->pos / 2,
					                          MEM_DIRECT);
#if 0
					printf("adpcm: [0x%08" PRIx32 "] = 0x%02" PRIx8 "\n",
					       channel->sad + channel->pos / 2, v);
#endif
					if (channel->pos & 1)
						v >>= 4;
					else
						v &= 0xF;
					int32_t diff = adpcm_table[channel->adpcm_idx] / 8;
					if (v & 0x1)
						diff += adpcm_table[channel->adpcm_idx] / 4;
					if (v & 0x2)
						diff += adpcm_table[channel->adpcm_idx] / 2;
					if (v & 0x4)
						diff += adpcm_table[channel->adpcm_idx] / 1;
					if (v & 0x8)
					{
						int32_t tmp = channel->sample - diff;
						channel->sample = tmp < -0x7FFF ? -0x7FFF : tmp;
					}
					else
					{
						int32_t tmp = channel->sample + diff;
						channel->sample = tmp > 0x7FFF ? 0x7FFF : tmp;
					}
					int32_t tmp = channel->adpcm_idx + adpcm_index_table[v & 0x7];
					if (tmp < 0)
						channel->adpcm_idx = 0;
					else if (tmp > 88)
						channel->adpcm_idx = 88;
					else
						channel->adpcm_idx = tmp;
					channel->pos += 1;
					break;
				}
				case 3:
					if (i >= 8 && i <= 13)
					{
						channel->pos = (channel->pos + 1) % 8;
						if (channel->pos < ((cnt >> 24) & 0x7))
							channel->sample = INT16_MIN;
						else
							channel->sample = INT16_MAX;
					}
					else if (i >= 14)
					{
						uint8_t carry = channel->pos & 0x1;
						channel->pos >>= 1;
						if (carry)
						{
							channel->pos ^= 0x6000;
							channel->sample = INT16_MIN;
						}
						else
						{
							channel->sample = INT16_MAX;
						}
					}
					continue; /* no timeout */
			}
			if (channel->pos >= channel->len + channel->pnt)
			{
				switch ((cnt >> 27) & 0x3)
				{
					case 0:
					case 2:
					case 3:
						mem_arm7_set_reg32(apu->mem, MEM_ARM7_REG_SOUNDXCNT(i),
						                   mem_arm7_get_reg32(apu->mem, MEM_ARM7_REG_SOUNDXCNT(i)) & ~(1 << 31));
						break;
					case 1:
						channel->pos = channel->pnt;
						if (((cnt >> 29) & 0x3) == 0x2)
						{
							channel->adpcm_idx = channel->adpcm_init_idx;
							channel->sample = channel->adpcm_init_sample;
						}
						break;
				}
			}
		}
	}
}

void apu_start_channel(struct apu *apu, uint8_t id)
{
	struct apu_channel *channel = &apu->channels[id];
	channel->pnt = mem_arm7_get_reg16(apu->mem, MEM_ARM7_REG_SOUNDXPNT(id)) * 8;
	channel->tmr = mem_arm7_get_reg16(apu->mem, MEM_ARM7_REG_SOUNDXTMR(id));
	channel->sad = mem_arm7_get_reg32(apu->mem, MEM_ARM7_REG_SOUNDXSAD(id));
	channel->len = (mem_arm7_get_reg32(apu->mem, MEM_ARM7_REG_SOUNDXLEN(id)) & 0x3FFFFF) * 8;
	channel->clock = channel->tmr;
	switch (mem_arm7_get_reg32(apu->mem, MEM_ARM7_REG_SOUNDXCNT(id) >> 29) & 0x3)
	{
		case 0x0:
		case 0x1:
			channel->sample = 0;
			channel->pos = 0;
			break;
		case 0x2:
		{
			uint32_t adpcm_hdr = mem_arm7_get32(apu->mem, channel->sad, MEM_DIRECT);
			channel->sample = (int16_t)(uint16_t)(adpcm_hdr & 0xFFFF);
			channel->adpcm_idx = (adpcm_hdr >> 16) & 0x7F;
			if (channel->adpcm_idx > 88)
				channel->adpcm_idx = 88;
			channel->pos = 8;
			break;
		}
		case 0x3:
			channel->sample = 0;
			channel->pos = 0x7FFF;
			break;
	}
#if 0
	printf("APU start %u: CNT=%08" PRIx32 " SAD=%08" PRIx32 " TMR=%04" PRIx16 " PNT=%04" PRIx16 " LEN=%08" PRIx32 "\n",
	       id, mem_arm7_get_reg32(apu->mem, MEM_ARM7_REG_SOUNDXCNT(id)),
	       channel->sad, channel->tmr, channel->pnt, channel->len);
#endif
}
