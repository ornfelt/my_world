#include "sh.h"

#include <readline/readline.h>
#include <readline/history.h>

#include <sys/utsname.h>
#include <sys/param.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <pwd.h>

static struct sh *g_sh;

static void prompt_putnstr(char **buf, size_t *size, const char *s, size_t n)
{
	while (n)
	{
		if (*size < 2)
			return;
		**buf = *s;
		(*buf)++;
		(*size)--;
		s++;
		n--;
	}
}

static void prompt_putstr(char **buf, size_t *size, const char *s)
{
	while (*s)
	{
		if (*size < 2)
			return;
		**buf = *s;
		(*buf)++;
		(*size)--;
		s++;
	}
}

static void prompt_putchar(char **buf, size_t *size, char c)
{
	if (*size < 2)
		return;
	**buf = c;
	(*buf)++;
	(*size)--;
}

static void prompt_strftime(char **buf, size_t *size, const char *fmt)
{
	char tbuf[512];
	struct tm tm;
	time_t t = time(NULL);
	if (!localtime_r(&t, &tm))
		return; /* XXX display error somehow ? */
	strftime(tbuf, sizeof(tbuf), fmt, &tm);
	prompt_putstr(buf, size, tbuf);
}

static void mkprompt(char *buf, size_t size, const char *ps)
{
	for (; *ps; ps++)
	{
		if (*ps != '\\')
		{
			prompt_putchar(&buf, &size, *ps);
			continue;
		}
		char nxt = *(++ps);
		switch (nxt)
		{
			case '\\':
				prompt_putchar(&buf, &size, '\\');
				break;
			case 'a':
				prompt_putchar(&buf, &size, '\a');
				break;
			case 'd':
				prompt_strftime(&buf, &size, "%a %n %d");
				break;
			case 'D':
			{
				char nxt2 = *(++ps);
				if (nxt2 != '{')
					break; /* print error */
				const char *end = strchr(ps, '}');
				if (!end)
					break; /* print error */
				char fmt[512];
				if (snprintf(fmt, sizeof(fmt), "%.*s", (int)(end - ps - 1), ps + 1) >= (int)sizeof(fmt))
					break; /* print error */
				prompt_strftime(&buf, &size, fmt);
				ps = end + 1;
				break;
			}
			case 'e':
				prompt_putchar(&buf, &size, '\033');
				break;
			case 'h':
			{
				struct utsname utsbuf;
				if (uname(&utsbuf))
					break;
				const char *dot = strchr(utsbuf.machine, '.');
				if (dot)
					prompt_putnstr(&buf, &size, utsbuf.nodename, dot - utsbuf.nodename);
				else
					prompt_putstr(&buf, &size, utsbuf.nodename);
				break;
			}
			case 'H':
			{
				struct utsname utsbuf;
				if (uname(&utsbuf))
					break;
				prompt_putstr(&buf, &size, utsbuf.nodename);
				break;
			}
			case 'j':
				/* jobs count */
				break;
			case 'l':
				/* XXX */
				break;
			case 'n':
				prompt_putchar(&buf, &size, '\n');
				break;
			case 'r':
				prompt_putchar(&buf, &size, '\r');
				break;
			case 's':
				prompt_putstr(&buf, &size, "sh");
				break;
			case 't':
				prompt_strftime(&buf, &size, "%H:%M:%S");
				break;
			case 'T':
				prompt_strftime(&buf, &size, "%I:%M:%S");
				break;
			case '@':
				prompt_strftime(&buf, &size, "%I:%M");
				break;
			case 'A':
				prompt_strftime(&buf, &size, "%H:%M");
				break;
			case 'u':
			{
				uid_t uid = getuid();
				struct passwd *pw = getpwuid(uid);
				if (pw && pw->pw_name)
				{
					prompt_putstr(&buf, &size, pw->pw_name);
				}
				else
				{
					char tmp[64];
					snprintf(tmp, sizeof(tmp), "%d", (int)uid);
					prompt_putstr(&buf, &size, tmp);
				}
				break;
			}
			case 'v':
				prompt_putstr(&buf, &size, "0.01");
				break;
			case 'V':
				prompt_putstr(&buf, &size, "0.01");
				break;
			case 'w':
			{
				char pwd[MAXPATHLEN];
				if (!getcwd(pwd, sizeof(pwd)))
					break; /* XXX display error somehow ? */
				prompt_putstr(&buf, &size, pwd);
				break;
			}
			case 'W':
			{
				char *cwd = getenv("PWD");
				if (cwd)
					prompt_putstr(&buf, &size, cwd);
				break;
			}
			case '!':
				/* XXX history number */
				break;
			case '#':
				/* XXX command number */
				break;
			case '$':
				if (geteuid())
					prompt_putchar(&buf, &size, '$');
				else
					prompt_putchar(&buf, &size, '#');
				break;
			default:
				prompt_putchar(&buf, &size, nxt);
				break;
		}
	}
	*buf = '\0';
}

static int hist_up(void)
{
	HIST_ENTRY *entry = previous_history();
	if (!entry)
		return 0;
	rl_delete_text(0, rl_end);
	rl_insert_text(entry->line);
	return 0;
}

static int hist_down(void)
{
	HIST_ENTRY *entry = next_history();
	if (!entry)
	{
		rl_delete_text(0, rl_end);
		return 0;
	}
	rl_delete_text(0, rl_end);
	rl_insert_text(entry->line);
	return 0;
}

static void set_ps2(struct sh *sh)
{
	if (sh->is_ps2)
		return;
	const char *ps2 = getenv("PS2");
	mkprompt(sh->prompt, sizeof(sh->prompt), ps2 ? ps2 : "> ");
	sh->is_ps2 = 1;
}

static void set_ps1(struct sh *sh)
{
	if (!sh->is_ps2)
		return;
	const char *ps1 = getenv("PS1");
	mkprompt(sh->prompt, sizeof(sh->prompt), ps1 ? ps1 : "\\u@\\h:\\w\\$ ");
	sh->is_ps2 = 0;
}

static void sigint_handler(int signum)
{
	(void)signum;
	parse_reset(g_sh);
	printf("\n");
	if (!g_sh->fg_pipeline)
	{
		rl_delete_text(0, rl_end);
		set_ps1(g_sh);
		printf("%s", rl_prompt);
		fflush(stdout);
		return;
	}
	pid_t pgid = g_sh->fg_pipeline->pgid;
	if (pgid)
		kill(-pgid, SIGINT);
}

static int run_interactive(struct sh *sh)
{
	int ret = EXIT_FAILURE;

	if (isatty(0))
	{
		sh->tty_input = 1;
		setsid();
		tcsetpgrp(0, getpgrp());
	}
	signal(SIGINT, sigint_handler);
	using_history();
	stifle_history(100);
	sh->is_ps2 = 1;
	while (!feof(stdin))
	{
		enum parse_status status;
		set_ps1(sh);
		rl_generic_bind(ISFUNC, "\033[A", hist_up, rl_get_keymap());
		rl_generic_bind(ISFUNC, "\033[B", hist_down, rl_get_keymap());
		while (1)
		{
			char *line = readline(sh->prompt);
			if (!line)
			{
				fprintf(stderr, "%s: readline: %s\n", sh->progname,
				        strerror(errno));
				goto end;
			}
			if (line && *line && *line != ' ')
			{
				HIST_ENTRY *previous = previous_history();
				if (!previous || strcmp(previous->line, line))
					add_history(line);
			}
			while (next_history())
				;
			status = parse(sh, line);
			free(line);
			switch (status)
			{
				case PARSE_OK:
					set_ps1(sh);
					break;
				case PARSE_ERR:
					parse_reset(sh);
					status = PARSE_OK;
					break;
				case PARSE_NEED_SQUOTE:
				case PARSE_NEED_DQUOTE:
				case PARSE_NEED_BQUOTE:
				case PARSE_NEED_NEWLINE:
				case PARSE_NEED_WHILE:
				case PARSE_NEED_UNTIL:
				case PARSE_NEED_FOR:
				case PARSE_NEED_IF:
				case PARSE_NEED_CASE:
				case PARSE_NEED_GROUP:
				case PARSE_NEED_FN:
					set_ps2(sh);
					break;
			}
		}
	}
	ret = EXIT_SUCCESS;

end:
	return ret;
}

static void print_parse_error(struct sh *sh, enum parse_status status)
{
	switch (status)
	{
		case PARSE_OK:
			break;
		case PARSE_ERR:
			fprintf(stderr, "%s: invalid input\n",
			        sh->progname);
			break;
		case PARSE_NEED_SQUOTE:
			fprintf(stderr, "%s: unfinished squote\n",
			        sh->progname);
			break;
		case PARSE_NEED_DQUOTE:
			fprintf(stderr, "%s: unfinished dquote\n",
			        sh->progname);
			break;
		case PARSE_NEED_BQUOTE:
			fprintf(stderr, "%s: unfinished bquote\n",
			        sh->progname);
			break;
		case PARSE_NEED_NEWLINE:
			fprintf(stderr, "%s: expected EOL\n",
			        sh->progname);
			break;
		case PARSE_NEED_WHILE:
			fprintf(stderr, "%s: unfinished while\n",
			        sh->progname);
			break;
		case PARSE_NEED_UNTIL:
			fprintf(stderr, "%s: unfinished until\n",
			        sh->progname);
			break;
		case PARSE_NEED_FOR:
			fprintf(stderr, "%s: unfinished for\n",
			        sh->progname);
			break;
		case PARSE_NEED_IF:
			fprintf(stderr, "%s: unfinished if\n",
			        sh->progname);
			break;
		case PARSE_NEED_CASE:
			fprintf(stderr, "%s: unfinished case\n",
			        sh->progname);
			break;
		case PARSE_NEED_GROUP:
			fprintf(stderr, "%s: unfinished group\n",
			        sh->progname);
			break;
		case PARSE_NEED_FN:
			fprintf(stderr, "%s: unfinished function\n",
			        sh->progname);
			break;
	}
}

static int run_str(struct sh *sh, const char *str)
{
	enum parse_status status;
	int ret = EXIT_FAILURE;

	status = parse(sh, str);
	if (status != PARSE_OK)
	{
		print_parse_error(sh, status);
		goto end;
	}
	ret = EXIT_SUCCESS;

end:
	return ret;
}

static int run_file(struct sh *sh, const char *file)
{
	FILE *fp = NULL;
	char *line = NULL;
	size_t n = 0;
	ssize_t rd;
	int ret = EXIT_FAILURE;
	enum parse_status status = PARSE_OK;

	fp = fopen(file, "r");
	if (!fp)
	{
		fprintf(stderr, "%s: open(%s): %s\n", sh->progname, file,
		        strerror(errno));
		goto end;
	}
	while ((rd = getline(&line, &n, fp)) > 0)
	{
		status = parse(sh, line);
		if (status == PARSE_ERR)
			goto end;
	}
	if (ferror(fp))
	{
		fprintf(stderr, "%s: read: %s\n", sh->progname, strerror(errno));
		goto end;
	}
	if (status != PARSE_OK)
	{
		print_parse_error(sh, status);
		goto end;
	}
	ret = EXIT_SUCCESS;

end:
	if (fp)
		fclose(fp);
	free(line);
	return ret;
}

static void usage(const char *progname)
{
	printf("%s [-c command] [file]\n", progname);
	printf("-c command: run the given command\n");
}

int main(int argc, char **argv)
{
	struct sh sh;
	g_sh = &sh;
	memset(&sh, 0, sizeof(sh));
	sh.progname = argv[0];
	sh.last_bg_pid = -1;
	TAILQ_INIT(&sh.aliases);
	TAILQ_INIT(&sh.functions);
	if (parse_init(&sh))
		return EXIT_FAILURE;
	const char *level = getenv("SHLVL");
	if (level)
	{
		char tmp[64];
		snprintf(tmp, sizeof(tmp), "%d", atoi(level) + 1);
		setenv("SHLVL", tmp, 1);
	}
	else
	{
		setenv("SHLVL", "1", 1);
	}
	setenv("SHELL", argv[0], 1);
	int c;
	if (argc < 1)
		return EXIT_FAILURE;
	while ((c = getopt(argc, argv, "c:")) != -1)
	{
		switch (c)
		{
			case 'c':
				sh.argc = argc - optind;
				sh.argv = &argv[optind];
				run_str(&sh, optarg);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (optind != argc)
	{
		sh.argc = argc - optind;
		sh.argv = &argv[optind];
		return run_file(&sh, argv[optind]);
	}
	sh.argc = 0;
	sh.argv = NULL;
	if (!getenv("OLDPWD"))
		setenv("OLDPWD", getenv("PWD"), 1);
	return run_interactive(&sh);
}
