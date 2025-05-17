#include "_rand.h"

#include <stdlib.h>

long
mrand48(void)
{
	return jrand48(rand48_x);
}
