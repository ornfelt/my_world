#include <time.h>

time_t mktime(struct tm *tm)
{
	return tm->tm_sec + tm->tm_min * 60 + tm->tm_hour * 3600
	     + tm->tm_yday * 86400 + (tm->tm_year - 70) * 31536000
	     + ((tm->tm_year - 69) / 4) * 86400
	     - ((tm->tm_year - 1) / 100) * 86400
	     + ((tm->tm_year + 299) / 400) * 86400;
}
