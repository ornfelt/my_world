#include "sh.h"

#include <sys/param.h>
#include <sys/stat.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

static int builtin_cd(struct sh *sh, int argc, char **argv)
{
	char path[MAXPATHLEN];
	char rpath[PATH_MAX];

	(void)sh;
	if (argc == 0)
	{
		const char *home = getenv("HOME");
		if (!home)
			return EXIT_FAILURE;
		if (strlcpy(path, home, sizeof(path)) >= sizeof(path))
			return EXIT_FAILURE;
	}
	else if (argc > 1)
	{
		fprintf(stderr, "%s: extra operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	else
	{
		if (!strcmp(argv[1], "-"))
		{
			const char *oldpwd = getenv("OLDPWD");
			if (!oldpwd)
				return EXIT_FAILURE;
			if (strlcpy(path, oldpwd, sizeof(path)) >= sizeof(path))
				return EXIT_FAILURE;
		}
		else
		{
			const char *pwd = getenv("PWD");
			snprintf(path, sizeof(path), "%s/%s", pwd, argv[1]);
		}
	}
	if (!realpath(path, rpath)) /* XXX shouldn't resolve symlinks */
	{
		fprintf(stderr, "%s: realpath: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	if (chdir(rpath) == -1)
	{
		fprintf(stderr, "%s: chdir: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	setenv("OLDPWD", getenv("PWD"), 1);
	setenv("PWD", rpath, 1);
	return EXIT_SUCCESS;
}

static int builtin_exit(struct sh *sh, int argc, char **argv)
{
	(void)sh;
	if (argc < 2)
		exit(EXIT_SUCCESS); /* XXX code from last command executed */
	exit(atoi(argv[1]));
	return EXIT_FAILURE;
}

static int builtin_export(struct sh *sh, int argc, char **argv)
{
	(void)sh;
	(void)argc;
	(void)argv;
	/* XXX */
	return EXIT_SUCCESS;
}

static int parse_mode(const char *progname, const char *str, mode_t *mode)
{
	*mode = 0;
	for (size_t i = 0; str[i]; ++i)
	{
		if (str[i] < '0' || str[i] > '7')
		{
			fprintf(stderr, "%s: invalid mode\n", progname);
			return 1;
		}
		*mode = *mode * 8 + str[i] - '0';
		if (*mode > 0777)
		{
			fprintf(stderr, "%s: mode out of bounds\n", progname);
			return 1;
		}
	}
	return 0;
}

static int builtin_umask(struct sh *sh, int argc, char **argv)
{
	(void)sh;
	if (argc == 1)
	{
		mode_t mask = umask(0);
		umask(mask);
		printf("%04o\n", (unsigned)mask);
		return EXIT_SUCCESS;
	}
	if (argc > 2)
	{
		fprintf(stderr, "%s: extra operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	mode_t mode;
	if (parse_mode(argv[0], argv[1], &mode))
		return EXIT_FAILURE;
	umask(mode);
	return EXIT_SUCCESS;
}

static int builtin_unset(struct sh *sh, int argc, char **argv)
{
	(void)sh;
	for (int i = 1; i < argc; ++i)
		unsetenv(argv[i]);
	return EXIT_SUCCESS;
}

static int builtin_alias(struct sh *sh, int argc, char **argv)
{
	(void)sh;
	if (argc == 1)
	{
		struct alias *alias;
		TAILQ_FOREACH(alias, &sh->aliases, chain)
		{
			printf("alias %s='%s'\n", alias->name, alias->cmd);
		}
		return EXIT_SUCCESS;
	}
	for (int i = 1; i < argc; ++i)
	{
		char *sep = strchr(argv[i], '=');
		if (sep)
		{
			struct alias *alias = malloc(sizeof(*alias));
			char *name = strndup(argv[i], sep - argv[i]);
			char *cmd = strdup(sep + 1);
			if (!alias || !name || !cmd)
			{
				fprintf(stderr, "%s: malloc: %s\n", argv[0],
				        strerror(errno));
				free(alias);
				free(name);
				free(cmd);
				continue;
			}
			alias->name = name;
			alias->cmd = cmd;
			TAILQ_INSERT_TAIL(&sh->aliases, alias, chain);
		}
		else
		{
			struct alias *alias;
			TAILQ_FOREACH(alias, &sh->aliases, chain)
			{
				if (strcmp(alias->name, argv[i]))
					continue;
				printf("alias %s='%s'\n", alias->name, alias->cmd);
				break;
			}
			if (!alias)
				fprintf(stderr, "%s: %s: not found\n", argv[0], argv[i]);
		}
	}
	return EXIT_SUCCESS;
}

const struct builtin g_builtins[] =
{
	{"cd",     builtin_cd},
	{"exit",   builtin_exit},
	{"export", builtin_export},
	{"umask",  builtin_umask},
	{"unset",  builtin_unset},
	{"alias",  builtin_alias},
	{NULL,     NULL},
};
