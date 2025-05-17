#include <stdio.h>
#include <errno.h>
#include <time.h>

char *
asctime_r(const struct tm *tm, char *buf)
{
	static const char *days[] =
	{
		"Sun",
		"Mon",
		"Tue",
		"Wed",
		"Thu",
		"Fri",
		"Sat",
	};
	static const char *months[] =
	{
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
		"Nov",
		"Dec",
	};
	if (tm->tm_sec < 0
	 || tm->tm_sec > 60
	 || tm->tm_min < 0
	 || tm->tm_min > 59
	 || tm->tm_hour < 0
	 || tm->tm_hour > 23
	 || tm->tm_mday < 1
	 || tm->tm_mday > 32
	 || tm->tm_mon < 0
	 || tm->tm_mon > 11
	 || tm->tm_year < -1900
	 || tm->tm_year >= 8100
	 || tm->tm_wday < 0
	 || tm->tm_wday > 6
	 || tm->tm_yday < 0
	 || tm->tm_yday > 365)
	{
		errno = EINVAL;
		return NULL;
	}
	sprintf(buf, "%.3s %.3s %2d %02d:%02d:%02d %4d\n",
	        days[tm->tm_wday],
	        months[tm->tm_mon],
	        tm->tm_mday,
	        tm->tm_hour,
	        tm->tm_min,
	        tm->tm_sec,
	        tm->tm_year + 1900);
	return buf;
}
