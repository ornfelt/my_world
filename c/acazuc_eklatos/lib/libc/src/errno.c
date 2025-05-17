#include <errno.h>

static __thread int _errno;

int *
__get_errno(void)
{
	return &_errno;
}

void
__set_errno(int err)
{
	_errno = err;
}
