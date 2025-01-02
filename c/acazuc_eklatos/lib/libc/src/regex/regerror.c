#include <regex.h>

size_t regerror(int err, const regex_t *regex, char *buf, size_t size)
{
	(void)err;
	(void)regex;
	(void)buf;
	(void)size;
	return 0;
}
