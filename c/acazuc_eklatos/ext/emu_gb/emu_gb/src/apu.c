#include "apu.h"
#include "mem.h"

#include <stdlib.h>
#include <stdio.h>

static uint8_t duties[4] = {1, 2, 4, 6};

struct apu *apu_new(struct mem *mem)
{
	struct apu *apu = calloc(sizeof(*apu), 1);
	if (!apu)
	{
		fprintf(stderr, "allocation failed\n");
		return NULL;
	}

	apu->mem = mem;
	return apu;
}

void apu_del(struct apu *apu)
{
	if (!apu)
		return;
	free(apu);
}

static uint8_t channel1(struct apu *apu)
{
	uint8_t gain = apu->wave1_env.val;
	gain |= gain << 4;
	return ((apu->wave1_val & 0x7) < duties[apu->wave1_duty]) ? gain : 0;
}

static uint8_t channel2(struct apu *apu)
{
	uint8_t gain = apu->wave2_env.val;
	gain |= gain << 4;
	return ((apu->wave2_val & 0x7) < duties[apu->wave2_duty]) ? gain : 0;
}

static uint8_t channel3(struct apu *apu)
{
	if (!mem_get_reg(apu->mem, MEM_REG_NR30))
		return 0;
	static uint8_t gain_shifts[4] = {8, 0, 1, 2};
	uint8_t pos = apu->wave3_val & 0x1F;
	uint8_t sample = mem_get_reg(apu->mem, MEM_REG_WAVE + pos / 2);
	if (pos & 1)
		sample >>= 4;
	sample &= 0xF;
	return (sample | (sample << 4)) >> gain_shifts[(mem_get_reg(apu->mem, MEM_REG_NR32) >> 5) & 0x3];
}

static uint8_t channel4(struct apu *apu)
{
	uint8_t gain = apu->wave4_env.val;
	gain |= gain << 4;
	return (apu->wave4_val & 1) ? gain : 0;
}

static uint16_t gen_sample(struct apu *apu)
{
	uint8_t nr52 = mem_get_reg(apu->mem, MEM_REG_NR52);
	if (!(nr52 & (1 << 7)))
		return 0;
	uint8_t nr50 = mem_get_reg(apu->mem, MEM_REG_NR50);
	uint8_t nr51 = mem_get_reg(apu->mem, MEM_REG_NR51);
	uint16_t l = 0;
	uint16_t r = 0;
	uint8_t values[4] = {0};
	if (nr52 & (1 << 0))
	{
		values[0] = channel1(apu) >> 2;
		if (nr51 & (1 << 4))
			l += values[0];
		if (nr51 & (1 << 0))
			r += values[0];
	}
	if (nr52 & (1 << 1))
	{
		values[1] = channel2(apu) >> 2;
		if (nr51 & (1 << 5))
			l += values[1];
		if (nr51 & (1 << 1))
			r += values[1];
	}
	if (nr52 & (1 << 2))
	{
		values[2] = channel3(apu) >> 2;
		if (nr51 & (1 << 6))
			l += values[2];
		if (nr51 & (1 << 2))
			r += values[2];
	}
	if (nr52 & (1 << 3))
	{
		values[3] = channel4(apu) >> 2;
		if (nr51 & (1 << 7))
			l += values[3];
		if (nr51 & (1 << 3))
			r += values[3];
	}
	l = l * ((nr50 & 0x70) >> 4) / 7;
	r = r * ((nr50 & 0x07) >> 0) / 7;
	mem_set_reg(apu->mem, MEM_REG_PCM1, (values[1] & 0xF0) | (values[0] >> 4));
	mem_set_reg(apu->mem, MEM_REG_PCM2, (values[3] & 0xF0) | (values[2] >> 4));
	return (l << 8) | r;
}

static void length_tick(struct apu *apu)
{
	uint8_t nr52 = mem_get_reg(apu->mem, MEM_REG_NR52);
	if ((nr52 & (1 << 0)) && apu->wave1_haslen)
	{
		if (!apu->wave1_len)
			mem_set_reg(apu->mem, MEM_REG_NR52, nr52 & ~(1 << 0));
		else
			apu->wave1_len--;
	}
	if ((nr52 & (1 << 1)) && apu->wave2_haslen)
	{
		if (!apu->wave2_len)
			mem_set_reg(apu->mem, MEM_REG_NR52, nr52 & ~(1 << 1));
		else
			apu->wave2_len--;
	}
	if ((nr52 & (1 << 2)) && apu->wave3_haslen)
	{
		if (!apu->wave3_len)
			mem_set_reg(apu->mem, MEM_REG_NR52, nr52 & ~(1 << 2));
		else
			apu->wave3_len = 0;
	}
	if ((nr52 & (1 << 3)) && apu->wave4_haslen)
	{
		if (!apu->wave4_len)
			mem_set_reg(apu->mem, MEM_REG_NR52, nr52 & ~(1 << 3));
		else
			apu->wave4_len--;
	}
}

static void swp_tick(struct apu *apu)
{
	if (apu->wave1_swp.nb)
	{
		if (++apu->wave1_swp.step >= apu->wave1_swp.time)
		{
			uint8_t diff = apu->wave1_nb >> (1 + apu->wave1_swp.nb);
			if (apu->wave1_swp.dir)
				apu->wave1_nb -= diff;
			else
				apu->wave1_nb += diff;
			apu->wave1_swp.nb--;
		}
	}
}

static bool update_env(struct apu_env *env)
{
	if (!env->time)
		return false;
	if (++env->step < env->time)
		return false;
	if (env->dir)
	{
		if (env->val < 0xF)
			env->val++;
	}
	else
	{
		if (env->val > 0)
		{
			env->val--;
			if (!env->val)
				return true;
		}
	}
	env->step = 0;
	return false;
}

static void env_tick(struct apu *apu)
{
	uint8_t nr52 = mem_get_reg(apu->mem, MEM_REG_NR52);
	if (nr52 & (1 << 0))
	{
		if (update_env(&apu->wave1_env))
			nr52 &= ~(1 << 0);
	}
	if (nr52 & (1 << 1))
	{
		if (update_env(&apu->wave2_env))
			nr52 &= ~(1 << 1);
	}
	if (nr52 & (1 << 3))
	{
		if (update_env(&apu->wave4_env))
			nr52 &= ~(1 << 3);
	}
	mem_set_reg(apu->mem, MEM_REG_NR52, nr52);
}

static void update_channel1(struct apu *apu)
{
	if (++apu->wave1_cnt >= apu->wave1_nb)
	{
		apu->wave1_cnt = 0;
		apu->wave1_val++;
	}
}

static void update_channel2(struct apu *apu)
{
	if (++apu->wave2_cnt >= apu->wave2_nb)
	{
		apu->wave2_cnt = 0;
		apu->wave2_val++;
	}
}

static void update_channel3(struct apu *apu)
{
	if (++apu->wave3_cnt >= apu->wave3_nb)
	{
		apu->wave3_cnt = 0;
		apu->wave3_val++;
	}
}

static void update_channel4(struct apu *apu)
{
	if (!(mem_get_reg(apu->mem, MEM_REG_NR52) & (1 << 3)))
		return;
	if (++apu->wave4_cnt >= apu->wave4_nb)
	{
		if (apu->wave4_cycle > 32767)
		{
			apu->wave4_cycle = 0;
			apu->wave4_val = 0xFFFF;
		}
		apu->wave4_cycle++;

		uint8_t width = mem_get_reg(apu->mem, MEM_REG_NR43) & (1 << 3);
		apu->wave4_cnt = 0;
		uint8_t xored = (apu->wave4_val ^ (apu->wave4_val >> 1)) & 1;
		apu->wave4_val >>= 1;
		if (!width)
			apu->wave4_val = (apu->wave4_val & 0x3FFF) | (xored << 14);
		else
			apu->wave4_val = (apu->wave4_val & 0x3F) | (xored << 6);
	}
}

void apu_clock(struct apu *apu)
{
	uint16_t timer = apu->mem->timer;

	if ((timer & 0x3FFF) == 0)
		length_tick(apu);

	if ((timer & 0x7FFF) == 0)
		swp_tick(apu);

	if ((timer & 0xFFFF) == 0)
		env_tick(apu);

	update_channel1(apu);
	update_channel2(apu);
	update_channel3(apu);
	update_channel4(apu);

	if (apu->clock % 70224 == apu->sample * 70224 / APU_FRAME_SAMPLES)
	{
		apu->data[apu->sample] = gen_sample(apu);
		apu->sample = (apu->sample + 1) % APU_FRAME_SAMPLES;
	}

	apu->clock++;
}

void apu_start_channel1(struct apu *apu)
{
	mem_set_reg(apu->mem, MEM_REG_NR52, mem_get_reg(apu->mem, MEM_REG_NR52) | (1 << 0));
	uint8_t nr10 = mem_get_reg(apu->mem, MEM_REG_NR10);
	uint8_t nr11 = mem_get_reg(apu->mem, MEM_REG_NR11);
	uint8_t nr12 = mem_get_reg(apu->mem, MEM_REG_NR12);
	uint8_t nr13 = mem_get_reg(apu->mem, MEM_REG_NR13);
	uint8_t nr14 = mem_get_reg(apu->mem, MEM_REG_NR14);
	apu->wave1_swp.step = 0;
	apu->wave1_swp.time = (nr10 >> 4) & 0x7;
	apu->wave1_swp.cnt = nr10 & 0x7;
	apu->wave1_swp.dir = (nr10 >> 3) & 0x1;
	apu->wave1_swp.nb = nr10 & 0x7;
	apu->wave1_env.step = 0;
	apu->wave1_env.time = nr12 & 0x7;
	apu->wave1_env.val = (nr12 >> 4) & 0xF;
	apu->wave1_env.dir = (nr12 >> 3) & 0x1;
	apu->wave1_nb = (1 << 2) * (2048 - (nr13 | ((nr14 & 0x7) << 8)));
	apu->wave1_len = 64 - (nr11 & 0x3F);
	apu->wave1_haslen = nr14 & (1 << 6);
	apu->wave1_duty = (nr11 >> 6) & 0x3;
}

void apu_start_channel2(struct apu *apu)
{
	mem_set_reg(apu->mem, MEM_REG_NR52, mem_get_reg(apu->mem, MEM_REG_NR52) | (1 << 1));
	uint8_t nr21 = mem_get_reg(apu->mem, MEM_REG_NR21);
	uint8_t nr22 = mem_get_reg(apu->mem, MEM_REG_NR22);
	uint8_t nr23 = mem_get_reg(apu->mem, MEM_REG_NR23);
	uint8_t nr24 = mem_get_reg(apu->mem, MEM_REG_NR24);
	apu->wave2_env.step = 0;
	apu->wave2_env.time = nr22 & 0x7;
	apu->wave2_env.val = (nr22 >> 4) & 0xF;
	apu->wave2_env.dir = (nr22 >> 3) & 0x1;
	apu->wave2_nb = (1 << 2) * (2048 - (nr23 | ((nr24 & 0x7) << 8)));
	apu->wave2_len = 64 - (nr21 & 0x3F);
	apu->wave2_haslen = nr24 & (1 << 6);
	apu->wave2_duty = (nr21 >> 6) & 0x3;
}

void apu_start_channel3(struct apu *apu)
{
	mem_set_reg(apu->mem, MEM_REG_NR52, mem_get_reg(apu->mem, MEM_REG_NR52) | (1 << 2));
	uint8_t nr31 = mem_get_reg(apu->mem, MEM_REG_NR31);
	uint8_t nr33 = mem_get_reg(apu->mem, MEM_REG_NR33);
	uint8_t nr34 = mem_get_reg(apu->mem, MEM_REG_NR34);
	apu->wave3_nb = (2048 - (nr33 | ((nr34 & 0x7) << 8)));
	apu->wave3_len = 256 - nr31;
	apu->wave3_haslen = nr34 & (1 << 6);
}

void apu_start_channel4(struct apu *apu)
{
	mem_set_reg(apu->mem, MEM_REG_NR52, mem_get_reg(apu->mem, MEM_REG_NR52) | (1 << 3));
	uint8_t nr41 = mem_get_reg(apu->mem, MEM_REG_NR41);
	uint8_t nr42 = mem_get_reg(apu->mem, MEM_REG_NR42);
	uint8_t nr43 = mem_get_reg(apu->mem, MEM_REG_NR43);
	uint8_t nr44 = mem_get_reg(apu->mem, MEM_REG_NR44);
	apu->wave4_env.step = 0;
	apu->wave4_env.time = nr42 & 0x7;
	apu->wave4_env.val = (nr42 >> 4) & 0xF;
	apu->wave4_env.dir = (nr42 >> 3) & 0x1;
	apu->wave4_val = 0xFF;
	uint8_t r = nr43 & 0x7;
	uint8_t s = (nr43 >> 4) & 0xF;
	if (r)
		apu->wave4_nb = (1u << 3) * (r * (1u << (s + 1)));
	else
		apu->wave4_nb = (1u << 3) * (1u << s);
	apu->wave4_len = 64 - (nr41 & 0x3F);
	apu->wave4_haslen = nr44 & (1 << 6);
}
