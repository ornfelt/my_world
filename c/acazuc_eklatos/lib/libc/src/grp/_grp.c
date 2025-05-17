#include "_grp.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <grp.h>

struct group grp_ent;
char grp_buf[1024];
FILE *grent_fp;

int
parse_grline(struct group *group,
             char *buf,
             size_t buflen,
             const char *line)
{
	const char *name = line;
	size_t name_len;
	const char *pass;
	size_t pass_len;
	const char *gid;
	size_t gid_len;
	const char *members;
	char *endptr;

	pass = strchr(line, ':');
	if (!pass)
		return EINVAL;
	name_len = pass - name;
	pass++;
	gid = strchr(pass, ':');
	if (!gid)
		return EINVAL;
	pass_len = gid - pass;
	gid++;
	members = strchr(gid, ':');
	if (!members)
		return EINVAL;
	gid_len = members - gid;
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

int search_grnam(struct group *grp,
                 char *buf,
                 size_t buflen,
                 struct group **result,
                 int (*cmp_fn)(struct group *grp, const void *ptr),
                 const void *cmp_ptr)
{
	char *line = NULL;
	size_t line_size = 0;
	FILE *fp;
	ssize_t ret;

	fp = fopen("/etc/group", "rb");
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
		ret = parse_grline(grp, buf, buflen, line);
		if (ret)
			goto err;
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
