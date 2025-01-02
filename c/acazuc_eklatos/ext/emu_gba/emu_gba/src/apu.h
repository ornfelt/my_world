#ifndef APU_H
#define APU_H

#include <stdint.h>

#define APU_FRAME_SAMPLES 804

struct mem;

struct apu_swp
{
	uint8_t step;
	uint8_t time;
	uint8_t dir;
	uint8_t cnt;
	uint8_t nb;
};

struct apu_env
{
	uint8_t step;
	uint8_t time;
	uint8_t dir;
	uint8_t val;
};

struct apu
{
	uint16_t data[APU_FRAME_SAMPLES];
	uint32_t sample;
	uint32_t clock;
	uint8_t wave1_haslen;
	struct apu_swp wave1_swp;
	struct apu_env wave1_env;
	uint8_t wave1_duty;
	uint32_t wave1_len;
	uint32_t wave1_val;
	uint32_t wave1_cnt;
	uint32_t wave1_nb;
	uint8_t wave2_haslen;
	struct apu_env wave2_env;
	uint8_t wave2_duty;
	uint32_t wave2_len;
	uint32_t wave2_val;
	uint32_t wave2_cnt;
	uint32_t wave2_nb;
	uint8_t wave3_haslen;
	uint32_t wave3_len;
	uint32_t wave3_val;
	uint32_t wave3_cnt;
	uint32_t wave3_nb;
	uint8_t wave4_haslen;
	struct apu_env wave4_env;
	uint32_t wave4_cycle;
	uint32_t wave4_len;
	uint32_t wave4_val;
	uint32_t wave4_cnt;
	uint32_t wave4_nb;
	uint8_t fifo1_val;
	uint8_t fifo2_val;
	uint32_t timer;
	struct mem *mem;
};

struct apu *apu_new(struct mem *mem);
void apu_del(struct apu *apu);

void apu_cycle(struct apu *apu);

void apu_start_channel1(struct apu *apu);
void apu_start_channel2(struct apu *apu);
void apu_start_channel3(struct apu *apu);
void apu_start_channel4(struct apu *apu);

#endif
