#include "_rand.h"

#include <stdlib.h>

void
srand48(long seed)
{
	rand48_a = 0x5DEECE66D;
	rand48_c = 0xB;
	rand48_x[0] = 0x330E;
	rand48_x[1] = seed;
	rand48_x[2] = seed >> 16;
}
