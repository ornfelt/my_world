#include "_pwd.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <pwd.h>

struct passwd pwd_ent;
char pwd_buf[1024];
FILE *pwent_fp;

int
parse_pwline(struct passwd *passwd,
             char *buf,
             size_t buflen,
             const char *line)
{
	const char *name = line;
	const char *pass = strchr(line, ':');
	if (!pass)
		return EINVAL;
	size_t name_len = pass - name;
	pass++;
	const char *uid = strchr(pass, ':');
	if (!uid)
		return EINVAL;
	size_t pass_len = uid - pass;
	uid++;
	const char *gid = strchr(uid, ':');
	if (!gid)
		return EINVAL;
	size_t uid_len = gid - uid;
	gid++;
	const char *comment = strchr(gid, ':');
	if (!comment)
		return EINVAL;
	size_t gid_len = comment - gid;
	comment++;
	const char *home = strchr(comment, ':');
	if (!home)
		return EINVAL;
	size_t comment_len = home - comment;
	home++;
	const char *shell = strchr(home, ':');
	if (!shell)
		return EINVAL;
	size_t home_len = shell - home;
	shell++;
	if (strchr(shell, ':'))
		return EINVAL;
	size_t shell_len = strlen(shell);
	if (shell_len && shell[shell_len - 1] == '\n')
		shell_len--;
	if (name_len + 1
	  + pass_len + 1
	  + comment_len + 1
	  + home_len + 1
	  + shell_len + 1 >= buflen)
		return EINVAL;
	if (uid_len >= 6)
		return EINVAL;
	for (size_t i = 0; i < uid_len; ++i)
	{
		if (!isdigit(uid[i]))
			return EINVAL;
	}
	char *endptr;
	passwd->pw_uid = strtol(uid, &endptr, 10);
	if (endptr != &uid[uid_len])
		return EINVAL;
	if (gid_len >= 6)
		return EINVAL;
	for (size_t i = 0; i < gid_len; ++i)
	{
		if (!isdigit(gid[i]))
			return EINVAL;
	}
	passwd->pw_gid = strtol(gid, &endptr, 10);
	if (endptr != &gid[gid_len])
		return EINVAL;
	passwd->pw_name = buf;
	memcpy(buf, name, name_len);
	buf[name_len] = '\0';
	buf += name_len + 1;
	passwd->pw_passwd = buf;
	memcpy(buf, pass, pass_len);
	buf[pass_len] = '\0';
	buf += pass_len + 1;
	passwd->pw_gecos = buf;
	memcpy(buf, comment, comment_len);
	buf[comment_len] = '\0';
	buf += comment_len + 1;
	passwd->pw_dir = buf;
	memcpy(buf, home, home_len);
	buf[home_len] = '\0';
	buf += home_len + 1;
	passwd->pw_shell = buf;
	memcpy(buf, shell, shell_len);
	buf[shell_len] = '\0';
	return 0;
}

int
search_pwnam(struct passwd *pwd,
             char *buf,
             size_t buflen,
             struct passwd **result,
             int (*cmp_fn)(struct passwd *pwd, const void *ptr),
             const void *cmp_ptr)
{
	FILE *fp;
	char *line = NULL;
	size_t line_size = 0;
	ssize_t ret;

	fp = fopen("/etc/passwd", "rb");
	if (!fp)
	{
		*result = NULL;
		return errno;
	}
	while (1)
	{
		ret = getline(&line, &line_size, fp);
		if (ret == -1)
		{
			ret = ENOENT;
			goto err;
		}
		ret = parse_pwline(pwd, buf, buflen, line);
		if (ret)
			goto err;
		if (!cmp_fn(pwd, cmp_ptr))
		{
			free(line);
			fclose(fp);
			*result = pwd;
			return 0;
		}
	}
	ret = ENOENT;

err:
	free(line);
	fclose(fp);
	*result = NULL;
	return ret;
}
