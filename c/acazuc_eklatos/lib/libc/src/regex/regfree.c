#include <stdlib.h>
#include <regex.h>

void regfree(regex_t *regex)
{
	free(regex->regex);
	regex->regex = NULL;
}
