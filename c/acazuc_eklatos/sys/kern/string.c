#include <errno.h>
#include <std.h>

#define fillsize(v) ((v) * ((size_t)-1 / 255))
#define haszero(x) (((x) - fillsize(1)) & ~(x) & fillsize(0x80))

void *memset(void *d, int c, size_t n)
{
	size_t i = 0;
	if (n >= sizeof(size_t))
	{
		size_t cz = fillsize((uint8_t)c);
		size_t end = n - n % sizeof(size_t);
		do
		{
			*(size_t*)&((uint8_t*)d)[i] = cz;
			i += sizeof(size_t);
		}
		while (i < end);
	}
	while (i < n)
	{
		((uint8_t*)d)[i] = c;
		i++;
	}
	return d;
}

void *memcpy(void *d, const void *s, size_t n)
{
	size_t i = 0;
	if (n >= sizeof(size_t))
	{
		size_t end = n - n % sizeof(size_t);
		do
		{
			*(size_t*)&((uint8_t*)d)[i] = *(size_t*)&((uint8_t*)s)[i];
			i += sizeof(size_t);
		} while (i < end);
	}
	while (i < n)
	{
		((uint8_t*)d)[i] = ((const uint8_t*)s)[i];
		i++;
	}
	return d;
}

void *memccpy(void *d, const void *s, int c, size_t n)
{
	size_t i = 0;
	if (n >= sizeof(size_t))
	{
		size_t cz = fillsize((uint8_t)c);
		size_t end = n - n % sizeof(size_t);
		do
		{
			size_t v = *(size_t*)&((uint8_t*)s)[i];
			size_t x = v ^ cz;
			if (haszero(x))
				break;
			*(size_t*)&((uint8_t*)d)[i] = v;
			i += sizeof(size_t);
		} while (i < end);
	}
	while (i < n)
	{
		if (((uint8_t*)s)[i] == c)
		{
			((uint8_t*)d)[i] = c;
			return (uint8_t*)d + i + 1;
		}
		((uint8_t*)d)[i] = ((uint8_t*)s)[i];
		i++;
	}
	return NULL;
}

void *memmove(void *d, const void *s, size_t n)
{
	if (!n)
		return d;
	if (d == s)
		return d;
	if (d < s)
		return memcpy(d, s, n);
	if ((size_t)((uint8_t*)d - (uint8_t*)s) >= sizeof(size_t))
	{
		while (n >= sizeof(size_t))
		{
			n -= sizeof(size_t);
			*(size_t*)&((uint8_t*)d)[n] = *(size_t*)&((uint8_t*)s)[n];
		}
	}
	while (n)
	{
		n--;
		((uint8_t*)d)[n] = ((uint8_t*)s)[n];
	}
	return d;
}

void *memchr(const void *s, int c, size_t n)
{
	size_t i = 0;
	if (n >= sizeof(size_t))
	{
		size_t cz = fillsize((uint8_t)c);
		size_t end = n - n % sizeof(size_t);
		do
		{
			size_t v = *(size_t*)&((uint8_t*)s)[i] ^ cz;
			if (haszero(v))
				break;
			i += sizeof(size_t);
		}
		while (i < end);
	}
	while (i < n)
	{
		if (((uint8_t*)s)[i] == (uint8_t)c)
			return (uint8_t*)s + i;
		i++;
	}
	return NULL;
}

void *memrchr(const void *s, int c, size_t n)
{
	if (n >= sizeof(size_t))
	{
		size_t cz = fillsize((uint8_t)c);
		do
		{
			size_t nxt = n - sizeof(size_t);
			size_t v = *(size_t*)&((uint8_t*)s)[nxt] ^ cz;
			if (haszero(v))
				break;
			n = nxt;
		} while (n >= sizeof(size_t));
	}
	while (n)
	{
		size_t nxt = n - 1;
		uint8_t *ptr = &((uint8_t*)s)[nxt];
		if (*ptr == (uint8_t)c)
			return ptr;
		n = nxt;
	}
	return NULL;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	size_t i = 0;
	if (n >= sizeof(size_t))
	{
		size_t end = n - n % sizeof(size_t);
		do
		{
			if (*(size_t*)&((uint8_t*)s1)[i] != *(size_t*)&((uint8_t*)s2)[i])
				break;
			i += sizeof(size_t);
		} while (i < end);
	}
	while (i < n)
	{
		uint8_t d = ((uint8_t*)s1)[i] - ((uint8_t*)s2)[i];
		if (d)
			return d;
		i++;
	}
	return 0;
}

void *memmem(const void *haystack, size_t haystacklen, const void *needle,
             size_t needlelen)
{
	if (needlelen > haystacklen)
		return NULL;
	haystacklen -= needlelen;
	for (size_t i = 0; i <= haystacklen; ++i)
	{
		if (!memcmp(&((uint8_t*)haystack)[i], needle, needlelen))
			return &((uint8_t*)haystack)[i];
	}
	return NULL;
}

size_t strlen(const char *s)
{
	size_t i = 0;
	while (((size_t)s + i) % sizeof(size_t))
	{
		if (!s[i])
			return i;
		i++;
	}
	while (1)
	{
		size_t v = *(size_t*)&s[i];
		if (haszero(v))
			break;
		i += sizeof(size_t);
	}
	while (s[i])
		i++;
	return i;
}

size_t strnlen(const char *s, size_t maxlen)
{
	size_t i = 0;
	if (i < maxlen && ((size_t)s + i) % sizeof(size_t))
	{
		if (!s[i])
			return i;
		i++;
	}
	while (i + sizeof(size_t) <= maxlen)
	{
		size_t v = *(size_t*)&s[i];
		if (haszero(v))
			break;
		i += sizeof(size_t);
	}
	while (i < maxlen && s[i])
		i++;
	return i;
}

char *strcpy(char *d, const char *s)
{
	size_t i = 0;
	while (((size_t)s + i) % sizeof(size_t))
	{
		uint8_t v = s[i];
		d[i] = v;
		if (!v)
			return d;
		i++;
	}
	while (1)
	{
		size_t v = *(size_t*)&s[i];
		if (haszero(v))
			break;
		*(size_t*)&d[i] = v;
		i += sizeof(size_t);
	}
	while (1)
	{
		uint8_t v = s[i];
		d[i] = v;
		if (!v)
			break;
		i++;
	}
	return d;
}

char *strncpy(char *d, const char *s, size_t n)
{
	size_t i = 0;
	while (1)
	{
		if (i >= n)
			return d;
		if (!(((size_t)s + i) % sizeof(size_t)))
			break;
		uint8_t v = s[i];
		d[i] = v;
		i++;
		if (!v)
			goto end;
	}
	while (i + sizeof(size_t) <= n)
	{
		size_t v = *(size_t*)&s[i];
		if (haszero(v))
			break;
		*(size_t*)&d[i] = v;
		i += sizeof(size_t);
	}
	while (1)
	{
		if (i >= n)
			return d;
		uint8_t v = s[i];
		d[i] = v;
		i++;
		if (!v)
			break;
	}
end:
	memset(&d[i], 0, n - i);
	return d;
}

size_t strlcpy(char *d, const char *s, size_t n)
{
	if (!n)
		return strlen(s);
	size_t i = 0;
	while (1)
	{
		if (i >= n - 1)
			goto end;
		if (!(((size_t)s + i) % sizeof(size_t)))
			break;
		uint8_t v = s[i];
		d[i] = v;
		if (!v)
			return i;
		i++;
	}
	while (i + sizeof(size_t) < n)
	{
		size_t v = *(size_t*)&s[i];
		if (haszero(v))
			break;
		*(size_t*)&d[i] = v;
		i += sizeof(size_t);
	}
	while (i < n - 1)
	{
		uint8_t v = s[i];
		d[i] = v;
		if (!v)
			return i;
		i++;
	}
end:
	d[i] = '\0';
	return i + strlen(&s[i]);
}

char *strcat(char *d, const char *s)
{
	size_t i = strlen(d);
	strcpy(&d[i], s);
	return d;
}

char *strncat(char *d, const char *s, size_t n)
{
	size_t i = strlen(d);
	size_t j = 0;
	while (1)
	{
		if (j >= n)
			goto end;
		if (!(((size_t)s + j) % sizeof(size_t)))
			break;
		uint8_t v = s[j];
		d[i] = v;
		if (!v)
			return d;
		i++;
		j++;
	}
	while (j + sizeof(size_t) <= n)
	{
		size_t v = *(size_t*)&s[j];
		if (haszero(v))
			break;
		*(size_t*)&d[i] = v;
		i += sizeof(size_t);
		j += sizeof(size_t);
	}
	while (j < n)
	{
		uint8_t v = s[j];
		d[i] = v;
		if (!v)
			return d;
		i++;
		j++;
	}
end:
	d[i] = '\0';
	return d;
}

size_t strlcat(char *d, const char *s, size_t n)
{
	if (!n)
		return strlen(d) + strlen(s);
	size_t i = strlen(d);
	if (i >= n)
		return i + strlen(s);
	size_t j = 0;
	while (1)
	{
		if (i >= n - 1)
			goto end;
		if (!(((size_t)s + j) % sizeof(size_t)))
			break;
		uint8_t v = s[j];
		d[i] = v;
		if (!v)
			return i;
		i++;
		j++;
	}
	while (i + sizeof(size_t) < n)
	{
		size_t v = *(size_t*)&s[j];
		if (haszero(v))
			break;
		*(size_t*)&d[i] = v;
		i += sizeof(size_t);
		j += sizeof(size_t);
	}
	while (i < n - 1)
	{
		uint8_t v = s[j];
		d[i] = v;
		if (!v)
			return i;
		i++;
		j++;
	}
end:
	d[i] = '\0';
	return i + strlen(&s[j]);
}

char *strchr(const char *s, int c)
{
	size_t i = 0;
	while (((size_t)s + i) % sizeof(size_t))
	{
		uint8_t v = (uint8_t)s[i];
		if (v == (uint8_t)c)
			return (char*)&s[i];
		if (!v)
			return (uint8_t)c ? NULL : (char*)&s[i];
		i++;
	}
	size_t cz = fillsize((uint8_t)c);
	while (1)
	{
		size_t v = *(size_t*)&s[i];
		if (haszero(v))
			break;
		v ^= cz;
		if (haszero(v))
			break;
		i += sizeof(size_t);
	}
	while (1)
	{
		uint8_t v = (uint8_t)s[i];
		if (v == (uint8_t)c)
			return (char*)&s[i];
		if (!v)
			break;
		i++;
	}
	return NULL;
}

char *strchrnul(const char *s, int c)
{
	size_t i = 0;
	while (((size_t)s + i) % sizeof(size_t))
	{
		uint8_t v = (uint8_t)s[i];
		if (v == (uint8_t)c)
			return (char*)&s[i];
		if (!v)
			return (char*)&s[i];
		i++;
	}
	size_t cz = fillsize((uint8_t)c);
	while (1)
	{
		size_t v = *(size_t*)&s[i];
		if (haszero(v))
			break;
		v ^= cz;
		if (haszero(v))
			break;
		i += sizeof(size_t);
	}
	while (1)
	{
		uint8_t v = (uint8_t)s[i];
		if (v == (uint8_t)c)
			return (char*)&s[i];
		if (!v)
			break;
		i++;
	}
	return (char*)&s[i];
}

char *strrchr(const char *s, int c)
{
	char *ret = NULL;
	size_t i = 0;
	while (((size_t)s + i) % sizeof(size_t))
	{
		uint8_t v = (uint8_t)s[i];
		if (v == (uint8_t)c)
			ret = (char*)&s[i];
		if (!v)
			return (uint8_t)c ? ret : (char*)&s[i];
		i++;
	}
	size_t cz = fillsize((uint8_t)c);
	while (1)
	{
		size_t v = *(size_t*)&s[i];
		if (haszero(v))
			break;
		v ^= cz;
		if (haszero(v))
			break;
		i += sizeof(size_t);
	}
	while (1)
	{
		uint8_t v = (uint8_t)s[i];
		if (v == (uint8_t)c)
			ret = (char*)&s[i];
		if (!v)
			break;
		i++;
	}
	return ret;
}

char *strstr(const char *s1, const char *s2)
{
	if (!*s2)
		return (char*)s1;
	for (size_t i = 0; s1[i]; ++i)
	{
		size_t j;
		for (j = 0; s1[i + j] == s2[j]; ++j)
		{
			if (!s2[j])
				return (char*)s1 + i;
		}
		if (!s2[j])
			return (char*)s1 + i;
	}
	return NULL;
}

char *strnstr(const char *s1, const char *s2, size_t n)
{
	if (!*s2)
		return (char*)s1;
	for (size_t i = 0; i < n && s1[i]; ++i)
	{
		for (size_t j = 0; (s1[i + j] == s2[j] || !s2[j]); ++j)
		{
			if (!s2[j])
				return (char*)s1 + i;
			if (i + j >= n)
				return NULL;
		}
	}
	return NULL;
}

int strcmp(const char *s1, const char *s2)
{
	size_t i = 0;
	while (s1[i] && s2[i] && s1[i] == s2[i])
		i++;
	return (int)((uint8_t*)s1)[i] - (int)((uint8_t*)s2)[i];
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	size_t i;
	for (i = 0; i < n && s1[i] && s2[i]; ++i)
	{
		uint8_t d = (int)((uint8_t*)s1)[i] - (int)((uint8_t*)s2)[i];
		if (d)
			return d;
	}
	if (i == n)
		return 0;
	return (int)((uint8_t*)s1)[i] - (int)((uint8_t*)s2)[i];
}

char *strdup(const char *s)
{
	size_t len = strlen(s);
	char *ret = malloc(len + 1, 0);
	if (!ret)
		return NULL;
	memcpy(ret, s, len + 1);
	return ret;
}

char *strndup(const char *s, size_t n)
{
	size_t len = strlen(s);
	if (n < len)
		len = n;
	char *ret = malloc(len + 1, 0);
	if (!ret)
		return NULL;
	memcpy(ret, s, len);
	ret[len] = '\0';
	return ret;
}

static const char *g_err_str[] =
{
	[E2BIG          ] = "Argument list too long",
	[EACCES         ] = "Permission denied",
	[EADDRINUSE     ] = "Address in use",
	[EADDRNOTAVAIL  ] = "Address not available",
	[EAFNOSUPPORT   ] = "Address family not supported",
	[EAGAIN         ] = "Resource unavailable, try again",
	[EALREADY       ] = "Connection already in progress",
	[EBADF          ] = "Bad file descriptor",
	[EBADMSG        ] = "Bad message",
	[EBUSY          ] = "Device or resource busy",
	[ECANCELED      ] = "Operation canceled",
	[ECHILD         ] = "No child processes",
	[ECONNABORTED   ] = "Connection aborted",
	[ECONNREFUSED   ] = "Connection refused",
	[ECONNRESET     ] = "Connection reset",
	[EDEADLK        ] = "Resource deadlock would occur",
	[EDESTADDRREQ   ] = "Destination address required",
	[EDOM           ] = "Mathematics argument out of domain of function",
	[EDQUOT         ] = "Reserved",
	[EEXIST         ] = "File exists",
	[EFAULT         ] = "Bad address",
	[EFBIG          ] = "File too large",
	[EHOSTUNREACH   ] = "Host is unreachable",
	[EIDRM          ] = "Identifier removed",
	[EILSEQ         ] = "Illegal byte sequence",
	[EINPROGRESS    ] = "Operation in progress",
	[EINTR          ] = "Interrupted function",
	[EINVAL         ] = "Invalid argument",
	[EIO            ] = "I/O error",
	[EISCONN        ] = "Socket is connected",
	[EISDIR         ] = "Is a directory",
	[ELOOP          ] = "Too many levels of symbolic links",
	[EMFILE         ] = "File descriptor value too large",
	[EMLINK         ] = "Too many links",
	[EMSGSIZE       ] = "Message too large",
	[EMULTIHOP      ] = "Reserved",
	[ENAMETOOLONG   ] = "Filename too long",
	[ENETDOWN       ] = "Network is down",
	[ENETRESET      ] = "Connection aborted by network",
	[ENETUNREACH    ] = "Network unreachable",
	[ENFILE         ] = "Too many files open in system",
	[ENOBUFS        ] = "No buffer space available",
	[ENODATA        ] = "No message is available on the STREAM head read queue",
	[ENODEV         ] = "No such device",
	[ENOENT         ] = "No such file or directory",
	[ENOEXEC        ] = "Executable file format error",
	[ENOLCK         ] = "No locks available",
	[ENOLINK        ] = "Reserved",
	[ENOMEM         ] = "Not enough space",
	[ENOMSG         ] = "No message of the desired type",
	[ENOPROTOOPT    ] = "Protocol not available",
	[ENOSPC         ] = "No space left on device",
	[ENOSR          ] = "No STREAM resources",
	[ENOSTR         ] = "Not a STREAM",
	[ENOSYS         ] = "Functionality not supported",
	[ENOTCONN       ] = "The socket is not connected",
	[ENOTDIR        ] = "Not a directory or a symbolic link to a directory",
	[ENOTEMPTY      ] = "Directory not empty",
	[ENOTRECOVERABLE] = "State not recoverable",
	[ENOTSOCK       ] = "Not a socket",
	[ENOTTY         ] = "Inappropriate I/O control operation",
	[ENXIO          ] = "No such device or address",
	[EOPNOTSUPP     ] = "Operation not supported on socket",
	[EOVERFLOW      ] = "Value too large to be stored in data type",
	[EOWNERDEAD     ] = "Previous owner died",
	[EPERM          ] = "Operation not permitted",
	[EPIPE          ] = "Broken pipe",
	[EPROTO         ] = "Protocol error",
	[EPROTONOSUPPORT] = "Protocol not supported",
	[EPROTOTYPE     ] = "Protocol wrong type for socket",
	[ERANGE         ] = "Result too large",
	[EROFS          ] = "Read-only file system",
	[ESPIPE         ] = "Invalid seek",
	[ESRCH          ] = "No such process",
	[ESTALE         ] = "Reserved",
	[ETIME          ] = "Stream ioctl() timeout",
	[ETIMEDOUT      ] = "Connection timed out",
	[ETXTBSY        ] = "Text file busy",
	[EXDEV          ] = "Cross-device link",
};

char *strerror(int errnum)
{
	if (errnum >= 0)
		return "Unknown error";
	errnum = -errnum;
	if ((unsigned)errnum >= sizeof(g_err_str) / sizeof(*g_err_str))
		return "Unknown error";
	char *ret = (char*)g_err_str[errnum];
	if (ret)
		return ret;
	return "Unknown error";
}
