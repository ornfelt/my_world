#include <assert.h>
#include <stdio.h>

void __assert_fail(const char *expr, const char *file, unsigned line,
                   const char *func)
{
	fprintf(stderr, "%s:%d: %s Assertion `%s' failed.\n", file, line, func,
	        expr);
	abort();
}
