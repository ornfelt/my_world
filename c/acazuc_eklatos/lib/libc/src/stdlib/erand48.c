#include "_rand.h"

#include <stdlib.h>

double erand48(uint16_t x[3])
{
	return rand48(x) / 1e48;
}
