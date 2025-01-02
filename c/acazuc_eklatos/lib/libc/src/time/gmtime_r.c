#include <errno.h>
#include <time.h>

static const int g_mdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/* not perfect but close enough implementation */
struct tm *gmtime_r(const time_t *timep, struct tm *tm)
{
	time_t t = *timep;
	if (t < -2208988800 || t > 253402300799) /* < 1900 || > 9999 */
	{
		errno = EINVAL;
		return NULL;
	}
	/* XXX handle negative */
	tm->tm_sec = t % 60;
	t /= 60;
	tm->tm_min = t % 60;
	t /= 60;
	tm->tm_hour = t % 24;
	t /= 24;
	tm->tm_wday = (t + 4) % 7;
	tm->tm_year = 70;
	/* handle the 3 non-leap years every 100 years */
	size_t megaleapcnt = t / (365 * 400 + 97);
	tm->tm_year += megaleapcnt * 400;
	t %= 365 * 400 + 97;
	/* handle classical leap years every 4 years */
	size_t leapcnt = t / (365 * 4 + 1);
	tm->tm_year += leapcnt * 4;
	t %= 365 * 4 + 1;
	tm->tm_year += t / 365;
	t %= 365;
	tm->tm_yday = t;
	for (int i = 0; i < 12; ++i)
	{
		int mdays;
		if (i == 1)
		{
			if (tm->tm_year % 4 == 0
			 && (tm->tm_year % 100 != 0 || tm->tm_year % 400 == 0))
				mdays = 29;
			else
				mdays = 28;
		}
		else
		{
			mdays = g_mdays[i];
		}
		if (t >= mdays)
		{
			t -= mdays;
			continue;
		}
		tm->tm_mday = t + 1;
		tm->tm_mon = i;
		break;
	}
	return tm;
}
