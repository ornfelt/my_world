#include <unistd.h>
#include <errno.h>

long
sysconf(int name)
{
	/* XXX */
	switch (name)
	{
		case _SC_ARG_MAX:
			return _POSIX_ARG_MAX;
		case _SC_CHILD_MAX:
			return _POSIX_CHILD_MAX;
		case _SC_HOST_NAME_MAX:
			return _POSIX_HOST_NAME_MAX;
		case _SC_LOGIN_NAME_MAX:
			return _POSIX_LOGIN_NAME_MAX;
		case _SC_NGROUPS_MAX:
			return _POSIX_NGROUPS_MAX;
		case _SC_CLK_TCK:
			return 1000000;
		case _SC_OPEN_MAX:
			return _POSIX_OPEN_MAX;
		case _SC_PAGESIZE:
		case _SC_PAGE_SIZE:
			return 4096;
		case _SC_RE_DUP_MAX:
			return _POSIX2_RE_DUP_MAX;
		case _SC_STREAM_MAX:
			return _POSIX_STREAM_MAX;
		case _SC_SYMLOOP_MAX:
			return _POSIX_SYMLOOP_MAX;
		case _SC_TTY_NAME_MAX:
			return _POSIX_TTY_NAME_MAX;
		case _SC_TZNAME_MAX:
			return _POSIX_TZNAME_MAX;
		case _SC_VERSION:
			return 199009L;
	}
	errno = EINVAL;
	return -1;
}
