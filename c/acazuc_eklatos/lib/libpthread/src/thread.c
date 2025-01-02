#include <eklat/syscall.h>

#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sched.h>
#include <errno.h>

#define PTHREAD_FL_JOINING  (1 << 0)
#define PTHREAD_FL_DETACHED (1 << 1)
#define PTHREAD_FL_EXITED   (1 << 2)

void *_dl_tls_alloc(void);
void _dl_tls_free(void *ptr);
int _dl_tls_set(void *ptr);

pid_t _pthread_fork(pthread_t pthread);
__attribute__((noreturn))
pid_t _pthread_exit(void *stack, size_t stack_size);

__thread pthread_t _self;
static struct pthread _leader; /* XXX mark as detached, should be initialized
                                * in a constructor
                                * BTW, init function should be called 
                                * inside crt0 instead of ld.so (otherwise
                                * constructors will be called before crt0
                                * has been called, and this is not a good idea)
                                */

int pthread_attr_init(pthread_attr_t *attr)
{

	if (!attr)
		return EINVAL;
	attr->stack_size = 1024 * 1024;
	attr->stack = NULL;
	attr->detachstate = PTHREAD_CREATE_DETACHED;
	return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr)
{
	if (!attr)
		return EINVAL;
	return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t size)
{
	if (!attr)
		return EINVAL;
	if (size < PTHREAD_STACK_MIN)
		return EINVAL;
	attr->stack_size = (size + 4095) & ~4095; /* XXX page size */
	attr->stack = NULL;
	return 0;
}

int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *size)
{
	if (!attr)
		return EINVAL;
	if (size)
		*size = attr->stack_size;
	return 0;
}

int pthread_attr_setstack(pthread_attr_t *attr, void *stack, size_t size)
{
	if (!attr)
		return EINVAL;
	if (size < PTHREAD_STACK_MIN)
		return EINVAL;
	uintptr_t end;
	if (__builtin_add_overflow((uintptr_t)stack, size, &end))
		return EINVAL;
	if (end & 0xF)
		return EINVAL;
	attr->stack = stack;
	attr->stack_size = size;
	return 0;
}

int pthread_attr_getstack(const pthread_attr_t *attr, void **stack, size_t *size)
{
	if (!attr)
		return EINVAL;
	if (stack)
		*stack = attr->stack;
	if (size)
		*size = attr->stack_size;
	return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
	if (!attr)
		return EINVAL;
	if (detachstate != PTHREAD_CREATE_JOINABLE
	 && detachstate != PTHREAD_CREATE_DETACHED)
		return EINVAL;
	attr->detachstate = detachstate;
	return 0;
}

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{
	if (!attr)
		return EINVAL;
	if (detachstate)
		*detachstate = attr->detachstate;
	return 0;
}

static void pthread_destroy(pthread_t thread)
{
	if (!thread)
		return;
	if (thread->stack && !(thread->flags & PTHREAD_FL_EXITED))
		munmap(thread->stack, thread->stack_size);
	pthread_mutex_destroy(&thread->mutex);
	pthread_cond_destroy(&thread->cond);
	void *tls = thread->dl_tls;
	free(thread);
	_dl_tls_free(tls);
}

__attribute__((noreturn))
void _pthread_start(pthread_t thread)
{
	if (!_dl_tls_set(thread->dl_tls))
		_exit(1);
	_self = thread;
	pthread_exit(thread->start(thread->arg));
}

int pthread_create(pthread_t *threadp, const pthread_attr_t *attr,
                   void *(*start)(void*), void *arg)
{
	pthread_t thread = calloc(1, sizeof(*thread));
	if (!thread)
		return ENOMEM;
	int ret = pthread_mutex_init(&thread->mutex, NULL);
	if (ret)
	{
		free(thread);
		return ret;
	}
	ret = pthread_cond_init(&thread->cond, NULL);
	if (ret)
	{
		pthread_mutex_destroy(&thread->mutex);
		free(thread);
		return ret;
	}
	thread->start = start;
	thread->arg = arg;
	if (attr)
	{
		thread->stack_size = attr->stack_size;
		if (attr->stack)
			thread->stack = attr->stack;
		else
			thread->stack = mmap(NULL, thread->stack_size,
			                     PROT_READ | PROT_WRITE,
			                     MAP_ANONYMOUS | MAP_PRIVATE,
			                     -1, 0);
		if (attr->detachstate == PTHREAD_CREATE_DETACHED)
			thread->flags |= PTHREAD_FL_DETACHED;
	}
	else
	{
		thread->stack_size = 1024 * 1024 * 1;
		thread->stack = mmap(NULL, thread->stack_size,
		                     PROT_READ | PROT_WRITE,
		                     MAP_ANONYMOUS | MAP_PRIVATE,
		                     -1, 0);
	}
	if (thread->stack == MAP_FAILED)
	{
		pthread_destroy(thread);
		return ENOMEM;
	}
	thread->dl_tls = _dl_tls_alloc();
	if (!thread->dl_tls)
	{
		pthread_destroy(thread);
		return ENOMEM;
	}
	pid_t tid = _pthread_fork(thread);
	if (tid == -1)
	{
		pthread_destroy(thread);
		return errno;
	}
	thread->tid = tid;
	/* XXX wait for thread to finish setup ? */
	*threadp = thread;
	return 0;
}

int pthread_detach(pthread_t thread)
{
	if (thread == pthread_self())
		return EINVAL;
	pthread_mutex_lock(&thread->mutex);
	if (thread->flags & (PTHREAD_FL_DETACHED | PTHREAD_FL_JOINING))
	{
		pthread_mutex_unlock(&thread->mutex);
		return EINVAL;
	}
	if (thread->flags & PTHREAD_FL_EXITED)
	{
		pthread_mutex_unlock(&thread->mutex);
		pthread_destroy(thread);
		return 0;
	}
	thread->flags |= PTHREAD_FL_DETACHED;
	pthread_mutex_unlock(&thread->mutex);
	return 0;
}

int pthread_join(pthread_t thread, void **retv)
{
	if (thread == pthread_self())
		return EDEADLK;
	pthread_mutex_lock(&thread->mutex);
	if (thread->flags & (PTHREAD_FL_DETACHED | PTHREAD_FL_JOINING))
	{
		pthread_mutex_unlock(&thread->mutex);
		return EINVAL;
	}
	if (thread->flags & PTHREAD_FL_EXITED)
		goto end;
	thread->flags |= PTHREAD_FL_JOINING;
	int ret = pthread_cond_wait(&thread->cond, &thread->mutex);
	if (ret)
	{
		thread->flags &= ~PTHREAD_FL_JOINING;
		pthread_mutex_unlock(&thread->mutex);
		return ret;
	}
end:
	if (retv)
		*retv = thread->ret;
	pthread_destroy(thread);
	return 0;
}

int pthread_kill(pthread_t thread, int sig)
{
	if (!thread)
		return EINVAL;
	if (!kill(thread->tid, sig))
		return 0;
	return errno;
}

void pthread_exit(void *ret)
{
	pthread_t self = pthread_self();
	pthread_mutex_lock(&self->mutex);
	self->flags |= PTHREAD_FL_EXITED;
	_pthread_key_cleanup(self);
	void *stack = self->stack;
	size_t stack_size = self->stack_size;
	if (self->flags & PTHREAD_FL_DETACHED)
	{
		pthread_destroy(self);
	}
	else
	{
		self->ret = ret;
		pthread_cond_broadcast(&self->cond);
		pthread_mutex_unlock(&self->mutex);
	}
	/* XXX if the thread gets killed here, the stack leaks... */
	syscall2_raw(SYS_munmap, (uintptr_t)stack, stack_size);
	syscall1_raw(SYS_exit, 0);
	while (1);
}

int pthread_cancel(pthread_t thread)
{
	(void)thread;
	/* XXX */
	return EINVAL;
}

pthread_t pthread_self(void)
{
	if (_self)
		return _self;
	return &_leader;
}

int pthread_compare(pthread_t t1, pthread_t t2)
{
	return t1 != t2;
}

int pthread_equal(pthread_t t1, pthread_t t2)
{
	return t1 == t2;
}

int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset)
{
	return sigprocmask(how, set, oldset);
}

int __libc_atfork(void (*prepare)(void), void (*parent)(void),
                  void (*child)(void));

int pthread_atfork(void (*prepare)(void), void (*parent)(void),
                   void (*child)(void))
{
	return __libc_atfork(prepare, parent, child);
}

int pthread_setschedprio(pthread_t thread, int prio)
{
	if (setpriority(PRIO_PROCESS, thread->tid, prio) == -1)
		return errno;
	return 0;
}
