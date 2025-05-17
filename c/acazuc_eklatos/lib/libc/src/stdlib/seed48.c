#include "_rand.h"

#include <stdlib.h>

uint16_t *
seed48(uint16_t v[3])
{
	static uint16_t old_v[3];

	rand48_a = 0x5DEECE66D;
	rand48_c = 0xB;
	old_v[0] = rand48_x[0];
	old_v[1] = rand48_x[1];
	old_v[2] = rand48_x[2];
	rand48_x[0] = v[0];
	rand48_x[1] = v[1];
	rand48_x[2] = v[2];
	return old_v;
}
