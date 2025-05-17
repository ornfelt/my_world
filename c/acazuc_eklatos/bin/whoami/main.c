#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>

int
main(int argc, char **argv)
{
	struct passwd *pwd;
	uid_t euid;

	(void)argc;
	(void)argv;
	euid = geteuid();
	pwd = getpwuid(euid);
	if (pwd)
		printf("%s\n", pwd->pw_name);
	else
		printf("%lu\n", (unsigned long)euid);
	return EXIT_SUCCESS;
}
