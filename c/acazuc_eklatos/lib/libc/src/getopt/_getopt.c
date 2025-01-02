#include "_getopt.h"

#include <string.h>
#include <stdio.h>

char *optarg;
int optind = 1;
int opterr = 1;
int optopt;

static const struct option *parse_long(int argc, char * const argv[],
                                       const struct option *longopts,
                                       int longopt_type, char *cur)
{
	if (*cur == '-')
	{
		if (longopt_type != LONGOPT_ONLY)
			return NULL;
		cur++;
	}
	char *eq = strchr(cur, '=');
	if (eq)
	{
		for (size_t i = 0; longopts[i].name; ++i)
		{
			const struct option *opt = &longopts[i];
			if (strlen(opt->name) != (size_t)(eq - cur)
			 || strcmp(opt->name, cur))
				continue;
			if (opt->has_arg == no_argument) /* XXX error ? */
				return opt;
			optarg = eq + 1;
			return opt;
		}
		return NULL;
	}
	for (size_t i = 0; longopts[i].name; ++i)
	{
		const struct option *opt = &longopts[i];
		if (strcmp(opt->name, cur))
			continue;
		if (opt->has_arg == no_argument)
			return opt;
		if (opt->has_arg == required_argument)
		{
			optind++;
			if (optind >= argc)
			{
				if (opterr)
					fprintf(stderr, "%s: option requires an argument -- '%s'\n", argv[0], opt->name);
				return (struct option*)-1;
			}
			optarg = argv[optind];
			return opt;
		}
		if (opt->has_arg == optional_argument)
		{
			if (optind + 1 >= argc)
			{
				optarg = NULL;
				return opt;
			}
			optind++;
			optarg = argv[optind];
			return opt;
		}
		optind++;
		if (opterr)
			fprintf(stderr, "%s: invalid option has_arg -- '%s'\n",
			        argv[0], opt->name);
		return (struct option*)-1;
	}
	return NULL;
}

int getopt_impl(int argc, char * const argv[], const char *optstring,
                const struct option *longopts, int *longindex,
                int longopt_type)
{
	static char *cur = NULL;
	if (optind >= argc)
		return -1;
	if (!cur)
	{
		cur = argv[optind];
		if (!cur
		 || cur[0] != '-'
		 || !strcmp(cur, "-")
		 || !strcmp(cur, "--"))
			return -1;
		cur++;
		if (longopt_type != LONGOPT_NONE)
		{
			const struct option *matched;
			matched = parse_long(argc, argv, longopts, longopt_type,
			                     cur);
			if (matched == (struct option*)-1)
			{
				cur = NULL;
				return '?';
			}
			if (matched)
			{
				cur = NULL;
				optind++;
				if (longindex)
					*longindex = matched - longopts;
				if (matched->flag)
				{
					*matched->flag = matched->val;
					return 0;
				}
				return matched->val;
			}
		}
	}
	char optc = *cur;
	char *optptr = strchr(optstring, optc);
	if (!optptr)
	{
		if (opterr)
			fprintf(stderr, "%s: illegal option -- '%c'\n",
			        argv[0], optc);
		optopt = optc;
		optind++;
		cur = NULL;
		return '?';
	}
	if (optptr[1] != ':')
	{
		cur++;
		if (!*cur)
		{
			cur = NULL;
			optind++;
		}
		return optc;
	}
	if (cur[1])
	{
		optind++;
		optarg = &cur[1];
		cur = NULL;
		return *optptr;
	}
	optind++;
	if (optind >= argc)
	{
		if (opterr)
			fprintf(stderr, "%s: option requires an argument -- '%c'\n",
			        argv[0], optc);
		optopt = optc;
		cur = NULL;
		return '?';
	}
	optarg = argv[optind];
	optind++;
	cur = NULL;
	return optc;
}
