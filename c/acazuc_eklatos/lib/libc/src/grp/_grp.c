#include "_grp.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <grp.h>

struct group grp_ent;
char grp_buf[1024];
FILE *grent_fp;

int parse_grline(struct group *group, char *buf, size_t buflen,
                 const char *line)
{
	const char *name = line;
	const char *pass = strchr(line, ':');
	if (!pass)
		return EINVAL;
	size_t name_len = pass - name;
	pass++;
	const char *gid = strchr(pass, ':');
	if (!gid)
		return EINVAL;
	size_t pass_len = gid - pass;
	gid++;
	const char *members = strchr(gid, ':');
	if (!members)
		return EINVAL;
	size_t gid_len = members - gid;
	members++;
	if (strchr(members, ':'))
		return EINVAL;
	if (name_len + 1
	  + pass_len + 1 >= buflen) /* + members len */
		return EINVAL;
	if (gid_len >= 6)
		return EINVAL;
	for (size_t i = 0; i < gid_len; ++i)
	{
		if (!isdigit(gid[i]))
			return EINVAL;
	}
	char *endptr;
	group->gr_gid = strtol(gid, &endptr, 10);
	if (endptr != &gid[gid_len])
		return EINVAL;
	group->gr_name = buf;
	memcpy(buf, name, name_len);
	buf[name_len] = '\0';
	buf += name_len + 1;
	group->gr_passwd = buf;
	memcpy(buf, pass, pass_len);
	buf[pass_len] = '\0';
	buf += pass_len + 1;
	group->gr_mem = NULL; /* XXX */
	return 0;
}

int search_grnam(struct group *grp, char *buf, size_t buflen,
                 struct group **result,
                 int (*cmp_fn)(struct group *grp, const void *ptr),
                 const void *cmp_ptr)
{
	FILE *fp = fopen("/etc/group", "rb");
	if (!fp)
	{
		*result = NULL;
		return errno;
	}
	char *line = NULL;
	size_t line_size = 0;
	int ret;
	while (1)
	{
		ssize_t res = getline(&line, &line_size, fp);
		if (res == -1)
		{
			ret = ENOENT;
			goto err;
		}
		res = parse_grline(grp, buf, buflen, line);
		if (res)
		{
			ret = res;
			goto err;
		}
		if (!cmp_fn(grp, cmp_ptr))
		{
			free(line);
			fclose(fp);
			*result = grp;
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
