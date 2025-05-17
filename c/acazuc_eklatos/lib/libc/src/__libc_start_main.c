#include <sys/auxv.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern char **environ;
extern const char *__libc_progname;
extern size_t *__libc_auxv;
extern uintptr_t __stack_chk_guard;

int
__libc_start_main(int (*main)(int, char**, char**),
                  int argc,
                  char **argv,
                  char **envp,
                  size_t *auxv)
{
	size_t envc = 0;
	int ret;

	__libc_progname = strchr(argv[0], '/');
	if (__libc_progname)
		__libc_progname++;
	else
		__libc_progname = argv[0];
	__libc_auxv = auxv;
	__stack_chk_guard = getauxval(AT_RANDOM);
	while (envp[envc])
		envc++;
	environ = malloc(sizeof(*environ) * (envc + 1));
	if (!environ)
	{
		fprintf(stderr, "failed to allocate environ\n");
		return EXIT_FAILURE;
	}
	for (size_t i = 0; i < envc; ++i)
	{
		environ[i] = strdup(envp[i]);
		if (!environ[i])
		{
			fprintf(stderr, "failed to allocate environ line\n");
			return EXIT_FAILURE;
		}
	}
	environ[envc] = NULL;
	ret = main(argc, argv, environ);
	fcloseall();
	return ret;
}
