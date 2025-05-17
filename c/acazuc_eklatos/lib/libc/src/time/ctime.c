#include <time.h>

char *
ctime(const time_t *timep)
{
	static char buf[128];
	return ctime_r(timep, buf);
}
