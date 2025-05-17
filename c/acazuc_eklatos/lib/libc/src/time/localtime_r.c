#include <time.h>

struct tm *
localtime_r(const time_t *timep, struct tm *result)
{
	/* XXX use timezone */
	return gmtime_r(timep, result);
}
