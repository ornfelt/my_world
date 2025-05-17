#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <pwd.h>

static void
usage(const char *progname)
{
	printf("%s [-s shell] [user]\n", progname);
	printf("-s shell: use the given shell\n");
}

int
main(int argc, char **argv)
{
	char *shell = NULL;
	uid_t uid;
	gid_t gid;
	int c;

	while ((c = getopt(argc, argv, "s")) != -1)
	{
		switch (c)
		{
			case 's':
				shell = optarg;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (geteuid() != 0)
	{
		/* XXX ask for password */
		fprintf(stderr, "%s: not root\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (argc - optind > 1)
	{
		fprintf(stderr, "%s: extra operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (!shell)
		shell = getenv("SHELL");
	if (optind != argc)
	{
		struct passwd *passwd = getpwnam(argv[optind]);
		if (!passwd)
		{
			fprintf(stderr, "%s: unknown user\n", argv[0]);
			return EXIT_FAILURE;
		}
		uid = passwd->pw_uid;
		gid = passwd->pw_gid;
		if (!shell)
			shell = passwd->pw_shell;
	}
	else
	{
		uid = 0;
		gid = 0;
	}
	if (!shell)
		shell = "/bin/sh";
	if (setgid(gid) == -1)
	{
		fprintf(stderr, "%s: setgid: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	if (setuid(uid) == -1)
	{
		fprintf(stderr, "%s: setuid: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	char * const shell_argv[] = {shell, NULL};
	execvp(shell, shell_argv);
	fprintf(stderr, "%s: execv: %s\n", argv[0], strerror(errno));
	return EXIT_FAILURE;
}
