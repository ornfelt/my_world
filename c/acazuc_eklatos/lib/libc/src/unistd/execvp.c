#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

int execvp(const char *file, char * const argv[])
{
	if (strchr(file, '/'))
		return execv(file, argv);
	const char *env_path = getenv("PATH");
	if (!env_path)
		env_path = "/bin:/usr/bin";
	int noexec = 0;
	int access = 0;
	while (*env_path)
	{
		char path[MAXPATHLEN];
		/* XXX handle quotes, escapes */
		const char *next_env_path = strchrnul(env_path, ':');
		snprintf(path, sizeof(path), "%.*s/%s",
		         (int)(next_env_path - env_path), env_path, file);
		int res = execv(path, argv);
		if (res != -1)
		{
			errno = EINVAL;
			return -1;
		}
		if (errno == EACCES)
			access = 1;
		else if (errno == ENOEXEC)
			noexec = 1;
		else if (errno != ENOENT)
			return -1;
		env_path = next_env_path;
		if (*env_path == ':')
			env_path++;
	}
	if (noexec)
		errno = ENOEXEC;
	else if (access)
		errno = EACCES;
	else
		errno = ENOENT;
	return -1;
}
