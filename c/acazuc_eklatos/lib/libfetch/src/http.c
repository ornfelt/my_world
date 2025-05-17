#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <fetch.h>
#include <errno.h>
#include <zlib.h>

enum http_encoding
{
	ENCODING_RAW,
	ENCODING_GZIP,
	ENCODING_DEFLATE,
};

struct http_state
{
	FILE *fp;
	int fd;
	char **headers;
	size_t headers_count;
	int headers_ended;
	char buf[4096]; /* XXX ringbuf */
	size_t buf_size;
	enum http_encoding encoding;
	z_stream z_stream;
	int eof;
};

static int
setup_socket(struct url *url, int *fd)
{
	struct addrinfo *addrs;
	struct sockaddr_in sin;
	int ret;

	*fd = -1;
	ret = getaddrinfo(url->host, NULL, NULL, &addrs);
	if (ret)
		return 1;
	if (!addrs)
		return 1;
	ret = 1;
	if (addrs->ai_family != AF_INET)
		goto end;
	*fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*fd == -1)
		goto end;
	sin.sin_family = AF_INET;
	sin.sin_addr = ((struct sockaddr_in*)addrs->ai_addr)->sin_addr;
	sin.sin_port = htons(url->port ? url->port : 80);
	if (connect(*fd, (struct sockaddr*)&sin, sizeof(sin)) == -1)
		goto end;
	ret = 0;

end:
	freeaddrinfo(addrs);
	if (ret)
	{
		close(*fd);
		*fd = -1;
	}
	return ret;
}

static int
handle_header(struct http_state *state, const char *header)
{
	if (!strncmp(header, "Content-Encoding: ", 18))
	{
		if (state->encoding)
			return -1;
		if (!strcmp(&header[18], "gzip"))
		{
			state->encoding = ENCODING_GZIP;
			if (inflateInit(&state->z_stream))
				return -1;
			return 0;
		}
		if (!strcmp(&header[18], "deflate"))
		{
			state->encoding = ENCODING_DEFLATE;
			if (inflateInit(&state->z_stream))
				return -1;
			return 0;
		}
		return -1;
	}
	return 0;
}

static int
parse_headers(struct http_state *state)
{
	int rd = read(state->fd, &state->buf[state->buf_size],
	              sizeof(state->buf) - state->buf_size);
	if (rd == -1 || !rd)
		return rd;
	state->buf_size += rd;
	while (1)
	{
		char *crnl = memmem(state->buf, state->buf_size, "\r\n", 2);
		if (!crnl)
		{
			/* XXX handle more then 4096 bytes headers lines */
			if (state->buf_size == sizeof(state->buf))
			{
				errno = EINVAL;
				return -1;
			}
			return 1;
		}
		if (crnl == state->buf)
		{
			memcpy(state->buf, &state->buf[2], state->buf_size - 2);
			state->buf_size -= 2;
			state->headers_ended = 1;
			return 1;
		}
		char **headers = realloc(state->headers,
		                         sizeof(*headers) * (state->headers_count + 1));
		if (!headers)
			return -1;
		state->headers = headers;
		size_t header_len = crnl - state->buf;
		headers[state->headers_count] = malloc(header_len + 1);
		if (!headers[state->headers_count])
			return -1;
		memcpy(headers[state->headers_count], state->buf, header_len);
		headers[state->headers_count][header_len] = '\0';
		state->headers_count++;
		memcpy(state->buf, &state->buf[header_len + 2],
		       state->buf_size - (header_len + 2));
		state->buf_size -= header_len + 2;
		if (handle_header(state, state->headers[state->headers_count - 1]))
			return -1;
	}
}

static int
zlib_read(struct http_state *state, char *buf, int size)
{
	int ret;
again:
	if (!state->eof && state->buf_size != sizeof(state->buf))
	{
		ret = read(state->fd, &state->buf[state->buf_size],
		           sizeof(state->buf) - state->buf_size);
		if (ret == -1)
			return -1;
		if (ret)
			state->buf_size += ret;
		else
			state->eof = 1;
	}
	state->z_stream.avail_in = state->buf_size;
	state->z_stream.next_in = (uint8_t*)state->buf;
	state->z_stream.avail_out = size;
	state->z_stream.next_out = (uint8_t*)buf;
	ret = inflate(&state->z_stream, Z_NO_FLUSH);
	switch (ret)
	{
		case Z_OK:
		case Z_STREAM_END:
			break;
		default:
			errno = EINVAL;
			return -1;
	}
	int processed_in = state->buf_size - state->z_stream.avail_in;
	if (processed_in)
	{
		memcpy(state->buf, &state->buf[processed_in],
		       state->buf_size - processed_in);
		state->buf_size -= processed_in;
	}
	if (state->z_stream.avail_out == (size_t)size)
	{
		if (state->eof)
			return 0;
		if (!processed_in)
		{
			errno = EINVAL;
			return -1; /* absolutely nothing was done, this shouldn't happend */
		}
		goto again;
	}
	return size - state->z_stream.avail_out;
}

static int
http_read(void *cookie, char *buf, int size)
{
	struct http_state *state = cookie;
	while (!state->headers_ended)
	{
		int ret = parse_headers(state);
		if (ret <= 0)
			return ret;
	}
	switch (state->encoding)
	{
		case ENCODING_RAW:
			if (state->buf_size)
			{
				int ret = state->buf_size;
				memcpy(buf, state->buf, state->buf_size);
				state->buf_size = 0;
				return ret;
			}
			return read(state->fd, buf, size);
		case ENCODING_GZIP:
		case ENCODING_DEFLATE:
		{
			int total = 0;
			while (total < size)
			{
				int ret = zlib_read(state, &buf[total], size - total);
				if (ret == -1)
					return -1;
				if (!ret)
					return total;
				total += ret;
			}
			return total;
		}
		default:
			return -1;
	}
}

static int
http_close(void *cookie)
{
	struct http_state *state = cookie;
	int fd = state->fd;
	switch (state->encoding)
	{
		case ENCODING_RAW:
			break;
		case ENCODING_GZIP:
		case ENCODING_DEFLATE:
			inflateEnd(&state->z_stream);
			break;
	}
	for (size_t i = 0; i < state->headers_count; ++i)
		free(state->headers[i]);
	free(state->headers);
	free(state);
	return close(fd);
}

FILE *
fetchGetHTTP(struct url *url, const char *flags)
{
	struct http_state *state;
	char buf[4096];
	size_t buf_len;

	(void)flags;
	state = calloc(1, sizeof(*state));
	if (!state)
		return NULL;
	state->fd = -1;
	if (setup_socket(url, &state->fd))
		goto err;
	state->fp = funopen(state, http_read, NULL, NULL, http_close);
	if (!state->fp)
		goto err;
	snprintf(buf, sizeof(buf), "GET %s HTTP/1.1\r\n"
	                           "Host: %s\r\n"
	                           "User-Agent: Mozilla/5.0 (Eklatos) fetch/1.0\r\n"
	                           "Connection: close\r\n"
	                           "Accept-Encoding: gzip, deflate\r\n"
	                           "\r\n",
	                           url->doc,
	                           url->host);
	buf_len = strlen(buf);
	if (write(state->fd, buf, buf_len) != (ssize_t)buf_len
	 || ferror(state->fp)
	 || fflush(state->fp))
		goto err;
	return state->fp;

err:
	if (state->fp)
		fclose(state->fp);
	else if (state->fd != -1)
		close(state->fd);
	free(state);
	return NULL;
}

FILE *
fetchPutHTTP(struct url *url, const char *flags)
{
	(void)url;
	(void)flags;
	/* XXX */
	return NULL;
}

int
fetchStatHTTP(struct url *url, struct url_stat *stat, const char *flags)
{
	(void)url;
	(void)stat;
	(void)flags;
	/* XXX */
	return -1;
}

struct url_ent *
fetchListHTTP(struct url *url, const char *flags)
{
	(void)url;
	(void)flags;
	/* XXX */
	return NULL;
}
