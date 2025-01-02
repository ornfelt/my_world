#include "internal.h"
#include <stdlib.h>

void apu_init(struct apu *apu, struct nes *nes)
{
	apu->nes = nes;
	apu->noise.value = 1;
}

static uint16_t generate_sample(struct apu *apu)
{
	static const uint8_t duties[] = {0x2, 0x4, 0x8, 0xC};
	uint16_t v = 0;
	if ((apu->status & (1 << 0)) && apu->wave1.frequency >= 8 && apu->wave1.length)
	{
		if (apu->wave1.value < duties[((apu->wave1.status >> 6) & 0x3)])
		{
			if (apu->wave1.status & (1 << 4))
				v += apu->wave1.status & 0xF;
			else
				v += apu->wave1.decay;
		}
	}
	if ((apu->status & (1 << 1)) && apu->wave2.frequency >= 8 && apu->wave2.length)
	{
		if (apu->wave2.value < duties[((apu->wave2.status >> 6) & 0x3)])
		{
			if (apu->wave2.status & (1 << 4))
				v += apu->wave2.status & 0xF;
			else
				v += apu->wave2.decay;
		}
	}
	if ((apu->status & (1 << 2)) && apu->triangle.length && (apu->triangle.status & 0x7F))
	{
		static const uint8_t triangle_form[] =
		{
			0x0, 0x1, 0x2, 0x3,
			0x4, 0x5, 0x6, 0x7,
			0x8, 0x9, 0xA, 0xB,
			0xC, 0xD, 0xE, 0xF,
			0xF, 0xE, 0xD, 0xC,
			0xB, 0xA, 0x9, 0x8,
			0x7, 0x6, 0x5, 0x4,
			0x3, 0x2, 0x1, 0x0,
		};
		v += triangle_form[apu->triangle.value];
	}
	if (apu->status & (1 << 3) && apu->noise.length)
	{
		if (apu->noise.value & 1)
		{
			if (apu->noise.status & (1 << 4))
				v += apu->noise.status & 0xF;
			else
				v += apu->noise.decay;
		}
	}
	return (v << 2) | (v >> 2);
}

static void update_wave(struct apu_wave *wave)
{
	if (wave->counter < wave->frequency)
	{
		wave->counter++;
		return;
	}
	wave->counter = 0;
	wave->value = (wave->value + 1) % 16;
}

static void update_triangle(struct apu_triangle *triangle)
{
	if (triangle->counter < triangle->frequency)
	{
		triangle->counter++;
		return;
	}
	triangle->counter = 0;
	triangle->value = (triangle->value + 1) % 32;
}

static void update_noise(struct apu_noise *noise)
{
	if (noise->counter / 2 < (noise->frequency & 0xF))
	{
		noise->counter++;
		return;
	}
	noise->counter = 0;
	if (noise->frequency & (1 << 7))
	{
		noise->value = (noise->value << 1) | (((noise->value >> 14) ^ (noise->value >> 8)) & 1);
	}
	else
	{
		noise->value = (noise->value << 1) | (((noise->value >> 14) ^ (noise->value >> 13)) & 1);
	}
}

static void apu_cycle(struct apu *apu)
{
	update_wave(&apu->wave1);
	update_wave(&apu->wave2);
	update_triangle(&apu->triangle);
	update_noise(&apu->noise);
}

static void apu_length(struct apu *apu)
{
	if (apu->wave1.status & (1 << 4))
	{
		if (!(apu->wave1.status & (1 << 5)))
		{
			if (apu->wave1.length)
				--apu->wave1.length;
		}
	}
	if (apu->wave1.length && (apu->wave1.sweep & (1 << 7)) && (apu->wave1.sweep & 0x7))
	{
		if (apu->wave1.sweep_count++ >= ((apu->wave1.sweep >> 4) & 0x7))
		{
			apu->wave1.sweep_count = 0;
			if (apu->wave1.sweep & (1 << 3))
				apu->wave1.frequency -= apu->wave1.frequency >> (apu->wave1.sweep & 0x7);
			else
				apu->wave1.frequency += apu->wave1.frequency >> (apu->wave1.sweep & 0x7);
		}
	}
	if (apu->wave2.status & (1 << 4))
	{
		if (!(apu->wave2.status & (1 << 5)))
		{
			if (apu->wave2.length)
				--apu->wave2.length;
		}
	}
	if (apu->wave2.length && (apu->wave2.sweep & (1 << 7)) && (apu->wave2.sweep & 0x7))
	{
		if (apu->wave2.sweep_count++ >= ((apu->wave2.sweep >> 4) & 0x7))
		{
			apu->wave2.sweep_count = 0;
			if (apu->wave2.sweep & (1 << 3))
				apu->wave2.frequency -= apu->wave2.frequency >> (apu->wave2.sweep & 0x7);
			else
				apu->wave2.frequency += apu->wave2.frequency >> (apu->wave2.sweep & 0x7);
		}
	}
	if (!(apu->triangle.status & (1 << 7)))
	{
		if (apu->triangle.length)
			--apu->triangle.length;
	}
	if (apu->noise.status & (1 << 4))
	{
		if (!(apu->noise.status & (1 << 5)))
		{
			if (apu->noise.length)
				--apu->noise.length;
		}
	}
}

static void apu_decay(struct apu *apu)
{
	if (!(apu->wave1.status & (1 << 4)))
	{
		if (apu->wave1.decay_count >= (apu->wave1.status & 0xF))
		{
			apu->wave1.decay_count = 0;
			if (apu->wave1.decay > 1)
				--apu->wave1.decay;
			else if (apu->wave1.status & (1 << 5))
				apu->wave1.decay = 0xF;
			else
				apu->wave1.length = 0;
		}
		else
		{
			apu->wave1.decay_count++;
		}
	}
	if (!(apu->wave2.status & (1 << 4)))
	{
		if (apu->wave2.decay_count >= (apu->wave2.status & 0xF))
		{
			apu->wave2.decay_count = 0;
			if (apu->wave2.decay > 1)
				--apu->wave2.decay;
			else if (apu->wave2.status & (1 << 5))
				apu->wave2.decay = 0xF;
			else
				apu->wave2.length = 0;
		}
		else
		{
			apu->wave2.decay_count++;
		}
	}
	if (apu->triangle.reload)
	{
		apu->triangle.linear = apu->triangle.status & 0x7F;
		apu->triangle.reload = 0;
	}
	else if (apu->triangle.linear)
	{
		apu->triangle.linear--;
	}
	if (!(apu->triangle.status & (1 << 7)))
		apu->triangle.reload = 1;
	if (!(apu->noise.status & (1 << 4)))
	{
		if (apu->noise.decay_count >= (apu->noise.status & 0xF))
		{
			apu->noise.decay_count = 0;
			if (apu->noise.decay)
				--apu->noise.decay;
			else if (apu->noise.status & (1 << 5))
				apu->noise.decay = 0xF;
			else
				apu->noise.length= 0;
		}
		else
		{
			apu->noise.decay_count++;
		}
	}
}

void apu_clock(struct apu *apu)
{
	/* 16 in pal, 12 in NTSC */
	if (++apu->clock_count == 12)
	{
		apu_cycle(apu);
		apu->clock_count = 0;
		++apu->frame_clock;
		if (apu->control & (1 << 7))
		{
			if (apu->frame_clock == 3728)
			{
				apu_decay(apu);
			}
			else if (apu->frame_clock == 7456)
			{
				apu_decay(apu);
				apu_length(apu);
			}
			else if (apu->frame_clock == 11185)
			{
				apu_decay(apu);
			}
			else if (apu->frame_clock == 18640)
			{
				apu_decay(apu);
				apu_length(apu);
			}
		}
		else
		{
			if (apu->frame_clock == 3729)
			{
				apu_decay(apu);
			}
			else if (apu->frame_clock == 7457)
			{
				apu_decay(apu);
				apu_length(apu);
			}
			else if (apu->frame_clock == 11186)
			{
				apu_decay(apu);
			}
			else if (apu->frame_clock == 14915)
			{
				apu_decay(apu);
				apu_length(apu);
			}
		}
		if (apu->frame_clock == 29830)
			apu->frame_clock = 0;
	}
	if (++apu->sample_clock == 446) /* 446.5, should be 446.71 */
	{
		if (apu->sample_count & 1)
			apu->sample_clock = -1;
		else
			apu->sample_clock = 0;
		if (apu->sample_count < 800)
			apu->data[apu->sample_count++] = generate_sample(apu);
	}
}
