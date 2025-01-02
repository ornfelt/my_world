#ifndef PWD_H
#define PWD_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FILE FILE;

struct passwd
{
	char *pw_name;
	char *pw_passwd;
	uid_t pw_uid;
	gid_t pw_gid;
	char *pw_gecos;
	char *pw_dir;
	char *pw_shell;
};

struct passwd *getpwnam(const char *name);
struct passwd *getpwuid(uid_t uid);
int getpwnam_r(const char *name, struct passwd *pwd, char *buf, size_t buflen,
               struct passwd **result);
int getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen,
               struct passwd **result);

struct passwd *getpwent(void);
void setpwent(void);
void endpwent(void);
struct passwd *fgetpwent(FILE *fp);
int putpwent(const struct passwd *p, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif
