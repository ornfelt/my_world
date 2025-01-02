#include <proc.h>
#include <cpu.h>
#include <uio.h>
#include <std.h>
#include <mem.h>

ssize_t uio_copyout(void *dst, struct uio *uio, size_t count)
{
	size_t wr = 0;
	if (count > uio->count)
		count = uio->count;
	while (count && uio->iovcnt)
	{
		struct iovec *iov = &uio->iov[0];
		if (!iov->iov_len)
		{
			uio->iov++;
			uio->iovcnt--;
			continue;
		}
		size_t wr_size = iov->iov_len;
		if (wr_size > count)
			wr_size = count;
		if (uio->userbuf)
		{
			int ret = vm_copyin(curcpu()->thread->proc->vm_space,
			                    dst, iov->iov_base, wr_size);
			if (ret)
				return ret;
		}
		else
		{
			memcpy(dst, iov->iov_base, wr_size);
		}
		wr += wr_size;
		count -= wr_size;
		iov->iov_base = (uint8_t*)iov->iov_base + wr_size;
		iov->iov_len -= wr_size;
		uio->count -= wr_size;
		uio->off += wr_size;
		dst = (uint8_t*)dst + wr_size;
	}
	return wr;
}

ssize_t uio_copyin(struct uio *uio, const void *src, size_t count)
{
	size_t rd = 0;
	if (count > uio->count)
		count = uio->count;
	while (count && uio->iovcnt)
	{
		struct iovec *iov = &uio->iov[0];
		if (!iov->iov_len)
		{
			uio->iov++;
			uio->iovcnt--;
			continue;
		}
		size_t rd_size = iov->iov_len;
		if (rd_size > count)
			rd_size = count;
		if (uio->userbuf)
		{
			int ret = vm_copyout(curcpu()->thread->proc->vm_space,
			                     iov->iov_base, src, rd_size);
			if (ret)
				return ret;
		}
		else
		{
			memcpy(iov->iov_base, src, rd_size);
		}
		rd += rd_size;
		count -= rd_size;
		iov->iov_base = (uint8_t*)iov->iov_base + rd_size;
		iov->iov_len -= rd_size;
		uio->count -= rd_size;
		uio->off += rd_size;
		src = (uint8_t*)src + rd_size;
	}
	return rd;
}

ssize_t uio_copyz(struct uio *uio, size_t count)
{
	size_t wr = 0;
	if (count > uio->count)
		count = uio->count;
	while (count && uio->iovcnt)
	{
		struct iovec *iov = &uio->iov[0];
		if (!iov->iov_len)
		{
			uio->iov++;
			uio->iovcnt--;
			continue;
		}
		size_t wr_size = iov->iov_len;
		if (wr_size > count)
			wr_size = count;
		if (wr_size > PAGE_SIZE)
			wr_size = PAGE_SIZE;
		if (uio->userbuf)
		{
			static const uint8_t zero_buf[PAGE_SIZE];
			int ret = vm_copyout(curcpu()->thread->proc->vm_space,
			                     iov->iov_base, zero_buf, wr_size);
			if (ret)
				return ret;
		}
		else
		{
			memset(iov->iov_base, 0, wr_size);
		}
		wr += wr_size;
		count -= wr_size;
		iov->iov_base = (uint8_t*)iov->iov_base + wr_size;
		iov->iov_len -= wr_size;
		uio->count -= wr_size;
		uio->off += wr_size;
	}
	return wr;
}

size_t uio_advance(struct uio *uio, size_t count)
{
	size_t nb = 0;
	if (count > uio->count)
		count = uio->count;
	while (count && uio->iovcnt)
	{
		struct iovec *iov = &uio->iov[0];
		if (!iov->iov_len)
		{
			uio->iov++;
			uio->iovcnt--;
			continue;
		}
		size_t size = iov->iov_len;
		if (size > count)
			size = count;
		nb += size;
		count -= size;
		iov->iov_base = (uint8_t*)iov->iov_base + size;
		iov->iov_len -= size;
		uio->count -= size;
		uio->off += size;
	}
	return nb;
}
