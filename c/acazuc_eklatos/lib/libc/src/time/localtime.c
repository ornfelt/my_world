#include <time.h>

struct tm *
localtime(const time_t *timep)
{
	static struct tm result;

	return localtime_r(timep, &result);
}
