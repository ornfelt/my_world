#ifndef APU_H
#define APU_H

#include <stdint.h>

#define APU_FRAME_SAMPLES 803

struct mem;

struct apu_channel
{
	uint16_t tmr;
	uint16_t pnt;
	uint32_t sad;
	uint32_t len;
	uint32_t pos; /* in 4 bits units, for adpcm simplicity */
	uint32_t clock;
	int16_t adpcm_init_sample;
	int16_t sample;
	uint8_t adpcm_init_idx;
	uint8_t adpcm_idx;
};

struct apu
{
	int16_t *data;
	struct apu_channel channels[16];
	struct mem *mem;
	uint32_t clock;
	uint32_t sample;
	uint32_t next_sample;
};

struct apu *apu_new(struct mem *mem);
void apu_del(struct apu *apu);

void apu_cycles(struct apu *cpu, uint32_t cycles);
void apu_sample(struct apu *apu, uint32_t cycles);

void apu_start_channel(struct apu *apu, uint8_t channel);

#endif
