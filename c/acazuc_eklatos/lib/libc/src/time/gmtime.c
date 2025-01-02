#include <time.h>

struct tm *gmtime(const time_t *timep)
{
	static struct tm result;
	return gmtime_r(timep, &result);
}
