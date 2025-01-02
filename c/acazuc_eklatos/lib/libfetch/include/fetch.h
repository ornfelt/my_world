#ifndef FETCH_H
#define FETCH_H

#include <sys/param.h>

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define URL_SCHEMELEN 16
#define URL_HOSTLEN   256
#define URL_USERLEN   256
#define URL_PWDLEN    256

#define SCHEME_HTTP "http"
#define SCHEME_FILE "file"

struct url
{
	char scheme[URL_SCHEMELEN];
	char host[URL_HOSTLEN];
	char user[URL_USERLEN];
	char pwd[URL_PWDLEN];
	int port;
	char *doc;
};

struct url_stat
{
	off_t size;
	size_t atime;
	size_t mtime;
};

struct url_ent
{
	char name[MAXPATHLEN];
	struct url_stat stat;
};

FILE *fetchGetFile(struct url *url, const char *flags);
FILE *fetchPutFile(struct url *url, const char *flags);
int fetchStatFile(struct url *url, struct url_stat *stat, const char *flags);
struct url_ent *fetchListFile(struct url *url, const char *flags);

FILE *fetchGetHTTP(struct url *url, const char *flags);
FILE *fetchPutHTTP(struct url *url, const char *flags);
int fetchStatHTTP(struct url *url, struct url_stat *stat, const char *flags);
struct url_ent *fetchListHTTP(struct url *url, const char *flags);

FILE *fetchGetURL(const char *url, const char *flags);
FILE *fetchPutURL(const char *url, const char *flags);
int fetchStatURL(const char *url, struct url_stat *stat, const char *flags);
struct url_ent *fetchListURL(const char *url, const char *flags);

FILE *fetchGet(struct url *url, const char *flags);
FILE *fetchPut(struct url *url, const char *flags);
int fetchStat(struct url *url, struct url_stat *stat, const char *flags);
struct url_ent *fetchList(struct url *url, const char *flags);

struct url *fetchMakeURL(const char *scheme, const char *host,
                         int port, const char *doc, const char *user,
                         const char *pwd);
struct url *fetchParseURL(const char *url);
void fetchFreeURL(struct url *url);

#ifdef __cplusplus
}
#endif

#endif
