#include <string.h>
#include <regex.h>
#include <stdio.h>

int
regexec(const regex_t *regex,
        const char *str,
        size_t nmatch,
        regmatch_t *matches,
        int flags)
{
	char *it;

	(void)flags;
	it = strstr(str, regex->regex);
	if (!it)
		return REG_NOMATCH;
	if (nmatch)
	{
		matches[0].rm_so = it - str;
		matches[0].rm_eo = matches[0].rm_so + regex->regex_len;
	}
	return 0;
}
