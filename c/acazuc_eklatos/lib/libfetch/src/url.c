#include <stdlib.h>
#include <string.h>
#include <fetch.h>
#include <ctype.h>

struct url *
fetchMakeURL(const char *scheme,
             const char *host,
             int port,
             const char *doc,
             const char *user,
             const char *pwd)
{
	if (port < 0 || port > UINT16_MAX)
		return NULL;
	struct url *url = calloc(1, sizeof(*url));
	if (!url)
		return NULL;
	if (doc)
	{
		url->doc = strdup(doc);
		if (!url->doc)
		{
			free(url);
			return NULL;
		}
	}
	if (scheme)
		strlcpy(url->scheme, scheme, sizeof(url->scheme));
	if (host)
		strlcpy(url->host, host, sizeof(url->host));
	if (user)
		strlcpy(url->user, user, sizeof(url->user));
	if (pwd)
		strlcpy(url->pwd, pwd, sizeof(url->pwd));
	url->port = port;
	return url;
}

struct url *
fetchParseURL(const char *str)
{
	if (!str)
		return NULL;
	struct url *url = calloc(1, sizeof(*url));
	if (!url)
		return NULL;
	char *ptr = strstr(str, "://");
	if (!ptr)
		goto err;
	snprintf(url->scheme, sizeof(url->scheme), "%.*s", (int)(ptr - str), str);
	ptr += 3;
	char *at = strpbrk(ptr, "@/"); /* you don't want to parse @ in the doc */
	if (at && *at == '@')
	{
		char *sc = memchr(ptr, ':', at - ptr);
		if (sc)
		{
			snprintf(url->user, sizeof(url->user), "%.*s", (int)(sc - ptr), ptr);
			sc++;
			snprintf(url->pwd, sizeof(url->pwd), "%.*s", (int)(at - sc), sc);
		}
		else
		{
			snprintf(url->user, sizeof(url->user), "%.*s", (int)(at - ptr), ptr);
		}
		ptr = at + 1;
	}
	char *sc = strpbrk(ptr, ":/");
	if (!sc)
	{
		strlcpy(url->host, ptr, sizeof(url->host));
		url->doc = strdup("/");
		if (!url->doc)
			goto err;
		return url;
	}
	snprintf(url->host, sizeof(url->host), "%.*s", (int)(sc - ptr), ptr);
	if (*sc == ':')
	{
		char *slash = strchrnul(sc, '/');
		sc++;
		while (sc < slash)
		{
			if (!isdigit(*sc))
				goto err;
			url->port = url->port * 10 + (*sc - '0');
			if (url->port > UINT16_MAX)
				goto err;
			sc++;
		}
		if (!*sc)
			return url;
		ptr = slash;
	}
	else
	{
		ptr = sc;
	}
	url->doc = strdup(ptr);
	if (!url->doc)
		goto err;
	return url;

err:
	fetchFreeURL(url);
	return NULL;
}

void
fetchFreeURL(struct url *url)
{
	if (!url)
		return;
	free(url->doc);
	free(url);
}
