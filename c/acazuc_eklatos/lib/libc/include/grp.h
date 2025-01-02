#ifndef GRP_H
#define GRP_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FILE FILE;

struct group
{
	char *gr_name;
	char *gr_passwd;
	gid_t gr_gid;
	char **gr_mem;
};

int setgroups(size_t size, const gid_t *list);
int initgroups(const char *user, gid_t group);

struct group *getgrnam(const char *name);
struct group *getgrgid(gid_t gid);
int getgrnam_r(const char *name, struct group *grp, char *buf, size_t buflen,
               struct group **result);
int getgrgid_r(gid_t gid, struct group *grp, char *buf, size_t buflen,
               struct group **result);

struct group *getgrent(void);
void setgrent(void);
void endgrent(void);
struct group *fgetgrent(FILE *fp);
int putgrent(const struct group *grp, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif
