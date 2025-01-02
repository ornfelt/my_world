#include "apu.h"
#include "mem.h"
#include <stdlib.h>

static uint8_t duties[4] = {1, 2, 4, 6};

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
	uint16_t cnt_l = mem_get_reg16(apu->mem, MEM_REG_SOUND3CNT_L);
	if (!(cnt_l & (1 << 7)))
		return 0;
	static uint8_t gain_shifts[8] = {8, 0, 1, 2};
	uint8_t pos = apu->wave3_val & 0x1F;
	if (cnt_l & (1 << 6))
		pos += 0x20;
	uint8_t sample = mem_get_wave4(apu->mem, pos);
	sample = sample | (sample << 4);
	uint8_t gain = mem_get_reg16(apu->mem, MEM_REG_SOUND3CNT_H) >> 13;
	if (gain & 0x4)
		return (sample >> 2) + (sample >> 4);
	return sample >> gain_shifts[gain & 0x3];
}

static uint8_t channel4(struct apu *apu)
{
	uint8_t gain = apu->wave4_env.val;
	gain |= gain << 4;
	return (apu->wave4_val & 1) ? gain : 0;
}

static uint16_t gen_sample(struct apu *apu)
{
	static const uint8_t psg_gains[4] = {2, 1, 0, 0};
	uint16_t cnt_x = mem_get_reg16(apu->mem, MEM_REG_SOUNDCNT_X);
	if (!(cnt_x & (1 << 7)))
		return 0;
	uint16_t cnt_l = mem_get_reg16(apu->mem, MEM_REG_SOUNDCNT_L);
	uint16_t cnt_h = mem_get_reg16(apu->mem, MEM_REG_SOUNDCNT_H);
	uint16_t bias = mem_get_reg16(apu->mem, MEM_REG_SOUNDBIAS);
	uint16_t l = 0;
	uint16_t r = 0;
	uint8_t values[4] = {0};
	if (cnt_x & (1 << 0))
	{
		values[0] = channel1(apu) >> 4;
		if (cnt_l & (1 << 4))
			l += values[0];
		if (cnt_l & (1 << 0))
			r += values[0];
	}
	if (cnt_x & (1 << 1))
	{
		values[1] = channel2(apu) >> 4;
		if (cnt_l & (1 << 5))
			l += values[1];
		if (cnt_l & (1 << 1))
			r += values[1];
	}
	if (cnt_x & (1 << 2))
	{
		values[2] = channel3(apu) >> 4;
		if (cnt_l & (1 << 6))
			l += values[2];
		if (cnt_l & (1 << 2))
			r += values[2];
	}
	if (cnt_x & (1 << 3))
	{
		values[3] = channel4(apu) >> 4;
		if (cnt_l & (1 << 7))
			l += values[3];
		if (cnt_l & (1 << 3))
			r += values[3];
	}
	l = l * ((cnt_l & 0x70) >> 4) / 7;
	r = r * ((cnt_l & 0x07) >> 0) / 7;
	uint8_t psg_gain = psg_gains[cnt_h & 0x3];
	l = l >> psg_gain;
	r = r >> psg_gain;
	uint8_t bias_level = (bias >> 2) & 0xFF;
	uint8_t fifo1 = apu->fifo1_val + bias_level;
	uint8_t fifo2 = apu->fifo2_val + bias_level;
	fifo1 >>= 1;
	if (!(cnt_h & (1 << 2)))
		fifo1 >>= 1;
	fifo2 >>= 1;
	if (!(cnt_h & (1 << 3)))
		fifo2 >>= 1;
	if (cnt_h & (1 << 9))
		l += fifo1;
	if (cnt_h & (1 << 8))
		r += fifo1;
	if (cnt_h & (1 << 13))
		l += fifo2;
	if (cnt_h & (1 << 12))
		r += fifo2;
	return (l << 8) | r;
}

static void length_tick(struct apu *apu)
{
	uint16_t cnt_x = mem_get_reg16(apu->mem, MEM_REG_SOUNDCNT_X);
	if ((cnt_x & (1 << 0)) && apu->wave1_haslen)
	{
		if (!apu->wave1_len)
			cnt_x &= ~(1 << 0);
		else
			apu->wave1_len--;
	}
	if ((cnt_x & (1 << 1)) && apu->wave2_haslen)
	{
		if (!apu->wave2_len)
			cnt_x &= ~(1 << 1);
		else
			apu->wave2_len--;
	}
	if ((cnt_x & (1 << 2)) && apu->wave3_haslen)
	{
		if (!apu->wave3_len)
			cnt_x &= ~(1 << 2);
		else
			apu->wave3_len = 0;
	}
	if ((cnt_x & (1 << 3)) && apu->wave4_haslen)
	{
		if (!apu->wave4_len)
			cnt_x &= ~(1 << 3);
		else
			apu->wave4_len--;
	}
	mem_set_reg16(apu->mem, MEM_REG_SOUNDCNT_X, cnt_x);
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
	uint16_t cnt_x = mem_get_reg16(apu->mem, MEM_REG_SOUNDCNT_X);
	if (cnt_x & (1 << 0))
	{
		if (update_env(&apu->wave1_env))
			cnt_x &= ~(1 << 0);
	}
	if (cnt_x & (1 << 1))
	{
		if (update_env(&apu->wave2_env))
			cnt_x &= ~(1 << 1);
	}
	if (cnt_x & (1 << 3))
	{
		if (update_env(&apu->wave4_env))
			cnt_x &= ~(1 << 3);
	}
	mem_set_reg16(apu->mem, MEM_REG_SOUNDCNT_X, cnt_x);
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
	if (!(mem_get_reg16(apu->mem, MEM_REG_SOUNDCNT_X) & (1 << 3)))
		return;
	if (++apu->wave4_cnt >= apu->wave4_nb)
	{
		if (apu->wave4_cycle > 32767)
		{
			apu->wave4_cycle = 0;
			apu->wave4_val = 0xFFFF;
		}
		apu->wave4_cycle++;

		uint8_t width = mem_get_reg16(apu->mem, MEM_REG_SOUND4CNT_H) & (1 << 3);
		apu->wave4_cnt = 0;
		uint8_t xored = (apu->wave4_val ^ (apu->wave4_val >> 1)) & 1;
		apu->wave4_val >>= 1;
		if (!width)
			apu->wave4_val = (apu->wave4_val & 0x3FFF) | (xored << 14);
		else
			apu->wave4_val = (apu->wave4_val & 0x3F) | (xored << 6);
	}
}

void apu_cycle(struct apu *apu)
{
	uint32_t timer = ++apu->timer;

	if (timer & 0x3)
		return;

	timer >>= 2;
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
	mem_set_reg16(apu->mem, MEM_REG_SOUNDCNT_X, mem_get_reg16(apu->mem, MEM_REG_SOUNDCNT_X) | (1 << 0));
	uint16_t cnt_l = mem_get_reg16(apu->mem, MEM_REG_SOUND1CNT_L);
	uint16_t cnt_h = mem_get_reg16(apu->mem, MEM_REG_SOUND1CNT_H);
	uint16_t cnt_x = mem_get_reg16(apu->mem, MEM_REG_SOUND1CNT_X);
	apu->wave1_swp.step = 0;
	apu->wave1_swp.time = (cnt_l >> 4) & 0x7;
	apu->wave1_swp.cnt = cnt_l & 0x7;
	apu->wave1_swp.dir = (cnt_l >> 3) & 0x1;
	apu->wave1_swp.nb = cnt_l & 0x7;
	apu->wave1_env.step = 0;
	apu->wave1_env.time = (cnt_h >> 8) & 0x7;
	apu->wave1_env.val = (cnt_h >> 12) & 0xF;
	apu->wave1_env.dir = (cnt_h >> 11) & 0x1;
	apu->wave1_nb = (1 << 2) * (2048 - (cnt_x & 0x7FF));
	apu->wave1_len = 64 - (cnt_h & 0x3F);
	apu->wave1_haslen = (cnt_x >> 14) & 1;
	apu->wave1_duty = (cnt_h >> 6) & 0x3;
}

void apu_start_channel2(struct apu *apu)
{
	mem_set_reg16(apu->mem, MEM_REG_SOUNDCNT_X, mem_get_reg16(apu->mem, MEM_REG_SOUNDCNT_X) | (1 << 1));
	uint16_t cnt_l = mem_get_reg16(apu->mem, MEM_REG_SOUND2CNT_L);
	uint16_t cnt_h = mem_get_reg16(apu->mem, MEM_REG_SOUND2CNT_H);
	apu->wave2_env.step = 0;
	apu->wave2_env.time = (cnt_l >> 8) & 0x7;
	apu->wave2_env.val = (cnt_l >> 12) & 0xF;
	apu->wave2_env.dir = (cnt_l >> 11) & 0x1;
	apu->wave2_nb = (1 << 2) * (2048 - (cnt_h & 0x7FF));
	apu->wave2_len = 64 - (cnt_l & 0x3F);
	apu->wave2_haslen = (cnt_h >> 14) & 1;
	apu->wave2_duty = (cnt_l >> 6) & 0x3;
}

void apu_start_channel3(struct apu *apu)
{
	mem_set_reg16(apu->mem, MEM_REG_SOUNDCNT_X, mem_get_reg16(apu->mem, MEM_REG_SOUNDCNT_X) | (1 << 2));
	uint16_t cnt_h = mem_get_reg16(apu->mem, MEM_REG_SOUND3CNT_H);
	uint16_t cnt_x = mem_get_reg16(apu->mem, MEM_REG_SOUND3CNT_X);
	apu->wave3_nb = 2048 - (cnt_x & 0x7FF);
	apu->wave3_len = 256 - (cnt_h & 0xFF);
	apu->wave3_haslen = (cnt_x >> 14) & 0x1;
}

void apu_start_channel4(struct apu *apu)
{
	mem_set_reg16(apu->mem, MEM_REG_SOUNDCNT_X, mem_get_reg16(apu->mem, MEM_REG_SOUNDCNT_X) | (1 << 3));
	uint16_t cnt_l = mem_get_reg16(apu->mem, MEM_REG_SOUND4CNT_L);
	uint16_t cnt_h = mem_get_reg16(apu->mem, MEM_REG_SOUND4CNT_H);
	apu->wave4_env.step = 0;
	apu->wave4_env.time = (cnt_l >> 8) & 0x7;
	apu->wave4_env.val = (cnt_l >> 12) & 0xF;
	apu->wave4_env.dir = (cnt_l >> 11) & 0x1;
	apu->wave4_val = 0xFF;
	uint8_t r = cnt_h & 0x7;
	uint8_t s = (cnt_h >> 4) & 0xF;
	if (r)
		apu->wave4_nb = (1u << 3) * (r * (1u << (s + 1)));
	else
		apu->wave4_nb = (1u << 3) * (1u << s);
	apu->wave4_len = 64 - (cnt_l & 0x3F);
	apu->wave4_haslen = (cnt_h >> 14) & 0x1;
}
