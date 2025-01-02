#include <pipebuf.h>
#include <errno.h>
#include <file.h>
#include <poll.h>
#include <uio.h>

int pipebuf_init(struct pipebuf *pipebuf, size_t size, struct mutex *mutex,
                 struct waitq *rwaitq, struct waitq *wwaitq)
{
	int ret = ringbuf_init(&pipebuf->ringbuf, size);
	if (ret)
		return ret;
	pipebuf->mutex = mutex;
	pipebuf->rwaitq = rwaitq;
	pipebuf->wwaitq = wwaitq;
	pipebuf->nreaders = 0;
	pipebuf->nwriters = 0;
	return 0;
}

void pipebuf_destroy(struct pipebuf *pipebuf)
{
	ringbuf_destroy(&pipebuf->ringbuf);
}

ssize_t pipebuf_read_locked(struct pipebuf *pipebuf, struct uio *uio, size_t min,
                            struct timespec *timeout)
{
	size_t rd = 0;
	while (uio->count)
	{
		size_t available = ringbuf_read_size(&pipebuf->ringbuf);
		if (!available)
		{
			if (!pipebuf->nwriters || rd)
				break;
		}
		size_t read_sz = uio->count;
		if (read_sz > PIPE_BUF)
			read_sz = PIPE_BUF;
		while (!available)
		{
			if (!pipebuf->nwriters)
			{
				read_sz = available;
				break;
			}
			if (rd >= min || !pipebuf->rwaitq)
			{
				if (rd)
					return rd;
				return -EAGAIN;
			}
			int ret;
			if (rd)
				ret = waitq_wait_head_mutex(pipebuf->rwaitq,
				                            pipebuf->mutex,
				                            timeout);
			else
				ret = waitq_wait_tail_mutex(pipebuf->rwaitq,
				                            pipebuf->mutex,
				                            timeout);
			if (ret)
			{
				if (ret != -EINTR || !rd)
					return ret;
				if (ret == -EINTR)
				{
					read_sz = 0;
					break;
				}
			}
			available = ringbuf_read_size(&pipebuf->ringbuf);
		}
		if (!read_sz)
			break;
		if (available < read_sz)
			read_sz = available;
		ssize_t ret = ringbuf_readuio(&pipebuf->ringbuf, uio, read_sz);
		if (pipebuf->wwaitq)
			waitq_broadcast(pipebuf->wwaitq, 0);
		if (ret < 0)
			return ret;
		rd += ret;
	}
	return rd;
}

ssize_t pipebuf_read(struct pipebuf *pipebuf, struct uio *uio, size_t min,
                     struct timespec *timeout)
{
	pipebuf_lock(pipebuf);
	ssize_t ret = pipebuf_read_locked(pipebuf, uio, min, timeout);
	pipebuf_unlock(pipebuf);
	return ret;
}

ssize_t pipebuf_write_locked(struct pipebuf *pipebuf, struct uio *uio, size_t min,
                             struct timespec *timeout)
{
	size_t wr = 0;
	if (!pipebuf->nreaders)
		return -EPIPE;
	while (uio->count)
	{
		size_t available = ringbuf_write_size(&pipebuf->ringbuf);
		if (!available && !pipebuf->nreaders)
			break;
		size_t write_sz = uio->count;
		if (write_sz > PIPE_BUF)
			write_sz = PIPE_BUF;
		while (!available)
		{
			if (!pipebuf->nreaders)
			{
				write_sz = available;
				break;
			}
			if (wr >= min || !pipebuf->wwaitq)
			{
				if (wr)
					return wr;
				return -EAGAIN;
			}
			int ret;
			if (wr)
				ret = waitq_wait_head_mutex(pipebuf->wwaitq,
				                            pipebuf->mutex,
				                            timeout);
			else
				ret = waitq_wait_tail_mutex(pipebuf->wwaitq,
				                            pipebuf->mutex,
				                            timeout);
			if (ret)
			{
				if (ret != -EINTR || !wr)
					return ret;
				if (ret == -EINTR)
				{
					write_sz = 0;
					break;
				}
			}
			available = ringbuf_write_size(&pipebuf->ringbuf);
		}
		if (!write_sz)
			break;
		ssize_t ret = ringbuf_writeuio(&pipebuf->ringbuf, uio, write_sz);
		if (pipebuf->rwaitq)
			waitq_broadcast(pipebuf->rwaitq, 0);
		if (ret < 0)
			return ret;
		wr += ret;
	}
	return wr;
}

ssize_t pipebuf_write(struct pipebuf *pipebuf, struct uio *uio, size_t min,
                      struct timespec *timeout)
{
	pipebuf_lock(pipebuf);
	ssize_t ret = pipebuf_write_locked(pipebuf, uio, min, timeout);
	pipebuf_unlock(pipebuf);
	return ret;
}

int pipebuf_poll_locked(struct pipebuf *pipebuf, int events)
{
	int ret = 0;
	if (events & POLLIN)
	{
		if (ringbuf_read_size(&pipebuf->ringbuf))
			ret |= POLLIN;
		if (!pipebuf->nwriters)
			ret |= POLLHUP;
	}
	if (events & POLLOUT)
	{
		if (ringbuf_write_size(&pipebuf->ringbuf))
			ret |= POLLOUT;
		if (!pipebuf->nreaders)
			ret |= POLLERR;
	}
	return ret;
}

int pipebuf_poll(struct pipebuf *pipebuf, int events)
{
	pipebuf_lock(pipebuf);
	int ret = pipebuf_poll_locked(pipebuf, events);
	pipebuf_unlock(pipebuf);
	return ret;
}
