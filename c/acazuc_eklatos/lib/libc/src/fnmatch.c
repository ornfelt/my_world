#include <fnmatch.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <ctype.h>

static int class_match(const char *pattern, size_t *i, char c)
{
	int neg = 0;
	if (pattern[*i] == '!')
	{
		neg = 1;
		(*i)++;
	}
	if (pattern[*i] == ']')
	{
		(*i)++;
		if (c == ']')
			goto end;
	}
	while (1)
	{
		if (!pattern[*i])
			return 1;
		if (pattern[*i] == ']')
			return !neg;
		if (pattern[*i] == '[' && pattern[*i + 1] == ':')
		{
#define TEST_PATTERN(name) \
do \
{ \
	if (!strncmp(&pattern[*i], "[:" #name ":]", sizeof(#name) + 3)) \
	{ \
		*i += sizeof(#name) + 3; \
		if (is##name(c)) \
			goto end; \
		continue; \
	} \
} while (0)

			TEST_PATTERN(alnum);
			TEST_PATTERN(alpha);
			TEST_PATTERN(blank);
			TEST_PATTERN(cntrl);
			TEST_PATTERN(digit);
			TEST_PATTERN(graph);
			TEST_PATTERN(lower);
			TEST_PATTERN(print);
			TEST_PATTERN(punct);
			TEST_PATTERN(space);
			TEST_PATTERN(upper);
			TEST_PATTERN(xdigit);

#undef TEST_PATTERN
			continue;
		}
		if (pattern[*i + 1] == '-' && pattern[*i + 2] != ']')
		{
			char first = pattern[*i];
			char last = pattern[*i + 2];
			(*i) += 3;
			if (first > last)
			{
				char tmp = first;
				first = last;
				last = tmp;
			}
			if (c >= first && c <= last)
				goto end;
			continue;
		}
		if (c == pattern[*i])
			break;
		(*i)++;
	}
end:
	while (1)
	{
		char tmp = pattern[*i];
		if (!tmp)
			return 1;
		if (tmp == ']')
			return neg;
		(*i)++;
	}
}

static int match(const char *pattern, const char *str, int flags)
{
	size_t n = 0;
	for (size_t i = 0; pattern[i]; ++i)
	{
		if (pattern[i] == '\\' && !(flags & FNM_NOESCAPE))
		{
			i++;
			if (pattern[i] != str[n])
				return FNM_NOMATCH;
			n++;
			continue;
		}
		if (pattern[i] == '?')
		{
			if (!str[n])
				return FNM_NOMATCH;
			if (str[n] == '/' && (flags & FNM_PATHNAME))
				return FNM_NOMATCH;
			n++;
			continue;
		}
		if (pattern[i] == '*')
		{
			do
			{
				i++;
			} while (pattern[i] == '*');
			if (pattern[i] == '\0')
			{
				if (!(flags & FNM_PATHNAME))
					return 0;
				while (str[n])
				{
					if (str[n] == '/')
						return FNM_NOMATCH;
				}
				return 0;
			}
			int found = 0;
			while (str[n])
			{
				if (str[n] == pattern[i])
				{
					if (!match(&pattern[i + 1], &str[n + 1], flags))
					{
						found = 1;
						n++;
						break;
					}
				}
				if (str[n] == '/' && (flags & FNM_PATHNAME))
					return FNM_NOMATCH;
				n++;
			}
			if (!found)
				return FNM_NOMATCH;
			continue;
		}
		if (pattern[i] == '[')
		{
			if (!str[n])
				return FNM_NOMATCH;
			if (str[n] == '/' && (flags & FNM_PATHNAME))
				return FNM_NOMATCH;
			i++;
			if (class_match(pattern, &i, str[n]))
				return FNM_NOMATCH; /* XXX another error ? */
			n++;
			continue;
		}
		if (pattern[i] != str[n])
			return FNM_NOMATCH;
		n++;
	}
	if (str[n])
		return FNM_NOMATCH;
	return 0;
}

int fnmatch(const char *pattern, const char *str, int flags)
{
	if ((flags & FNM_PERIOD) && str[0] == '.')
	{
		if (pattern[0] != '.'
		 && ((flags & FNM_NOESCAPE)
		  || pattern[0] != '\\'
		  || pattern[1] != '.'))
			return FNM_NOMATCH;
	}
	return match(pattern, str, flags);
}
