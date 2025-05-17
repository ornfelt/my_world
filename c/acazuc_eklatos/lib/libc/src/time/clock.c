#include <time.h>

clock_t
clock(void)
{
	return time(NULL) * CLOCKS_PER_SEC; /* XXX :D */
}
