#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>

int main(int argc, char **argv)
{
	uid_t euid;

	(void)argc;
	(void)argv;
	euid = geteuid();
	struct passwd *pwd = getpwuid(euid);
	if (pwd)
		printf("%s\n", pwd->pw_name);
	else
		printf("%lu\n", (unsigned long)euid);
	return EXIT_SUCCESS;
}
