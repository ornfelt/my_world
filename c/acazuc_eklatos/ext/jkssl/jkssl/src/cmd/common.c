#include "utils/utils.h"
#include "cmd/common.h"

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#endif

int cmd_handle_format(const char *progname, const char *arg,
                      enum cmd_format *format)
{
	if (!strcmp(arg, "PEM"))
	{
		*format = CMD_FORMAT_PEM;
		return 1;
	}
	if (!strcmp(arg, "DER"))
	{
		*format = CMD_FORMAT_DER;
		return 1;
	}
	fprintf(stderr, "%s: unsupported format: %s\n", progname, arg);
	return 0;
}

int cmd_handle_in(const char *progname, const char *arg, FILE **fp)
{
	if (*fp && *fp != stdin)
		fclose(*fp);
	*fp = fopen(arg, "r");
	if (!*fp)
	{
		fprintf(stderr, "%s: open(%s): %s\n", progname, arg,
		        strerror(errno));
		return 0;
	}
	return 1;
}

int cmd_handle_out(const char *progname, const char *arg, FILE **fp)
{
	if (*fp && *fp != stdout)
		fclose(*fp);
	*fp = fopen(arg, "w");
	if (!*fp)
	{
		fprintf(stderr, "%s: open(%s): %s\n", progname, arg,
		        strerror(errno));
		return 0;
	}
	return 1;
}

static int handle_pass_text(const char *progname, const char *arg, char **pass)
{
	*pass = strdup(arg);
	if (!*pass)
	{
		fprintf(stderr, "%s: allocation failed\n", progname);
		return 0;
	}
	return 1;
}

static int handle_pass_env(const char *progname, const char *arg, char **pass)
{
	const char *env = getenv(arg);
	if (!env)
	{
		fprintf(stderr, "%s: unknown environment var '%s'\n",
		        progname, arg);
		return 0;
	}
	*pass = strdup(env);
	if (!*pass)
	{
		fprintf(stderr, "%s: allocation failed\n", progname);
		return 0;
	}
	return 1;
}

static int handle_pass_fp(const char *progname, FILE *fp, char **pass)
{
	char buf[1024];
	if (!fgets(buf, sizeof(buf), fp))
	{
		fprintf(stderr, "%s: file read failed: %s\n", progname,
		        strerror(errno));
		return 0;
	}
	*pass = strdup(buf);
	if (!*pass)
	{
		fprintf(stderr, "%s: allocation failed\n", progname);
		return 0;
	}
	size_t passlen = strlen(*pass);
	if (passlen && (*pass)[passlen - 1] == '\n')
		(*pass)[passlen - 1] = '\0';
	return 1;
}

static int handle_pass_file(const char *progname, const char *arg, char **pass)
{
	FILE *fp = fopen(arg, "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: open(%s): %s\n", progname, arg,
		        strerror(errno));
		return 0;
	}
	int ret = handle_pass_fp(progname, fp, pass);
	fclose(fp);
	return ret;
}

#ifndef _WIN32
static int handle_pass_fd(const char *progname, const char *arg, char **pass)
{
	errno = 0;
	char *endptr;
	long fd = strtol(arg, &endptr, 10);
	if (errno || *endptr || fd < 0 || fd > INT_MAX)
	{
		fprintf(stderr, "%s: invalid file descriptor\n", progname);
		return 0;
	}
	FILE *fp = fdopen(fd, "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: failed to open fd %d\n", progname, (int)fd);
		return 0;
	}
	int ret = handle_pass_fp(progname, fp, pass);
	fclose(fp);
	return ret;
}
#endif

int cmd_handle_pass(const char *progname, const char *arg, char **pass)
{
	if (!strncmp(arg, "pass:", 5))
		return handle_pass_text(progname, arg + 5, pass);
	if (!strncmp(arg, "env:", 4))
		return handle_pass_env(progname, arg + 4, pass);
	if (!strncmp(arg, "file:", 5))
		return handle_pass_file(progname, arg + 5, pass);
#ifndef _WIN32
	if (!strncmp(arg, "fd:", 3))
		return handle_pass_fd(progname, arg + 3, pass);
#endif
	if (!strcmp(arg, "stdin"))
		return handle_pass_fp(progname, stdin, pass);
	fprintf(stderr, "%s: invalid password format\n", progname);
	return 0;
}

static char *get_password(const char *prompt)
{
#ifdef _WIN32
	static char buf[4096];
	size_t len = 0;
	DWORD current_mode;
	HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
	printf("%s", prompt);
	fflush(stdout);
	GetConsoleMode(handle, &current_mode);
	SetConsoleMode(handle, ENABLE_LINE_INPUT);
	int c;
	while ((c = getchar()) != EOF)
	{
		if (c == '\n')
			break;
		if (len + 1 >= sizeof(buf))
			return NULL;
		buf[len++] = c;
	}
	buf[len] = '\0';
	printf("\n");
	SetConsoleMode(handle, current_mode);
	return buf;
#else
	return getpass(prompt);
#endif
}

char *cmd_ask_password(void)
{
	return get_password("Enter cipher password: ");
}

char *cmd_ask_password_confirm(void)
{
	char *pass;
	char *tmp;

	tmp = get_password("Enter cipher password: ");
	if (!tmp)
		return NULL;
	pass = strdup(tmp);
	if (!pass)
		return NULL;
	tmp = get_password("Verifying - Enter cipher password: ");
	if (!tmp)
	{
		free(pass);
		return NULL;
	}
	if (strcmp(tmp, pass))
	{
		fprintf(stderr, "Password are not the same\n");
		free(pass);
		return NULL;
	}
	free(pass);
	return tmp;
}

int cmd_ask_pass(char *buf, int buf_size, int rw, void *userdata)
{
	(void)userdata;
	char *pass;
	if (rw)
		pass = cmd_ask_password_confirm();
	else
		pass = cmd_ask_password();
	if (!pass)
		return -1;
	size_t len = strlen(pass);
	if (len >= (size_t)buf_size)
		return -1;
	memcpy(buf, pass, len + 1);
	return len;
}
