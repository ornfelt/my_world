#include "_rand.h"

#include <stdlib.h>

void lcong48(uint16_t param[7])
{
	rand48_x[0] = param[0];
	rand48_x[1] = param[1];
	rand48_x[2] = param[2];
	rand48_a = ((uint64_t)param[3] << 0)
	         | ((uint64_t)param[4] << 16)
	         | ((uint64_t)param[5] << 32);
	rand48_c = param[6];
}
