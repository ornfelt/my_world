#include <sys/wait.h>
#include <sys/stat.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

static int
open_std_fd(void)
{
	int fdin;
	int fdout;
	int fderr;

	close(0);
	close(1);
	close(2);
	fdin = open("/dev/tty0", O_RDONLY);
	if (fdin == -1)
		fdin = open("/dev/ttyS0", O_RDONLY);
	fdout = open("/dev/tty0", O_WRONLY);
	if (fdout == -1)
		fdout = open("/dev/ttyS0", O_WRONLY);
	fderr = open("/dev/tty0", O_WRONLY);
	if (fderr == -1)
		fderr = open("/dev/ttyS0", O_WRONLY);
	if (fdin != 0 || fdout != 1 || fderr != 2)
		return 1;
	return 0;
}

static pid_t
run_sh(void)
{
	int pid;
	char * const argv[] = {"/bin/sh", NULL};

	pid = fork();
	if (pid == -1)
	{
		perror("init: fork");
		return -1;
	}
	if (pid != 0)
		return pid;
	if (open_std_fd())
		return EXIT_FAILURE;
	execv("/bin/sh", argv);
	perror("init: sh");
	exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
	int wstatus;
	pid_t sh_pid;
	pid_t wpid;

	(void)argc;
	(void)argv;
	umask(02);
	if (open_std_fd())
		return EXIT_FAILURE;
	setenv("DISPLAY", ":5", 1); /* XXX */
	setenv("PATH", "/bin", 1);
	setenv("LD_LIBRARY_PATH", "/lib", 1);
	setenv("PWD", "/", 1);
	setenv("HOME", "/root", 1);
	setenv("PS1", "\033[1;32m\\u@\\h\033[0m:\033[1;34m\\w\033[0m\\$ ", 1);
	setenv("PS2", "> ", 1);
	setenv("IFS", " \t\n", 1);
	system("/etc/rc");
	sh_pid = run_sh();
	while ((wpid = waitpid(-1, &wstatus, 0)))
	{
		if (wpid == -1)
		{
			if (errno != EINTR)
				fprintf(stderr, "init: wait: %s\n", strerror(errno));
			continue;
		}
		if (wpid == sh_pid)
		{
			if (WIFEXITED(wstatus) || WIFSIGNALED(wstatus))
				sh_pid = run_sh();
		}
	}
	return EXIT_FAILURE;
}
