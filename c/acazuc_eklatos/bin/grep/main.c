#include <sys/stat.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <regex.h>

#define OPT_R (1 << 0)
#define OPT_i (1 << 1)
#define OPT_v (1 << 2)
#define OPT_n (1 << 3)
#define OPT_H (1 << 4)
#define OPT_l (1 << 5)
#define OPT_L (1 << 6)
#define OPT_o (1 << 7)
#define OPT_c (1 << 8)
#define OPT_s (1 << 9)
#define OPT_q (1 << 10)
#define OPT_A (1 << 11)
#define OPT_B (1 << 12)
#define OPT_Z (1 << 13)

struct match
{
	size_t start;
	size_t len;
};

struct before_line
{
	char *line;
	struct match match;
};

enum color_id
{
	COLOR_SEPARATOR,
	COLOR_PATH,
	COLOR_LINE,
	COLOR_MATCH,
	COLOR_RESET,
	COLOR_LAST,
};

enum regex_type
{
	REGEX_STR,
	REGEX_BRE,
	REGEX_ERE,
};

struct pattern
{
	regex_t regex;
	char *str;
	size_t len;
};

struct env
{
	const char *progname;
	enum regex_type regex_type;
	int opt;
	int multiple;
	regmatch_t *regmatches;
	size_t regmatches_nb;
	struct pattern *patterns;
	size_t patterns_nb;
	int has_written;
	int colored;
	char path_marks[2];
	char line_marks[2];
	const char *colors[COLOR_LAST];
	unsigned long before;
	unsigned long after;
	unsigned long max;
	struct before_line *before_lines; /* XXX this could (and should)
	                                   * be done by storing offsets
	                                   * in file on seekable files
	                                   */
};

static size_t getline_no_nl(char **line, size_t *len, FILE *fp)
{
	ssize_t ret = getline(line, len, fp);
	if (ret <= 0)
		return 0;
	if ((*line)[ret - 1] == '\n')
		(*line)[ret - 1] = '\0';
	return 1;
}

static int match_line(struct env *env, const char *line, struct match *match)
{
	int matched = 0;
	if (match)
	{
		match->start = SIZE_MAX;
		match->len = SIZE_MAX;
	}
	switch (env->regex_type)
	{
		case REGEX_ERE:
		case REGEX_BRE:
			for (size_t i = 0; i < env->patterns_nb; ++i)
			{
				struct pattern *pattern = &env->patterns[i];
				int ret = regexec(&pattern->regex, line,
				                  env->regmatches_nb,
				                  env->regmatches, 0);
				if (!!ret == !(env->opt & OPT_v))
					continue;
				if (match && !ret)
				{
					match->start = env->regmatches[0].rm_so;
					match->len = env->regmatches[0].rm_eo - match->start;
				}
				matched = !ret;
				break;
			}
			break;
		case REGEX_STR:
			matched = 0;
			for (size_t i = 0; i < env->patterns_nb; ++i)
			{
				struct pattern *pattern = &env->patterns[i];
				char *it;
				if (env->opt & OPT_i)
					it = strcasestr(line, pattern->str);
				else
					it = strstr(line, pattern->str);
				if (!it != !!(env->opt & OPT_v))
					continue;
				if (match && it)
				{
					match->start = it - line;
					match->len = pattern->len;
				}
				matched = !!it;
				break;
			}
			break;
		default:
			return 0;
	}
	if (matched && (env->opt & OPT_q))
		exit(EXIT_SUCCESS);
	return matched;
}

static void print_line(struct env *env, const char *path,
                       const char *line, struct match *match,
                       size_t n)
{
	if ((env->opt & OPT_q) || ((env->opt & OPT_o) && match->start == SIZE_MAX))
		return;
	int colored = 0;
	if (env->multiple || (env->opt & OPT_H))
	{
		if (env->colored)
			printf("%s%s%s%c",
			      env->colors[COLOR_PATH], path,
			      env->colors[COLOR_SEPARATOR], env->path_marks[match->start != SIZE_MAX]);
		else
			printf("%s%c", path, env->path_marks[match->start != SIZE_MAX]);
		colored = 1;
	}
	if (env->opt & OPT_n)
	{
		if (env->colored)
			printf("%s%zu%s%c",
			       env->colors[COLOR_LINE], n + 1,
			       env->colors[COLOR_SEPARATOR], env->line_marks[match->start != SIZE_MAX]);
		else
			printf("%zu%c", n + 1, env->line_marks[match->start != SIZE_MAX]);
		colored = 1;
	}
	if (env->colored && colored)
		printf("%s", env->colors[COLOR_RESET]);
	if (env->opt & OPT_o)
	{
		if (env->colored)
			printf("%s%.*s%s\n",
			       env->colors[COLOR_MATCH],
			       (int)match->len, &line[match->start],
			       env->colors[COLOR_RESET]);
		else
			printf("%.*s\n", (int)match->len, &line[match->start]);
	}
	else
	{
		if (env->colored && match->start != SIZE_MAX)
			printf("%.*s%s%.*s%s%s\n",
			       (int)match->start, line,
			       env->colors[COLOR_MATCH],
			       (int)match->len, &line[match->start],
			       env->colors[COLOR_RESET],
			       &line[match->start + match->len]);
		else
			printf("%s\n", line);
	}
}

static void print_separator(struct env *env)
{
	if (!env->has_written)
		return;
	if (env->colored)
		printf("%s--%s\n",
		       env->colors[COLOR_SEPARATOR],
		       env->colors[COLOR_RESET]);
	else
		printf("--\n");
}

static int grep_normal(struct env *env, FILE *fp, const char *path)
{
	char *line = NULL;
	size_t len = 0;
	size_t n = 0;
	size_t after = 0;
	size_t last_before = 0;
	size_t match_count = 0;

	while (getline_no_nl(&line, &len, fp))
	{
		struct match match;
		if (match_line(env, line, &match) && match_count < env->max)
		{
			if (env->after > 0)
				after = env->after;
		}
		else if (after)
		{
			after--;
		}
		else
		{
			if (match_count >= env->max)
				return 0;
			if (env->before > 0)
			{
				struct before_line *before_line = &env->before_lines[n % env->before];
				free(before_line->line);
				before_line->line = strdup(line);
				before_line->match = match;
				if (!before_line->line)
				{
					fprintf(stderr, "%s: malloc: %s\n", env->progname,
					        strerror(errno));
					return 1;
				}
			}
			n++;
			continue;
		}
		if (env->opt & OPT_B)
		{
			size_t before_start = last_before;
			if (env->before < n - last_before)
			{
				print_separator(env);
				before_start = n - env->before;
			}
			else if (!last_before)
			{
				print_separator(env);
			}
			for (size_t i = before_start; i < n; ++i)
			{
				struct before_line *before_line = &env->before_lines[i % env->before];
				print_line(env, path, before_line->line, &before_line->match, i);
				env->has_written = 1;
			}
		}
		else if (env->opt & OPT_A)
		{
			if (!last_before || last_before != n)
				print_separator(env);
		}
		print_line(env, path, line, &match, n);
		env->has_written = 1;
		n++;
		match_count++;
		last_before = n;
	}
	return ferror(fp);
}

static int grep_filename(struct env *env, FILE *fp, const char *path)
{
	char *line = NULL;
	size_t len = 0;

	while (getline_no_nl(&line, &len, fp))
	{
		int matched = match_line(env, line, NULL);
		if (((env->opt & OPT_l) && matched)
		 || ((env->opt & OPT_L) && !matched))
		{
			if (env->colored)
				printf("%s%s%s\n",
				       env->colors[COLOR_PATH], path,
				       env->colors[COLOR_RESET]);
			else
				printf("%s\n", path);
			return 0;
		}
	}
	return ferror(fp);
}

static int grep_count(struct env *env, FILE *fp, const char *path)
{
	char *line = NULL;
	size_t len = 0;
	size_t match_count = 0;

	while (getline_no_nl(&line, &len, fp))
	{
		if (match_line(env, line, NULL))
		{
			match_count++;
			if (match_count >= env->max)
				break;
		}
	}
	if (ferror(fp))
		return 1;
	if (env->multiple || (env->opt & OPT_H))
	{
		if (env->colored)
			printf("%s%s%s%c%s",
			       env->colors[COLOR_PATH], path,
			       env->colors[COLOR_SEPARATOR],
			       env->path_marks[1],
			       env->colors[COLOR_RESET]);
		else
			printf("%s:", path);
	}
	printf("%zu\n", match_count);
	return 0;
}

static int grep_fp(struct env *env, FILE *fp, const char *path)
{
	if (!env->max)
		return 0;
	if (env->opt & (OPT_l | OPT_L))
		return grep_filename(env, fp, path);
	if (env->opt & OPT_c)
		return grep_count(env, fp, path);
	return grep_normal(env, fp, path);
}

static int is_dir(DIR *dir, struct dirent *dirent)
{
	switch (dirent->d_type)
	{
		case DT_DIR:
			return 1;
		case DT_LNK:
		case DT_UNKNOWN:
		{
			struct stat st;
			if (fstatat(dirfd(dir), dirent->d_name, &st, 0) == -1)
				return -1;
			return S_ISDIR(st.st_mode) != 0;
		}
		default:
			return 0;
	}
}

static int grep_fd(struct env *env, int fd, const char *path)
{
	FILE *fp;
	int ret;

	fp = fdopen(fd, "rb");
	if (!fp)
	{
		if (!(env->opt & OPT_s))
			fprintf(stderr, "%s: fdopen(%s): %s\n", env->progname,
			        path, strerror(errno));
		return 1;
	}
	ret = grep_fp(env, fp, path);
	fclose(fp);
	return ret;
}

static int grep_dir(struct env *env, int fd, const char *path)
{
	struct dirent *dirent;
	DIR *dir;
	int ret = 1;

	env->multiple = 1;
	dir = fdopendir(fd);
	if (!dir)
	{
		if (!(env->opt & OPT_s))
			fprintf(stderr, "%s: fdopendir(%s): %s\n", env->progname,
			        path, strerror(errno));
		goto end;
	}
	while ((dirent = readdir(dir)))
	{
		if (!strcmp(dirent->d_name, ".")
		 || !strcmp(dirent->d_name, ".."))
			continue;
		int tmp = is_dir(dir, dirent);
		if (tmp == -1)
			goto end;
		int cfd = openat(fd, dirent->d_name, O_RDONLY, 0);
		if (cfd == -1)
		{
			if (!(env->opt & OPT_s))
				fprintf(stderr, "%s: openat(%s/%s): %s",
				        env->progname, path, dirent->d_name,
				        strerror(errno));
			continue;
		}
		char cpath[MAXPATHLEN];
		snprintf(cpath, sizeof(cpath), "%s/%s", path,
		         dirent->d_name);
		if (tmp)
			grep_dir(env, cfd, cpath);
		else
			grep_fd(env, cfd, cpath);
		close(cfd);
	}
	ret = 0;

end:
	closedir(dir);
	return ret;
}

static int grep_file(struct env *env, const char *path)
{
	struct stat st;
	int fd = -1;
	int ret = 1;

	fd = open(path, O_RDONLY);
	if (fd == -1)
	{
		if (!(env->opt & OPT_s))
			fprintf(stderr, "%s: open(%s): %s\n", env->progname,
			        path, strerror(errno));
		goto end;
	}
	if (fstat(fd, &st) == -1)
	{
		if (!(env->opt & OPT_s))
			fprintf(stderr, "%s: stat(%s): %s\n", env->progname,
			        path, strerror(errno));
		goto end;
	}
	if (S_ISDIR(st.st_mode))
	{
		if (env->opt & OPT_R)
			ret = grep_dir(env, fd, path);
		goto end;
	}
	ret = grep_fd(env, fd, path);

end:
	if (fd != -1)
		close(fd);
	return ret;
}

static int add_pattern(struct env *env, const char *str)
{
	struct pattern *new_patterns;
	char *dup;

	if (!*str)
		return 0;
	dup = strdup(str);
	if (!dup)
	{
		fprintf(stderr, "%s: malloc: %s\n", env->progname, strerror(errno));
		return 1;
	}
	new_patterns = realloc(env->patterns,
	                       sizeof(*new_patterns) * (env->patterns_nb + 1));
	if (!new_patterns)
	{
		fprintf(stderr, "%s: realloc: %s\n", env->progname, strerror(errno));
		free(dup);
		return 1;
	}
	new_patterns[env->patterns_nb].str = dup;
	new_patterns[env->patterns_nb].len = strlen(dup);
	env->patterns = new_patterns;
	env->patterns_nb++;
	return 0;
}

static int add_pattern_file(struct env *env, const char *filename)
{
	char *line = NULL;
	size_t len = 0;
	FILE *fp;
	int ret = 1;

	fp = fopen(filename, "r");
	if (!fp)
	{
		fprintf(stderr, "%s: open(%s): %s\n", env->progname, filename,
		        strerror(errno));
		goto end;
	}
	while (getline_no_nl(&line, &len, fp))
	{
		if (add_pattern(env, line))
			goto end;
	}
	if (!ferror(fp))
		ret = 0;

end:
	if (fp)
		fclose(fp);
	return ret;
}

static void usage(const char *progname)
{
	printf("%s [-h] [-R] [-r] [-i] [-v] [-n] [-H] [-A num] [-B num] [-C num] [-l] [-L] [-o] [-c] [-s] [-q] [-m num] [-Z] [-E] [-F] [-G] [-e pattern] PATTERN FILES\n", progname);
	printf("-h        : show this help\n");
	printf("-R        : recursively search in directories\n");
	printf("-r        : alias of -R\n");
	printf("-i        : do case-incensitive matching\n");
	printf("-v        : print non-matching lines\n");
	printf("-n        : display line number before matched text\n");
	printf("-H        : display file name before matched text\n");
	printf("-A num    : display num lines of context after matched line\n");
	printf("-B num    : display num lines of context before matched line\n");
	printf("-C num    : display num lines of context before and after matched line\n");
	printf("-l        : display only the name of matching files\n");
	printf("-L        : display only the name of non-matching files\n");
	printf("-o        : display only the matched parts of the lines\n");
	printf("-c        : display the number of matching lines per file\n");
	printf("-s        : don't output message on error\n");
	printf("-q        : don't output anything; return 0 if a match is found\n");
	printf("-m num    : stop after matching num lines\n");
	printf("-Z        : output a NULL byte after file names\n");
	printf("-E        : interpret patterns as extended regular expression format\n");
	printf("-F        : interpret patterns as fixed strings\n");
	printf("-G        : interpret patterns as basic regular expression format\n");
	printf("-e pattern: add this pattern to the list\n");
	printf("-f file   : use file as pattern list (one pattern per line)\n");
}

int main(int argc, char **argv)
{
	struct env env;
	int has_pattern = 0;
	int c;

	memset(&env, 0, sizeof(env));
	env.colored = isatty(1);
	if (env.colored)
	{
		env.colors[COLOR_SEPARATOR] = "\033[0;36m";
		env.colors[COLOR_PATH] = "\033[0;35m";
		env.colors[COLOR_LINE] = "\033[0;32m";
		env.colors[COLOR_MATCH] = "\033[1;31m";
		env.colors[COLOR_RESET] = "\033[0m";
	}
	env.max = ULONG_MAX;
	env.progname = argv[0];
	env.regex_type = REGEX_BRE;
	while ((c = getopt(argc, argv, "hRrivHnA:B:C:lLocsqm:ZEFGe:f:")) != -1)
	{
		switch (c)
		{
			case 'r':
			case 'R':
				env.opt |= OPT_R;
				break;
			case 'i':
				env.opt |= OPT_i;
				break;
			case 'v':
				env.opt |= OPT_v;
				break;
			case 'n':
				env.opt |= OPT_n;
				break;
			case 'H':
				env.opt |= OPT_H;
				break;
			case 'A':
			{
				errno = 0;
				char *endptr;
				env.after = strtoul(optarg, &endptr, 0);
				if (errno || *endptr)
				{
					fprintf(stderr, "%s: invalid line count\n", argv[0]);
					return EXIT_FAILURE;
				}
				env.opt |= OPT_A;
				break;
			}
			case 'B':
			{
				errno = 0;
				char *endptr;
				env.before = strtoul(optarg, &endptr, 0);
				if (errno || *endptr)
				{
					fprintf(stderr, "%s: invalid line count\n", argv[0]);
					return EXIT_FAILURE;
				}
				env.opt |= OPT_B;
				break;
			}
			case 'C':
			{
				errno = 0;
				char *endptr;
				env.after = strtoul(optarg, &endptr, 0);
				if (errno || *endptr)
				{
					fprintf(stderr, "%s: invalid line count\n", argv[0]);
					return EXIT_FAILURE;
				}
				env.before = env.after;
				env.opt |= OPT_A | OPT_B;
				break;
			}
			case 'l':
				env.opt &= ~(OPT_L | OPT_c);
				env.opt |= OPT_l;
				break;
			case 'L':
				env.opt &= ~(OPT_l | OPT_c);
				env.opt |= OPT_L;
				break;
			case 'o':
				env.opt |= OPT_o;
				break;
			case 'c':
				env.opt &= ~(OPT_L | OPT_l);
				env.opt |= OPT_c;
				break;
			case 's':
				env.opt |= OPT_s;
				break;
			case 'q':
				env.opt |= OPT_q | OPT_s;
				break;
			case 'Z':
				env.opt |= OPT_Z;
				break;
			case 'm':
			{
				errno = 0;
				char *endptr;
				env.max = strtoul(optarg, &endptr, 0);
				if (errno || *endptr)
				{
					fprintf(stderr, "%s: invalid match count\n", argv[0]);
					return EXIT_FAILURE;
				}
				break;
			}
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			case 'E':
				env.regex_type = REGEX_ERE;
				break;
			case 'F':
				env.regex_type = REGEX_STR;
				break;
			case 'G':
				env.regex_type = REGEX_BRE;
				break;
			case 'e':
				has_pattern = 1;
				if (add_pattern(&env, optarg))
					return EXIT_FAILURE;
				break;
			case 'f':
				has_pattern = 1;
				if (add_pattern_file(&env, optarg))
					return EXIT_FAILURE;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (!has_pattern)
	{
		if (optind == argc)
		{
			fprintf(stderr, "%s: missing operand\n", argv[0]);
			return EXIT_FAILURE;
		}
		if (add_pattern(&env, argv[optind]))
			return EXIT_FAILURE;
		optind++;
	}
	if (!env.patterns_nb)
	{
		fprintf(stderr, "%s: no pattern given\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (!(env.opt & OPT_c) && env.opt & OPT_v)
	{
		env.path_marks[0] = ':';
		env.path_marks[1] = '-';
		env.line_marks[0] = ':';
		env.line_marks[1] = '-';
	}
	else
	{
		env.path_marks[0] = '-';
		env.path_marks[1] = ':';
		env.line_marks[0] = '-';
		env.line_marks[1] = ':';
	}
	if (env.opt & OPT_Z)
	{
		env.path_marks[0] = '\0';
		env.path_marks[1] = '\0';
	}
	if (env.before > 0)
	{
		env.before_lines = calloc(env.before, sizeof(*env.before_lines));
		if (!env.before_lines)
		{
			fprintf(stderr, "%s: malloc: %s\n", argv[0], strerror(errno));
			return EXIT_FAILURE;
		}
	}
	if (env.regex_type == REGEX_BRE || env.regex_type == REGEX_ERE)
	{
		for (size_t i = 0; i < env.patterns_nb; ++i)
		{
			struct pattern *pattern = &env.patterns[i];
			int ret = regcomp(&pattern->regex, pattern->str,
			                  (env.opt & OPT_i) ? REG_ICASE : 0);
			if (ret)
			{
				char buf[1024];
				regerror(ret, &pattern->regex, buf, sizeof(buf));
				fprintf(stderr, "%s: regcomp(%s): %s\n", argv[0],
				        pattern->str, buf);
				return EXIT_FAILURE;
			}
			if (pattern->regex.re_nsub > env.regmatches_nb)
				env.regmatches_nb = pattern->regex.re_nsub;
		}
		env.regmatches_nb++;
		env.regmatches = malloc(sizeof(*env.regmatches) * env.regmatches_nb);
		if (!env.regmatches)
		{
			fprintf(stderr, "%s: malloc: %s\n", argv[0], strerror(errno));
			return EXIT_FAILURE;
		}
	}
	if (optind == argc)
	{
		if (env.opt & OPT_R)
		{
			if (grep_file(&env, "."))
				return EXIT_FAILURE;
		}
		else
		{
			if (grep_fp(&env, stdin, "stdin"))
				return EXIT_FAILURE;
		}
	}
	else
	{
		env.multiple = (argc - optind) > 1;
		for (int i = optind; i < argc; ++i)
		{
			if (grep_file(&env, argv[i]))
			{
				if (!env.multiple)
					return EXIT_FAILURE;
			}
		}
	}
	if (env.opt & OPT_q)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
