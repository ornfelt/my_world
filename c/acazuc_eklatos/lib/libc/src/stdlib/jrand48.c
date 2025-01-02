#include "_rand.h"

#include <stdlib.h>

long jrand48(uint16_t x[3])
{
	return (int32_t)(uint32_t)rand48(x);
}
