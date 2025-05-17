#include <stdlib.h>
#include <stdio.h>

struct _funopen_cookie
{
	void *cookie;
	int (*readfn)(void *, char *, int);
	int (*writefn)(void *, const char *, int);
	off_t (*seekfn)(void *, off_t, int);
	int (*closefn)(void *);
};

static ssize_t
_cookie_read(void *cookie, char *buf, size_t len)
{
	struct _funopen_cookie *funopen_cookie = cookie;
	return funopen_cookie->readfn(funopen_cookie->cookie, buf, len);
}

static ssize_t
_cookie_write(void *cookie, const char *buf, size_t len)
{
	struct _funopen_cookie *funopen_cookie = cookie;
	return funopen_cookie->writefn(funopen_cookie->cookie, buf, len);
}

static int
_cookie_seek(void *cookie, off_t off, int whence)
{
	struct _funopen_cookie *funopen_cookie = cookie;
	return funopen_cookie->seekfn(funopen_cookie->cookie, off, whence);
}

static int
_cookie_close(void *cookie)
{
	struct _funopen_cookie *funopen_cookie = cookie;
	int ret = 0;

	if (funopen_cookie->closefn)
		ret = funopen_cookie->closefn(funopen_cookie->cookie);
	free(funopen_cookie);
	return ret;
}

FILE *
funopen(const void *cookie,
        int (*readfn)(void *, char *, int),
        int (*writefn)(void *, const char *, int),
        off_t (*seekfn)(void *, off_t, int),
        int (*closefn)(void *))
{
	cookie_io_functions_t funcs;
	struct _funopen_cookie *funopen_cookie;
	char *mode;
	FILE *fp;

	funopen_cookie = malloc(sizeof(*funopen_cookie));
	if (!funopen_cookie)
		return NULL;
	funopen_cookie->cookie = (void*)cookie;
	funopen_cookie->readfn = readfn;
	funopen_cookie->writefn = writefn;
	funopen_cookie->seekfn = seekfn;
	funopen_cookie->closefn = closefn;
	if (readfn)
		funcs.read = _cookie_read;
	if (writefn)
		funcs.write = _cookie_write;
	if (seekfn)
		funcs.seek = _cookie_seek;
	funcs.close = _cookie_close;
	if (readfn)
	{
		if (writefn)
			mode = "w+b";
		else
			mode = "wb";
	}
	else
	{
		mode = "rb";
	}
	fp = fopencookie(funopen_cookie, mode, &funcs);
	if (!fp)
	{
		free(funopen_cookie);
		return NULL;
	}
	return fp;
}
