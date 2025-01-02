#include <string.h>
#include <fetch.h>

FILE *fetchGetURL(const char *str, const char *flags)
{
	struct url *url = fetchParseURL(str);
	if (!url)
		return NULL;
	FILE *fp = fetchGet(url, flags);
	fetchFreeURL(url);
	return fp;
}

FILE *fetchPutURL(const char *str, const char *flags)
{
	struct url *url = fetchParseURL(str);
	if (!url)
		return NULL;
	FILE *fp = fetchPut(url, flags);
	fetchFreeURL(url);
	return fp;
}

int fetchStatURL(const char *str, struct url_stat *stat, const char *flags)
{
	struct url *url = fetchParseURL(str);
	if (!url)
		return -1;
	int ret = fetchStat(url, stat, flags);
	fetchFreeURL(url);
	return ret;
}

struct url_ent *fetchListURL(const char *str, const char *flags)
{
	struct url *url = fetchParseURL(str);
	if (!url)
		return NULL;
	struct url_ent *ret = fetchList(url, flags);
	fetchFreeURL(url);
	return ret;
}

FILE *fetchGet(struct url *url, const char *flags)
{
	if (!strcmp(url->scheme, SCHEME_FILE))
		return fetchGetFile(url, flags);
	if (!strcmp(url->scheme, SCHEME_HTTP))
		return fetchGetHTTP(url, flags);
	return NULL;
}

FILE *fetchPut(struct url *url, const char *flags)
{
	if (!strcmp(url->scheme, SCHEME_FILE))
		return fetchPutFile(url, flags);
	if (!strcmp(url->scheme, SCHEME_HTTP))
		return fetchPutHTTP(url, flags);
	return NULL;
}

int fetchStat(struct url *url, struct url_stat *stat, const char *flags)
{
	if (!strcmp(url->scheme, SCHEME_FILE))
		return fetchStatFile(url, stat, flags);
	if (!strcmp(url->scheme, SCHEME_HTTP))
		return fetchStatHTTP(url, stat, flags);
	return -1;
}

struct url_ent *fetchList(struct url *url, const char *flags)
{
	if (!strcmp(url->scheme, SCHEME_FILE))
		return fetchListFile(url, flags);
	if (!strcmp(url->scheme, SCHEME_HTTP))
		return fetchListHTTP(url, flags);
	return NULL;
}
