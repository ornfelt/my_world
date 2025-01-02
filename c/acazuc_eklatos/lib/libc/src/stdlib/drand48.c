#include "_rand.h"

#include <stdlib.h>

double drand48(void)
{
	return rand48(rand48_x) / 1e48;
}
