#include "_rand.h"

uint32_t rand_val;
uint16_t rand48_x[3];
uint64_t rand48_a = 0x5DEECE66D;
uint64_t rand48_c = 0xB;
static uint32_t random_val_default;
uint32_t *random_val = &random_val_default;

uint64_t rand48(uint16_t x[3])
{
	uint64_t v = ((uint64_t)x[0] << 0)
	           | ((uint64_t)x[1] << 16)
	           | ((uint64_t)x[2] << 32);
	v = (v * rand48_a + rand48_c) % (1ULL << 48);
	x[0] = v >> 0;
	x[1] = v >> 16;
	x[2] = v >> 32;
	return v;
}
