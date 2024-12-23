#include "utils.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

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

char *ask_password(void)
{
	return get_password("Enter cipher password: ");
}

char *ask_password_confirm(void)
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
