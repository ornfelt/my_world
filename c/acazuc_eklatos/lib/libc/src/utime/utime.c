#include <sys/stat.h>

#include <utime.h>
#include <fcntl.h>

int
utime(const char *file, const struct utimbuf *times)
{
	struct timespec ts[2];

	ts[0].tv_sec = times[0].actime;
	ts[0].tv_nsec = 0;
	ts[1].tv_sec = times[0].modtime;
	ts[1].tv_nsec = 0;
	return utimensat(AT_FDCWD, file, ts, 0);
}
