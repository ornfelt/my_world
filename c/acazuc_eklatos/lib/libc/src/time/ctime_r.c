#include <time.h>

char *
ctime_r(const time_t *timep, char *buf)
{
	struct tm tm;

	localtime_r(timep, &tm);
	return asctime_r(&tm, buf);
}
