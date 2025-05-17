#include <sys/stat.h>

#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#define ARG_NEXT() \
do \
{ \
	(*argv)++; \
	(*argc)--; \
} while (0)

#define CMP_INTS(argc, argv, cmp) \
do \
{ \
	intmax_t a, b; \
	enum status ret_ints = getints(argc, argv, &a, &b); \
	if (ret_ints) \
		return ret_ints; \
	return a cmp b ? STATUS_OK : STATUS_KO; \
} while (0)

enum status
{
	STATUS_OK,
	STATUS_KO,
	STATUS_ERR,
};

static const char *progname;

static enum status eval_op(int *argc, char ***argv);

static enum status
getint(int *argc, char ***argv, intmax_t *n)
{
	char *endptr;

	errno = 0;
	*n = strtoimax(**argv, &endptr, 10);
	if (errno || *endptr)
	{
		fprintf(stderr, "%s: invalid integer '%s'\n", progname, **argv);
		return STATUS_ERR;
	}
	ARG_NEXT();
	return STATUS_OK;
}

static enum status
getints(int *argc, char ***argv, intmax_t *a, intmax_t *b)
{
	enum status ret;

	ret = getint(argc, argv, a);
	if (ret)
		return ret;
	ARG_NEXT();
	ret = getint(argc, argv, b);
	if (ret)
		return ret;
	return STATUS_OK;
}

static enum status
stat_file(int *argc, char ***argv, struct stat *st)
{
	if (!*argc)
		return STATUS_ERR;
	if (stat(**argv, st) == -1)
	{
		ARG_NEXT();
		return STATUS_KO;
	}
	ARG_NEXT();
	return STATUS_OK;
}

static enum status
test_file_type(int *argc, char ***argv, mode_t type)
{
	struct stat st;
	enum status ret;

	ARG_NEXT();
	ret = stat_file(argc, argv, &st);
	if (ret)
		return ret;
	return ((st.st_mode & S_IFMT) == type) ? STATUS_OK : STATUS_KO;
}

static enum status
test_file_mode(int *argc, char ***argv, mode_t mode)
{
	struct stat st;
	enum status ret;

	ARG_NEXT();
	ret = stat_file(argc, argv, &st);
	if (ret)
		return ret;
	return (st.st_mode & mode) ? STATUS_OK : STATUS_KO;
}

static enum status
test_file_access(int *argc, char ***argv, int mode)
{
	ARG_NEXT();
	if (!*argc)
		return STATUS_ERR;
	if (access(**argv, mode) == -1)
	{
		ARG_NEXT();
		return STATUS_KO;
	}
	ARG_NEXT();
	return STATUS_OK;
}

static enum status
eval_logical(int *argc, char ***argv)
{
	enum status ret;

	ret = eval_op(argc, argv);
	if (ret == STATUS_ERR)
		return STATUS_ERR;
	if (!*argc)
		return ret;
	if (!strcmp(**argv, "-a"))
	{
		enum status ret2;

		ARG_NEXT();
		ret2 = eval_op(argc, argv);
		if (ret2 == STATUS_ERR)
			return STATUS_ERR;
		return ret || ret2;
	}
	if (!strcmp(**argv, "-o"))
	{
		enum status ret2;

		ARG_NEXT();
		ret2 = eval_op(argc, argv);
		if (ret2 == STATUS_ERR)
			return STATUS_ERR;
		return ret && ret2;
	}
	return ret;
}

static enum status
eval_parenthesis(int *argc, char ***argv)
{
	enum status ret;

	ARG_NEXT();
	ret = eval_logical(argc, argv);
	if (ret == STATUS_ERR)
		return STATUS_ERR;
	if (strcmp(**argv, ")"))
		return STATUS_ERR;
	ARG_NEXT();
	return ret;
}

static enum status
eval_op(int *argc, char ***argv)
{
	enum status ret;
	struct stat st;

	if (!strcmp(**argv, "("))
		return eval_parenthesis(argc, argv);
	if (!strcmp(**argv, "!"))
	{
		ARG_NEXT();
		ret = eval_op(argc, argv);
		if (ret == STATUS_ERR)
			return STATUS_ERR;
		return !ret;
	}
	if (!strcmp(**argv, "-b"))
		return test_file_type(argc, argv, S_IFBLK);
	if (!strcmp(**argv, "-c"))
		return test_file_type(argc, argv, S_IFCHR);
	if (!strcmp(**argv, "-d"))
		return test_file_type(argc, argv, S_IFDIR);
	if (!strcmp(**argv, "-e"))
	{
		ARG_NEXT();
		return stat_file(argc, argv, &st);
	}
	if (!strcmp(**argv, "-f"))
		return test_file_type(argc, argv, S_IFREG);
	if (!strcmp(**argv, "-g"))
		return test_file_mode(argc, argv, S_ISGID);
	if (!strcmp(**argv, "-k"))
		return test_file_mode(argc, argv, S_ISVTX);
	if (!strcmp(**argv, "-n"))
	{
		ARG_NEXT();
		if (!*argc)
			return STATUS_ERR;
		if (!***argv)
		{
			ARG_NEXT();
			return STATUS_KO;
		}
		ARG_NEXT();
		return STATUS_OK;
	}
	if (!strcmp(**argv, "-p"))
		return test_file_type(argc, argv, S_IFIFO);
	if (!strcmp(**argv, "-r"))
		return test_file_access(argc, argv, R_OK);
	if (!strcmp(**argv, "-s"))
	{
		ARG_NEXT();
		ret = stat_file(argc, argv, &st);
		if (ret)
			return ret;
		if (!st.st_size)
			return STATUS_KO;
		return STATUS_OK;
	}
	if (!strcmp(**argv, "-u"))
		return test_file_mode(argc, argv, S_ISUID);
	if (!strcmp(**argv, "-w"))
		return test_file_access(argc, argv, W_OK);
	if (!strcmp(**argv, "-x"))
		return test_file_access(argc, argv, X_OK);
	if (!strcmp(**argv, "-z"))
	{
		ARG_NEXT();
		if (!*argc)
			return STATUS_ERR;
		if (***argv)
		{
			ARG_NEXT();
			return STATUS_KO;
		}
		ARG_NEXT();
		return STATUS_OK;
	}
	if (!strcmp(**argv, "-L")
	 || !strcmp(**argv, "-h"))
	{
		ARG_NEXT();
		if (!*argc)
			return STATUS_ERR;
		if (lstat(**argv, &st) == -1)
		{
			ARG_NEXT();
			return STATUS_KO;
		}
		ARG_NEXT();
		if (!S_ISLNK(st.st_mode))
			return STATUS_KO;
		return STATUS_OK;
	}
	if (!strcmp(**argv, "-O"))
	{
		ret = stat_file(argc, argv, &st);
		ARG_NEXT();
		if (ret)
			return ret;
		if (st.st_uid != geteuid())
			return STATUS_KO;
		return STATUS_OK;
	}
	if (!strcmp(**argv, "-G"))
	{
		ARG_NEXT();
		ret = stat_file(argc, argv, &st);
		if (ret)
			return ret;
		if (st.st_gid != getegid())
			return STATUS_KO;
		return STATUS_OK;
	}
	if (!strcmp(**argv, "-s"))
		return test_file_type(argc, argv, S_IFSOCK);
	if (*argc == 1)
		return ***argv ? STATUS_OK : STATUS_KO;
	if (*argc < 3)
		return STATUS_ERR;
	if (!strcmp((*argv)[1], "=")
	 || !strcmp((*argv)[1], "=="))
	{
		if (strcmp((*argv)[0], (*argv)[2]))
			ret = STATUS_KO;
		else
			ret = STATUS_OK;
		ARG_NEXT();
		ARG_NEXT();
		ARG_NEXT();
		return ret;
	}
	if (!strcmp((*argv)[1], "!="))
	{
		if (strcmp((*argv)[0], (*argv)[2]))
			ret = STATUS_OK;
		else
			ret = STATUS_KO;
		ARG_NEXT();
		ARG_NEXT();
		ARG_NEXT();
		return ret;
	}
	if (!strcmp((*argv)[1], "-eq"))
		CMP_INTS(argc, argv, ==);
	if (!strcmp((*argv)[1], "-ne"))
		CMP_INTS(argc, argv, !=);
	if (!strcmp((*argv)[1], "-gt"))
		CMP_INTS(argc, argv, >);
	if (!strcmp((*argv)[1], "-ge"))
		CMP_INTS(argc, argv, >=);
	if (!strcmp((*argv)[1], "-lt"))
		CMP_INTS(argc, argv, <);
	if (!strcmp((*argv)[1], "-le"))
		CMP_INTS(argc, argv, <=);
	return STATUS_ERR;
}

int
main(int argc, char **argv)
{
	progname = argv[0];
	if (!strcmp(argv[0], "["))
	{
		if (strcmp(argv[argc - 1], "]"))
		{
			fprintf(stderr, "%s: missing ]\n", argv[0]);
			return EXIT_FAILURE;
		}
		if (argc == 2)
			return EXIT_FAILURE;
		argc--;
		argv[argc] = NULL;
	}
	else if (argc == 1)
	{
		return EXIT_FAILURE;
	}
	argc--;
	argv++;
	return eval_logical(&argc, &argv);
}
