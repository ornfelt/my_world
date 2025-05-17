#include "_rand.h"

#include <stdlib.h>

long
nrand48(uint16_t x[3])
{
	return rand48(x) & 0x7FFFFFFF;
}
