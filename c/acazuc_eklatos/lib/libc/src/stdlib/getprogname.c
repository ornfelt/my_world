#include <stdlib.h>

const char *__libc_progname;

const char *getprogname(void)
{
	return __libc_progname;
}
