#ifndef UIO_H
#define UIO_H

#include <types.h>

#define IOV_MAX 1024

struct iovec
{
	void *iov_base;
	size_t iov_len;
};

struct uio
{
	struct iovec *iov;
	size_t iovcnt;
	size_t count;
	off_t off;
	int userbuf;
};

ssize_t uio_copyout(void *dst, struct uio *uio, size_t count);
ssize_t uio_copyin(struct uio *uio, const void *src, size_t count);
ssize_t uio_copyz(struct uio *uio, size_t count);
size_t uio_advance(struct uio *uio, size_t count);

static inline void uio_fromkbuf(struct uio *uio, struct iovec *iov, void *data,
                                size_t len, off_t off)
{
	iov->iov_base = data;
	iov->iov_len = len;
	uio->iov = iov;
	uio->iovcnt = 1;
	uio->count = len;
	uio->off = off;
	uio->userbuf = 0;
}

#endif
