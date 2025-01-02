#include <net/net.h>

#include <resource.h>
#include <syscall.h>
#include <ptrace.h>
#include <endian.h>
#include <reboot.h>
#include <sched.h>
#include <errno.h>
#include <futex.h>
#include <sock.h>
#include <wait.h>
#include <proc.h>
#include <file.h>
#include <proc.h>
#include <time.h>
#include <pipe.h>
#include <poll.h>
#include <kmod.h>
#if WITH_ACPI
#include <acpi.h>
#endif
#include <ipc.h>
#include <vfs.h>
#include <std.h>
#include <uio.h>
#include <cpu.h>
#include <mem.h>

#define DEBUG_SYSCALL        0
#define DEBUG_SYSCALL_VMSIZE 0
#define DEBUG_SYSCALL_TIME   0
#define DEBUG_SYSCALL_PREFIX 0

struct utsname
{
	char sysname[256];
	char nodename[256];
	char release[256];
	char version[256];
	char machine[256];
};

static ssize_t is_node_rofs(struct node *node)
{
	return node->sb && (node->sb->flags & ST_RDONLY);
}

static ssize_t update_node_times(struct node *node, fs_attr_mask_t mask)
{
	struct timespec ts;
	struct fs_attr attr;
	ssize_t ret;

	if (is_node_rofs(node))
		return -EROFS;
	ret = clock_gettime(CLOCK_REALTIME, &ts);
	if (ret < 0)
		return ret;
	if (mask & FS_ATTR_ATIME)
		attr.atime = ts;
	if (mask & FS_ATTR_CTIME)
		attr.ctime = ts;
	if (mask & FS_ATTR_MTIME)
		attr.mtime = ts;
	ret = node_setattr(node, mask, &attr);
	if (ret < 0)
		return ret;
	return 0;
}

static ssize_t getnode(struct thread *thread, int fd, struct node **node)
{
	struct file *file;
	ssize_t ret;

	ret = proc_getfile(thread->proc, fd, &file);
	if (ret < 0)
		return ret;
	*node = file->node;
	if (!*node)
	{
		file_free(file);
		return -EINVAL;
	}
	node_ref(*node);
	file_free(file);
	return 0;
}

static ssize_t getcwdat(struct thread *thread, int dirfd, const char *pathname,
                        int flags, struct node **cwd)
{
	ssize_t ret;

	if (!(flags & AT_EMPTY_PATH) && !pathname[0])
		return -ENOENT;
	if (dirfd == AT_FDCWD || *pathname == '/')
	{
		*cwd = thread->proc->cwd;
		node_ref(*cwd);
		return 0;
	}
	ret = getnode(thread, dirfd, cwd);
	if (ret < 0)
		return ret;
	if (pathname && !S_ISDIR((*cwd)->attr.mode))
	{
		node_free(*cwd);
		return -ENOTDIR;
	}
	return 0;
}

static ssize_t getnodeat(struct thread *thread, int dirfd, const char *pathname,
                         int flags, struct node **node)
{
	struct node *cwd;
	ssize_t ret;
	int vfs_flags = 0;

	if (!pathname[0] && (flags & AT_EMPTY_PATH))
	{
		struct file *file;
		ret = proc_getfile(thread->proc, dirfd, &file);
		if (ret < 0)
			return ret;
		if (!file->node)
		{
			file_free(file);
			return -EBADF;
		}
		*node = file->node;
		node_ref(*node);
		file_free(file);
		return 0;
	}
	ret = getcwdat(thread, dirfd, pathname, flags, &cwd);
	if (ret < 0)
		return ret;
	if (flags & AT_SYMLINK_NOFOLLOW)
		vfs_flags |= VFS_NOFOLLOW;
	ret = vfs_getnode(cwd, pathname, vfs_flags, node);
	node_free(cwd);
	return ret;
}

static ssize_t getdirat(struct thread *thread, int dirfd, const char *pathname,
                        int flags, struct node **dir, char **end_fn)
{
	struct node *cwd;
	ssize_t ret;

	ret = getcwdat(thread, dirfd, pathname, flags, &cwd);
	if (ret < 0)
		return ret;
	ret = vfs_getdir(cwd, pathname, 0, dir, end_fn);
	node_free(cwd);
	return ret;
}

static ssize_t getsock(struct thread *thread, int fd, struct sock **sock)
{
	struct file *file;
	ssize_t ret;

	ret = proc_getfile(thread->proc, fd, &file);
	if (ret < 0)
		return ret;
	*sock = file->sock;
	if (!*sock)
	{
		file_free(file);
		return -ENOTSOCK;
	}
	sock_ref(*sock);
	file_free(file);
	return 0;
}

static void rusage_from_stats(struct rusage *rusage, struct procstat *stats)
{
	rusage->ru_utime.tv_sec = stats->utime.tv_sec;
	rusage->ru_utime.tv_usec = stats->utime.tv_nsec / 1000;
	rusage->ru_stime.tv_sec = stats->stime.tv_sec;
	rusage->ru_stime.tv_usec = stats->stime.tv_nsec / 1000;
	rusage->ru_maxrss = 0;
	rusage->ru_ixrss = 0;
	rusage->ru_idrss = 0;
	rusage->ru_isrss = 0;
	rusage->ru_minflt = stats->faults;
	rusage->ru_majflt = 0;
	rusage->ru_nswap = 0;
	rusage->ru_inblock = 0;
	rusage->ru_outblock = 0;
	rusage->ru_msgsnd = stats->msgsnd;
	rusage->ru_msgrcv = stats->msgrcv;
	rusage->ru_nsignals = stats->nsignals;
	rusage->ru_nvcsw = stats->nctxsw;
	rusage->ru_nivcsw = 0;
}

static int test_sockaddr_len(int family, socklen_t addrlen)
{
	switch (family)
	{
		case AF_INET:
			if (addrlen < sizeof(struct sockaddr_in))
				return -EINVAL;
			break;
		case AF_INET6:
			if (addrlen < sizeof(struct sockaddr_in6))
				return -EINVAL;
			break;
		case AF_UNIX:
			if (addrlen < sizeof(struct sockaddr_un))
				return -EINVAL;
			break;
		default:
			return -EAFNOSUPPORT;
	}
	return 0;
}

ssize_t sys_exit(int code)
{
	struct thread *oldthread = curcpu()->thread;
	thread_exit(oldthread, (code & 0xFF) << 8);
	return arch_get_syscall_retval(&curcpu()->thread->tf_user);
}

ssize_t sys_clone(int flags)
{
	struct thread *thread = curcpu()->thread;
	struct thread *newthread;
	ssize_t ret;

	if (flags & ~(CLONE_VFORK | CLONE_VM | CLONE_THREAD))
		return -EINVAL;
	if (flags & CLONE_THREAD)
	{
		if (!(flags & CLONE_VM))
			return -EINVAL;
		if (flags & CLONE_VFORK)
			return -EINVAL;
		ret = uthread_clone(thread, flags, &newthread);
	}
	else
	{
		ret = uproc_clone(thread, flags, &newthread);
	}
	if (ret < 0)
		return ret;
	arch_set_syscall_retval(&newthread->tf_user, 0);
	assert(newthread->state == THREAD_PAUSED, "new thread isn't in paused state");
	if (flags & CLONE_VFORK)
	{
		struct proc *newp = newthread->proc;
		struct proc *proc = thread->proc;
		newp->vfork_rel = proc;
		proc->vfork_rel = newp;
		sched_run(newthread);
		spinlock_lock(&proc->vfork_waitq_lock);
		if (proc->vfork_rel)
			waitq_wait_head(&proc->vfork_waitq, &proc->vfork_waitq_lock, NULL);
		spinlock_unlock(&proc->vfork_waitq_lock);
	}
	else
	{
		sched_run(newthread);
	}
	return newthread->tid;
}

ssize_t sys_readv(int fd, const struct iovec *uiov, int iovcnt)
{
	struct thread *thread = curcpu()->thread;
	struct iovec iov[IOV_MAX];
	struct file *file;
	struct uio uio;
	ssize_t count;
	ssize_t ret;

	if (iovcnt < 0 || iovcnt > IOV_MAX)
		return -EINVAL;
	ret = vm_copyin(thread->proc->vm_space, iov, uiov, sizeof(*iov) * iovcnt);
	if (ret < 0)
		return ret;
	count = 0;
	for (int i = 0; i < iovcnt; ++i)
	{
		if (__builtin_add_overflow(count, iov[i].iov_len, &count))
			return -EINVAL;
	}
	ret = proc_getfile(thread->proc, fd, &file);
	if (ret < 0)
		return ret;
	switch (file->flags & 3)
	{
		case O_RDONLY:
		case O_RDWR:
			break;
		default:
			return -EBADF;
	}
	if (file->node)
	{
		ret = update_node_times(file->node, FS_ATTR_ATIME);
		if (ret && ret != -EROFS)
		{
			file_free(file);
			return ret;
		}
	}
	uio.iov = iov;
	uio.iovcnt = iovcnt;
	uio.count = count;
	uio.off = file->off;
	uio.userbuf = 1;
	ret = file_read(file, &uio);
	if (ret >= 0)
		file->off = uio.off;
	file_free(file);
	return ret;
}

ssize_t sys_writev(int fd, const struct iovec *uiov, int iovcnt)
{
	struct thread *thread = curcpu()->thread;
	struct iovec iov[IOV_MAX];
	struct file *file;
	struct uio uio;
	ssize_t count;
	ssize_t ret;

	if (iovcnt < 0 || iovcnt > IOV_MAX)
		return -EINVAL;
	ret = vm_copyin(thread->proc->vm_space, iov, uiov, sizeof(*iov) * iovcnt);
	if (ret < 0)
		return ret;
	count = 0;
	for (int i = 0; i < iovcnt; ++i)
	{
		if (__builtin_add_overflow(count, iov[i].iov_len, &count))
			return -EINVAL;
	}
	ret = proc_getfile(thread->proc, fd, &file);
	if (ret < 0)
		return ret;
	switch (file->flags & 3)
	{
		case O_WRONLY:
		case O_RDWR:
			break;
		default:
			return -EBADF;
	}
	if (file->node)
	{
		if (is_node_rofs(file->node))
		{
			file_free(file);
			return -EROFS;
		}
		ret = update_node_times(file->node, FS_ATTR_ATIME
		                                  | FS_ATTR_CTIME
		                                  | FS_ATTR_MTIME);
		if (ret < 0 && ret != -EROFS)
		{
			file_free(file);
			return ret;
		}
	}
	uio.iov = iov;
	uio.iovcnt = iovcnt;
	uio.count = count;
	uio.off = file->off;
	uio.userbuf = 1;
	ret = file_write(file, &uio);
	if (ret >= 0)
		file->off = uio.off;
	file_free(file);
	return ret;
}

ssize_t sys_openat(int dirfd, const char *upathname, int flags,
                   mode_t mode)
{
	struct thread *thread = curcpu()->thread;
	char pathname[MAXPATHLEN];
	struct node *cwd;
	struct node *node;
	int ret;

	mode &= 07777;
	if ((flags & (O_DIRECTORY | O_WRONLY)) == (O_DIRECTORY | O_WRONLY))
		return -EINVAL;
	if (flags & ~(O_RDONLY | O_WRONLY | O_RDWR | O_APPEND | O_ASYNC | O_CREAT
	            | O_TRUNC | O_NOFOLLOW | O_DIRECTORY | O_EXCL | O_CLOEXEC
	            | O_NOCTTY))
		return -EINVAL;
	if ((flags & O_TRUNC) && !(flags & (O_WRONLY | O_RDWR)))
		return -EINVAL;
	/* test only one of O_RDONLY | O_WRONLY | O_RDWR */
	if ((flags & 3) == 3)
		return -EINVAL;
	/* XXX handle O_NOCTTY */
	ret = vm_copystr(thread->proc->vm_space, pathname, upathname,
	                 sizeof(pathname));
	if (ret < 0)
		return ret;
	ret = getcwdat(thread, dirfd, pathname, 0, &cwd);
	if (ret < 0)
		return ret;
	ret = vfs_getnode(cwd, pathname,
	                  (flags & O_NOFOLLOW) ? VFS_NOFOLLOW : 0,
	                  &node);
	if (ret < 0)
	{
		if (ret != -ENOENT || !(flags & O_CREAT))
		{
			node_free(cwd);
			return ret;
		}
		char *end_fn;
		ret = vfs_getdir(cwd, pathname, 0, &node, &end_fn);
		node_free(cwd);
		if (ret < 0)
			return ret;
		if (!end_fn)
		{
			node_free(node);
			return -EINVAL;
		}
		struct timespec ts;
		ret = clock_gettime(CLOCK_REALTIME, &ts);
		if (ret < 0)
		{
			node_free(node);
			return ret;
		}
		fs_attr_mask_t mask = FS_ATTR_UID | FS_ATTR_GID | FS_ATTR_MODE
		                    | FS_ATTR_CTIME | FS_ATTR_ATIME | FS_ATTR_MTIME;
		struct fs_attr attr;
		attr.mode = mode & ~thread->proc->umask;
		attr.uid = thread->proc->cred.euid;
		attr.gid = thread->proc->cred.egid;
		attr.atime = ts;
		attr.ctime = ts;
		attr.mtime = ts;
		size_t end_fn_len = strlen(end_fn);
		ret = node_mknode(node, end_fn, end_fn_len, mask, &attr, 0);
		if (ret < 0)
		{
			node_free(node);
			return ret;
		}
		struct node *child;
		ret = vfs_getnode(node, end_fn, 0, &child);
		node_free(node);
		if (ret < 0)
			return ret;
		node = child;
	}
	else
	{
		node_free(cwd);
		if ((flags & (O_EXCL | O_CREAT)) == (O_EXCL | O_CREAT))
		{
			node_free(node);
			return -EEXIST;
		}
		ret = update_node_times(node, FS_ATTR_ATIME);
		if (ret < 0 && ret != -EROFS)
		{
			node_free(node);
			return ret;
		}
	}
	if ((flags & O_DIRECTORY) && !(S_ISDIR(node->attr.mode)))
	{
		node_free(node);
		return -ENOTDIR;
	}
	mode_t perm = 0;
	switch (flags & 3)
	{
		case O_RDONLY:
			perm = R_OK;
			break;
		case O_WRONLY:
			perm = W_OK;
			break;
		case O_RDWR:
			perm = R_OK | W_OK;
			break;
		default:
			panic("invalid mode\n");
			break;
	}
	if (thread->proc->cred.euid)
	{
		if (!(vfs_getperm(node, thread->proc->cred.euid, thread->proc->cred.egid) & perm))
		{
			node_free(node);
			return -EACCES;
		}
	}
	struct file *file;
	ret = file_fromnode(node, flags, &file);
	node_free(node);
	if (ret < 0)
		return ret;
	ret = file_open(file, node);
	if (ret < 0)
	{
		file_free(file);
		return ret;
	}
	ret = proc_allocfd(thread->proc, file,
	                   (flags & O_CLOEXEC) ? FD_CLOEXEC : 0);
	file_free(file);
	return ret;
}

ssize_t sys_close(int fd)
{
	return proc_freefd(curcpu()->thread->proc, fd);
}

ssize_t sys_time(time_t *tloc)
{
	struct thread *thread = curcpu()->thread;
	struct timespec ts;
	ssize_t ret;

	ret = clock_gettime(CLOCK_REALTIME, &ts);
	if (ret < 0)
		return ret;
	if (tloc)
	{
		time_t r = ts.tv_sec;
		ret = vm_copyout(thread->proc->vm_space, tloc, &r, sizeof(r));
		if (ret < 0)
			return ret;
	}
	return 0;
}

ssize_t sys_getpid()
{
	return curcpu()->thread->proc->pid;
}

ssize_t sys_getuid()
{
	return curcpu()->thread->proc->cred.uid;
}

ssize_t sys_getgid()
{
	return curcpu()->thread->proc->cred.gid;
}

ssize_t sys_setuid(uid_t uid)
{
	struct thread *thread = curcpu()->thread;
	struct cred *cred = &thread->proc->cred;

	if (!cred->uid || !cred->euid)
	{
		cred->uid = uid;
		cred->euid = uid;
		cred->suid = uid;
		return 0;
	}
	if (uid != cred->uid
	 && uid != cred->euid
	 && uid != cred->suid)
		return -EPERM;
	cred->uid = uid;
	return 0;
}

ssize_t sys_setgid(gid_t gid)
{
	struct thread *thread = curcpu()->thread;
	struct cred *cred = &thread->proc->cred;

	if (!cred->uid || !cred->euid)
	{
		cred->gid = gid;
		cred->egid = gid;
		cred->sgid = gid;
		return 0;
	}
	if (gid != cred->gid
	 && gid != cred->egid
	 && gid != cred->sgid)
		return -EPERM;
	cred->gid = gid;
	return 0;
}

ssize_t sys_geteuid()
{
	return curcpu()->thread->proc->cred.euid;
}

ssize_t sys_getegid()
{
	return curcpu()->thread->proc->cred.egid;
}

ssize_t sys_setpgid(pid_t pid, pid_t pgid)
{
	struct proc *proc;
	struct pgrp *pgrp;

	if (pgid < 0)
		return -EINVAL;
	if (!pid)
	{
		proc = curcpu()->thread->proc;
	}
	else
	{
		TAILQ_FOREACH(proc, &curcpu()->thread->proc->childs, child_chain)
		{
			if (proc->pid != pid)
				continue;
			break;
		}
		if (!proc)
			return -ESRCH;
	}
	if (!pgid)
		pgid = proc->pid;
	pgrp = getpgrp(pgid);
	if (pgrp)
	{
		if (pgrp->sess != proc->pgrp->sess)
		{
			pgrp_free(pgrp);
			return -EPERM;
		}
		proc_setpgrp(proc, pgrp);
		pgrp_free(pgrp);
		return 0;
	}
	pgrp = pgrp_alloc(pgid, proc->pgrp->sess);
	if (!pgrp)
		return -ENOMEM;
	proc_setpgrp(proc, pgrp);
	pgrp_free(pgrp);
	return 0;
}

ssize_t sys_getppid()
{
	struct proc *parent = curcpu()->thread->proc->parent;
	if (parent)
		return parent->pid;
	return 0;
}

ssize_t sys_getpgrp()
{
	return curcpu()->thread->proc->pgrp->id;
}

ssize_t sys_setsid()
{
	struct proc *proc = curcpu()->thread->proc;
	struct sess *sess;
	struct pgrp *pgrp = getpgrp(proc->pid);
	ssize_t ret;

	if (pgrp)
	{
		pgrp_free(pgrp);
		return -EPERM;
	}
	sess = sess_alloc(proc->pid);
	if (!sess)
		return -ENOMEM;
	pgrp = pgrp_alloc(proc->pid, sess);
	sess_free(sess);
	if (!pgrp)
		return -ENOMEM;
	ret = pgrp->id;
	proc_setpgrp(proc, pgrp);
	pgrp_free(pgrp);
	return ret;
}

ssize_t sys_setreuid(uid_t ruid, uid_t euid)
{
	struct thread *thread = curcpu()->thread;
	struct cred *cred = &thread->proc->cred;

	if (cred->uid)
	{
		if (ruid != (uid_t)-1
		 && ruid != cred->uid
		 && ruid != cred->euid)
			return -EPERM;
		if (euid != (uid_t)-1
		 && euid != cred->uid
		 && euid != cred->euid
		 && euid != cred->suid)
			return -EPERM;
	}
	if (ruid != (uid_t)-1
	 || euid != cred->uid)
		cred->suid = euid;
	if (ruid != (uid_t)-1)
		cred->uid = ruid;
	if (euid != (uid_t)-1)
		cred->euid = euid;
	return 0;
}

ssize_t sys_setregid(gid_t rgid, gid_t egid)
{
	struct thread *thread = curcpu()->thread;
	struct cred *cred = &thread->proc->cred;

	if (cred->uid)
	{
		if (rgid != (gid_t)-1
		 && rgid != cred->gid
		 && rgid != cred->egid)
			return -EPERM;
		if (egid != (gid_t)-1
		 && egid != cred->gid
		 && egid != cred->egid
		 && egid != cred->sgid)
			return -EPERM;
	}
	if (rgid != (gid_t)-1
	 || egid != cred->gid)
		cred->sgid = egid;
	if (rgid != (gid_t)-1)
		cred->gid = rgid;
	if (egid != (gid_t)-1)
		cred->egid = egid;
	return 0;
}

size_t sys_getgroups(int size, gid_t *ulist)
{
	if (size < 0)
		return -EINVAL;
	struct thread *thread = curcpu()->thread;
	if (!size)
		return thread->proc->cred.groups_nb;
	if ((size_t)size < thread->proc->cred.groups_nb)
		return -EINVAL;
	ssize_t ret = vm_copyout(thread->proc->vm_space, ulist,
	                         thread->proc->cred.groups,
	                         thread->proc->cred.groups_nb * sizeof(*ulist));
	if (ret < 0)
		return ret;
	return thread->proc->cred.groups_nb;
}

ssize_t sys_setgroups(size_t size, const gid_t *ulist)
{
	if (size > 65535)
		return -EINVAL;
	struct thread *thread = curcpu()->thread;
	gid_t *list;
	if (size)
	{
		list = malloc(size * sizeof(*list), 0);
		if (!list)
			return -ENOMEM;
		ssize_t ret = vm_copyin(thread->proc->vm_space, list, ulist,
		                        size * sizeof(*list));
		if (ret < 0)
		{
			free(list);
			return ret;
		}
	}
	else
	{
		list = NULL;
	}
	free(thread->proc->cred.groups);
	thread->proc->cred.groups = list;
	thread->proc->cred.groups_nb = size;
	return 0;
}

ssize_t sys_getpgid()
{
	return curcpu()->thread->proc->pgrp->id;
}

ssize_t sys_ioctl(int fd, unsigned long req, uintptr_t data)
{
	struct thread *thread = curcpu()->thread;
	struct file *file;
	ssize_t ret;

	ret = proc_getfile(thread->proc, fd, &file);
	if (ret < 0)
		return ret;
	ret = file_ioctl(file, req, data);
	file_free(file);
	return ret;
}

ssize_t sys_fstatat(int dirfd, const char *upathname,
                    struct stat *ustatbuf, int flags)
{
	struct thread *thread = curcpu()->thread;
	char pathname[MAXPATHLEN];
	struct node *node;
	struct stat statbuf;
	ssize_t ret;

	if (flags & ~(AT_SYMLINK_NOFOLLOW | AT_EMPTY_PATH))
		return -EINVAL;
	ret = vm_copystr(thread->proc->vm_space, pathname, upathname,
	                 sizeof(pathname));
	if (ret < 0)
		return ret;
	ret = getnodeat(thread, dirfd, pathname, flags, &node);
	if (ret < 0)
		return ret;
	statbuf.st_dev = node->sb ? node->sb->dev : 0;
	statbuf.st_ino = node->ino;
	statbuf.st_mode = node->attr.mode;
	statbuf.st_nlink = node->nlink;
	statbuf.st_uid = node->attr.uid;
	statbuf.st_gid = node->attr.gid;
	statbuf.st_rdev = node->rdev;
	statbuf.st_size = node->attr.size;
	statbuf.st_blksize = node->blksize;
	statbuf.st_blocks = node->blocks;
	statbuf.st_atim = node->attr.atime;
	statbuf.st_mtim = node->attr.mtime;
	statbuf.st_ctim = node->attr.ctime;
	node_free(node);
	return vm_copyout(thread->proc->vm_space, ustatbuf, &statbuf,
	                  sizeof(statbuf));
}

struct getdents_ctx
{
	struct thread *thread;
	struct fs_readdir_ctx readdir_ctx;
	int res;
	struct sys_dirent *dirp;
	size_t count;
	size_t off;
};

static int getdents_fn(struct fs_readdir_ctx *ctx, const char *name,
                       uint32_t namelen, off_t off, ino_t ino, uint32_t type)
{
	/* XXX bufferize */
	(void)off;
	struct getdents_ctx *getdents_ctx = ctx->userdata;
	uint32_t entry_size = offsetof(struct sys_dirent, name) + namelen;
	struct sys_dirent dirent;
	ssize_t ret;

	if (entry_size > getdents_ctx->count)
		return -EINVAL;
	dirent.ino = ino;
	dirent.off = getdents_ctx->off;
	dirent.reclen = entry_size;
	dirent.type = type;
	ret = vm_copyout(getdents_ctx->thread->proc->vm_space, getdents_ctx->dirp,
	                 &dirent, offsetof(struct sys_dirent, name));
	if (ret < 0)
	{
		getdents_ctx->res = ret;
		return ret;
	}
	ret = vm_copyout(getdents_ctx->thread->proc->vm_space,
	                 (uint8_t*)getdents_ctx->dirp + offsetof(struct sys_dirent, name), name,
	                 namelen);
	if (ret < 0)
	{
		getdents_ctx->res = ret;
		return ret;
	}
	getdents_ctx->count -= entry_size;
	getdents_ctx->dirp = (struct sys_dirent*)((char*)getdents_ctx->dirp
	                                         + entry_size);
	getdents_ctx->off += entry_size;
	return 0;
}

ssize_t sys_getdents(int fd, struct sys_dirent *dirp, size_t count)
{
	struct thread *thread = curcpu()->thread;
	struct file *file;
	struct node *node;
	struct getdents_ctx getdents_ctx;
	struct fs_readdir_ctx readdir_ctx;
	ssize_t ret;

	ret = proc_getfile(thread->proc, fd, &file);
	if (ret < 0)
		return ret;
	node = file->node;
	if (!node)
	{
		file_free(file);
		return -EINVAL;
	}
	if (!S_ISDIR(node->attr.mode))
	{
		file_free(file);
		return -ENOTDIR;
	}
	readdir_ctx.fn = getdents_fn;
	readdir_ctx.off = file->off;
	readdir_ctx.userdata = &getdents_ctx;
	getdents_ctx.res = 0;
	getdents_ctx.dirp = dirp;
	getdents_ctx.count = count;
	getdents_ctx.off = 0;
	getdents_ctx.thread = thread;
	ret = node_readdir(node, &readdir_ctx);
	file->off = readdir_ctx.off;
	file_free(file);
	if (ret < 0)
		return ret;
	if (getdents_ctx.res)
		return getdents_ctx.res;
	return getdents_ctx.off;
}

ssize_t sys_execveat(int dirfd, const char *upathname,
                     const char * const *uargv,
                     const char * const *uenvp,
                     int flags)
{
	struct thread *thread = curcpu()->thread;
	char pathname[MAXPATHLEN];
	struct file *file;
	struct node *node;
	ssize_t ret;

	if (flags & ~(AT_SYMLINK_NOFOLLOW | AT_EMPTY_PATH))
		return -EINVAL;
	ret = vm_copystr(thread->proc->vm_space, pathname, upathname,
	                 sizeof(pathname));
	if (ret < 0)
		return ret;
	ret = vm_verifystra(thread->proc->vm_space, uargv);
	if (ret < 0)
		return ret;
	ret = vm_verifystra(thread->proc->vm_space, uenvp);
	if (ret < 0)
		return ret;
	ret = getnodeat(thread, dirfd, pathname, flags, &node);
	if (ret < 0)
		return ret;
	if (!(vfs_getperm(node, thread->proc->cred.euid, thread->proc->cred.egid) & X_OK))
	{
		node_free(node);
		return -EACCES;
	}
	if (node->sb && (node->sb->flags & ST_NOEXEC))
	{
		node_free(node);
		return -EACCES;
	}
	if (!S_ISREG(node->attr.mode))
	{
		node_free(node);
		return -EACCES;
	}
	ret = file_fromnode(node, O_RDONLY, &file);
	node_free(node);
	if (ret < 0)
		return ret;
	ret = file_open(file, node);
	if (ret < 0)
	{
		file_free(file);
		return ret;
	}
	ret = uproc_execve(thread, file, pathname[0] ? pathname : NULL, uargv, uenvp);
	file_free(file);
	if (ret < 0)
		return ret;
	if (thread->proc->vfork_rel)
		proc_wakeup_vfork(thread->proc->vfork_rel);
	return arch_get_syscall_retval(&thread->tf_user);
}

ssize_t sys_lseek(int fd, off_t *uoff, int whence)
{
	struct thread *thread = curcpu()->thread;
	struct file *file;
	ssize_t ret;
	off_t off;

	ret = proc_getfile(thread->proc, fd, &file);
	if (ret)
		return ret;
	ret = vm_copyin(thread->proc->vm_space, &off, uoff, sizeof(off));
	if (ret < 0)
	{
		file_free(file);
		return ret;
	}
	ret = file_seek(file, off, whence);
	file_free(file);
	if (ret < 0)
		return ret;
	return vm_copyout(thread->proc->vm_space, uoff, &ret, sizeof(ret));
}

void *sys_mmap(void *uaddr, size_t len, int prot, int flags, int fd, off_t *uoff)
{
	struct thread *thread = curcpu()->thread;
	struct vm_space *vm_space = thread->proc->vm_space;
	struct vm_zone *zone;
	uintptr_t addr;
	size_t alignment;
	off_t off;
	int kprot = 0;
	struct file *file = NULL;
	int ret;

	if (!len)
		return (void*)-EINVAL;
	if (flags & ~(MAP_ANONYMOUS | MAP_SHARED | MAP_PRIVATE | MAP_FIXED | MAP_EXCL | MAP_ALIGNED(63)))
		return (void*)-EINVAL;
	if (!(flags & (MAP_SHARED | MAP_PRIVATE)))
		return (void*)-EINVAL;
	if ((flags & (MAP_SHARED | MAP_PRIVATE)) == (MAP_SHARED | MAP_PRIVATE))
		return (void*)-EINVAL;
	if (prot & ~(PROT_READ | PROT_WRITE | PROT_EXEC))
		return (void*)-EINVAL;
	if ((prot & (PROT_WRITE | PROT_EXEC)) == (PROT_WRITE | PROT_EXEC))
		return (void*)-EINVAL;
	if ((flags & MAP_FIXED) && (!uaddr || ((uintptr_t)uaddr & PAGE_MASK)))
		return (void*)-EINVAL;
	if (flags & MAP_SHARED) /* XXX */
		return (void*)-EINVAL;
	alignment = (flags >> MAP_ALIGNED_SHIFT) & 63;
	if (alignment >= sizeof(size_t) * 8 - 1)
		return (void*)-EINVAL;
	alignment = (size_t)1 << alignment;
	if (alignment < PAGE_SIZE)
		alignment = 0;
	else if ((uintptr_t)uaddr & (alignment - 1))
		return (void*)-EINVAL;
	if (uoff)
	{
		ret = vm_copyin(thread->proc->vm_space, &off, uoff, sizeof(off));
		if (ret < 0)
			return (void*)(intptr_t)ret;
		if (off & PAGE_MASK)
			return (void*)-EINVAL;
	}
	else
	{
		off = 0;
	}
	if ((flags & MAP_ANONYMOUS) && (off != 0 || fd != -1))
		return (void*)-EINVAL;
	len += (uintptr_t)uaddr & PAGE_MASK;
	uaddr = (void*)((uintptr_t)uaddr & ~PAGE_MASK);
	if (prot & PROT_READ)
		kprot |= VM_PROT_R;
	if (prot & PROT_WRITE)
		kprot |= VM_PROT_W;
	if (prot & PROT_EXEC)
		kprot |= VM_PROT_X;
	if (!(flags & MAP_ANONYMOUS))
	{
		ret = proc_getfile(thread->proc, fd, &file);
		if (ret)
			return (void*)(intptr_t)ret;
	}
	/* XXX really ? */
	if (len & PAGE_MASK)
		len += PAGE_SIZE - (len & PAGE_MASK);
	mutex_lock(&vm_space->mutex);
	ret = vm_alloc(vm_space, (uintptr_t)uaddr, off, len, alignment, kprot,
	               flags, file, &zone);
	if (ret < 0)
	{
		if ((flags & (MAP_FIXED | MAP_EXCL)) == MAP_FIXED)
		{
			ret = vm_free(vm_space, (uintptr_t)uaddr, len);
			if (ret < 0)
			{
				addr = ret;
				goto end;
			}
			ret = vm_alloc(vm_space, (uintptr_t)uaddr, off,
			               len, alignment, kprot, flags, file,
			               &zone);
			if (ret < 0)
			{
				addr = ret;
				goto end;
			}
		}
		else if (!uaddr)
		{
			addr = -EINVAL;
			goto end;
		}
		else
		{
			ret = vm_alloc(vm_space, 0, off, len, alignment, kprot,
			               flags, file, &zone);
			if (ret < 0)
			{
				addr = ret;
				goto end;
			}
		}
	}
	if (!(flags & MAP_ANONYMOUS))
	{
		ret = file_mmap(file, zone);
		if (ret < 0)
		{
			vm_free(vm_space, zone->addr, zone->size);
			addr = ret;
			goto end;
		}
	}
	if (flags & MAP_POPULATE)
	{
		ret = vm_populate(vm_space, zone->addr, zone->size);
		if (ret < 0)
		{
			vm_free(vm_space, zone->addr, zone->size);
			addr = ret;
			goto end;
		}
	}
	addr = zone->addr;

end:
	if (file)
		file_free(file);
	mutex_unlock(&vm_space->mutex);
	return (void*)addr;
}

ssize_t sys_munmap(void *addr, size_t len)
{
	struct vm_space *vm_space = curcpu()->thread->proc->vm_space;
	mutex_lock(&vm_space->mutex);
	int ret = vm_free(vm_space, (uintptr_t)addr, len);
	mutex_unlock(&vm_space->mutex);
	return ret;
}

ssize_t sys_readlinkat(int dirfd, const char *upathname, char *buf,
                       size_t bufsize)
{
	struct thread *thread = curcpu()->thread;
	char pathname[MAXPATHLEN];
	struct node *node;
	struct iovec iov;
	struct uio uio;
	ssize_t ret;

	ret = vm_copystr(thread->proc->vm_space, pathname, upathname,
	                 sizeof(pathname));
	if (ret < 0)
		return ret;
	ret = getnodeat(thread, dirfd, pathname, AT_SYMLINK_NOFOLLOW, &node);
	if (ret < 0)
		return ret;
	if (!S_ISLNK(node->attr.mode))
	{
		node_free(node);
		return -EINVAL;
	}
	iov.iov_base = buf;
	iov.iov_len = bufsize;
	uio.iov = &iov;
	uio.iovcnt = 1;
	uio.count = bufsize;
	uio.off = 0;
	uio.userbuf = 1;
	ret = node_readlink(node, &uio);
	node_free(node);
	return ret;
}

ssize_t sys_dup(int oldfd)
{
	struct thread *thread = curcpu()->thread;
	struct file *file;
	ssize_t ret;

	ret = proc_getfile(thread->proc, oldfd, &file);
	if (ret < 0)
		return ret;
	ret = proc_allocfd(thread->proc, file, 0);
	file_free(file);
	return ret;
}

ssize_t sys_dup3(int oldfd, int newfd, int flags)
{
	struct thread *thread = curcpu()->thread;
	struct file *file;
	ssize_t ret;

	if (flags & ~(O_CLOEXEC))
		return -EINVAL;
	if (newfd < 0)
		return -EBADF;
	ret = proc_getfile(thread->proc, oldfd, &file);
	if (ret < 0)
		return ret;
	if (oldfd == newfd)
	{
		/* XXX should return EINVAL if dup3 */
		file_free(file);
		return newfd;
	}
	if ((unsigned)newfd < thread->proc->files_nb)
	{
		rwlock_wrlock(&thread->proc->files_lock);
		struct filedesc *prv = &thread->proc->files[newfd];
		if (prv->file)
			file_free(prv->file);
		thread->proc->files[newfd].file = file;
		thread->proc->files[newfd].cloexec = flags;
		rwlock_unlock(&thread->proc->files_lock);
		return newfd;
	}
	/* XXX set a maximum bound (ulimit pls) */
	rwlock_wrlock(&thread->proc->files_lock);
	struct filedesc *fd = realloc(thread->proc->files,
	                              sizeof(*thread->proc->files) * (newfd + 1),
	                              M_ZERO);
	if (!fd)
	{
		rwlock_unlock(&thread->proc->files_lock);
		file_free(file);
		return -ENOMEM;
	}
	thread->proc->files = fd;
	thread->proc->files_nb = newfd + 1;
	thread->proc->files[newfd].file = file;
	thread->proc->files[newfd].cloexec = flags;
	rwlock_unlock(&thread->proc->files_lock);
	return newfd;
}

static int wait_predicate(struct thread *child, int *wstatus,
                          int options)
{
	if (!child->waitable)
		return 0;
	if (child->proc->state == PROC_STOPPED)
	{
		if (!(options & WUNTRACED))
			return 0;
	}
	*wstatus = child->wstatus;
	child->waitable = 0;
	return 1;
}

static int wait_proc(struct proc *proc, struct thread *child,
                     int *wstatus, int options)
{
	spinlock_lock(&proc->wait_waitq_lock);
	while (1)
	{
		if (wait_predicate(child, wstatus, options))
			break;
		if (options & WNOHANG)
			break;
		int ret = waitq_wait_tail(&proc->wait_waitq,
		                          &proc->wait_waitq_lock,
		                          NULL);
		if (ret)
		{
			spinlock_unlock(&proc->wait_waitq_lock);
			return ret;
		}
	}
	spinlock_unlock(&proc->wait_waitq_lock);
	return 0;
}

static int wait_pgrp(struct proc *proc, struct pgrp *pgrp, int *wstatus,
                    int options, struct thread **child_thread)
{
	struct proc *child;
	spinlock_lock(&proc->wait_waitq_lock);
	while (1)
	{
		pgrp_lock(pgrp);
		TAILQ_FOREACH(child, &pgrp->processes, child_chain)
		{
			TAILQ_FOREACH(*child_thread, &child->threads, thread_chain)
			{
				if (wait_predicate(*child_thread, wstatus, options))
					break;
			}
			if (child_thread)
				break;
		}
		pgrp_unlock(pgrp);
		if (*child_thread)
			break;
		if (options & WNOHANG)
			break;
		int ret = waitq_wait_tail(&proc->wait_waitq,
		                          &proc->wait_waitq_lock,
		                          NULL);
		if (ret)
		{
			spinlock_unlock(&proc->wait_waitq_lock);
			return ret;
		}
	}
	spinlock_unlock(&proc->wait_waitq_lock);
	return 0;
}

ssize_t sys_wait4(pid_t pid, int *uwstatus, int options,
                  struct rusage *urusage)
{
	struct thread *thread = curcpu()->thread;
	ssize_t ret;
	struct thread *child_thread;
	int wstatus = 0;

	if (options & ~(WNOHANG | WUNTRACED | WCONTINUED))
		return -EINVAL;
	if (options & WCONTINUED) /* XXX support it */
		return -EINVAL;
	if (pid > 0)
	{
		struct proc *child = proc_getchild(thread->proc, pid);
		if (child != NULL)
		{
			child_thread = TAILQ_FIRST(&child->threads);
		}
		else
		{
			TAILQ_FOREACH(child_thread, &thread->proc->threads, thread_chain)
			{
				if (child_thread->tid == pid)
					break;
			}
			if (!child_thread)
				return -ECHILD;
			child = thread->proc;
		}
		ret = wait_proc(thread->proc, child_thread, &wstatus, options);
		if (ret < 0)
			return ret;
		goto end;
	}
	if (options & WUNTRACED) /* XXX support it */
		return -EINVAL;
	if (pid == -1)
	{
		struct proc *child;
		spinlock_lock(&thread->proc->wait_waitq_lock);
		while (1)
		{
			TAILQ_FOREACH(child, &thread->proc->childs, child_chain)
			{
				TAILQ_FOREACH(child_thread, &child->threads, thread_chain)
				{
					if (wait_predicate(child_thread, &wstatus, options))
						break;
				}
				if (child_thread)
					break;
			}
			if (child)
				break;
			TAILQ_FOREACH(child_thread, &thread->proc->threads, thread_chain)
			{
				if (wait_predicate(child_thread, &wstatus, options))
					break;
			}
			if (child_thread)
			{
				child = thread->proc;
				break;
			}
			if (options & WNOHANG)
				break;
			ret = waitq_wait_tail(&thread->proc->wait_waitq,
			                      &thread->proc->wait_waitq_lock,
			                      NULL);
			if (ret < 0)
			{
				spinlock_unlock(&thread->proc->wait_waitq_lock);
				return ret;
			}
		}
		spinlock_unlock(&thread->proc->wait_waitq_lock);
		goto end;
	}
	if (!pid)
	{
		ret = wait_pgrp(thread->proc, thread->proc->pgrp, &wstatus,
		                options, &child_thread);
		if (ret < 0)
			return ret;
	}
	if (pid < -1)
	{
		struct pgrp *pgrp = getpgrp(-pid);
		if (!pgrp)
			return -ECHILD;
		ret = wait_pgrp(thread->proc, pgrp, &wstatus, options,
		                &child_thread);
		pgrp_free(pgrp);
		if (ret < 0)
			return ret;
	}

end:
	if (!child_thread)
		return 0;
	pid = child_thread->tid;
	if (child_thread->proc->state == PROC_ZOMBIE)
	{
		struct procstat *dst = &child_thread->proc->parent->cstats;
		struct procstat *src = &child_thread->proc->stats;
		struct procstat *csrc = &child_thread->proc->cstats;
		timespec_add(&dst->utime, &dst->utime, &src->utime);
		timespec_add(&dst->utime, &dst->utime, &csrc->utime);
		timespec_add(&dst->stime, &dst->stime, &src->stime);
		timespec_add(&dst->stime, &dst->stime, &csrc->stime);
		dst->faults += src->faults;
		dst->faults += csrc->faults;
		dst->msgsnd += src->msgsnd;
		dst->msgsnd += csrc->msgsnd;
		dst->msgrcv += src->msgrcv;
		dst->msgrcv += csrc->msgrcv;
		dst->nsignals += src->nsignals;
		dst->nsignals += csrc->nsignals;
		dst->nctxsw += src->nctxsw;
		dst->nctxsw += csrc->nctxsw;
		proc_free(child_thread->proc);
		if (urusage)
		{
			struct rusage rusage;
			rusage_from_stats(&rusage, src);
			ret = vm_copyout(thread->proc->vm_space, urusage, &rusage,
			                 sizeof(rusage));
			if (ret)
				return ret;
		}
	}
	else if (child_thread->state == THREAD_ZOMBIE)
	{
		if (urusage)
		{
			struct rusage rusage;
			rusage_from_stats(&rusage, &thread->stats);
			ret = vm_copyout(thread->proc->vm_space, urusage, &rusage,
			                 sizeof(rusage));
			if (ret < 0)
				return ret;
		}
		thread_free(child_thread);
	}
	if (uwstatus)
	{
		ret = vm_copyout(thread->proc->vm_space, uwstatus,
		                 &wstatus, sizeof(int));
		if (ret < 0)
			return ret;
	}
	return pid;
}

static int send_signal(struct proc *self, struct proc *proc, int sig)
{
	if (self->cred.uid
	 && proc->cred.uid != self->cred.uid
	 && proc->cred.suid != self->cred.uid
	 && proc->cred.uid != self->cred.euid
	 && proc->cred.suid != self->cred.euid)
	{
		if (sig != SIGCONT || proc->pgrp->sess != self->pgrp->sess)
			return -EPERM;
	}
	if (sig)
		proc_signal(proc, sig);
	return 0;
}

ssize_t sys_kill(pid_t pid, int sig)
{
	if (sig < 0 || sig > SIGLAST)
		return -EINVAL;
	struct proc *self = curcpu()->thread->proc;
	if (pid > 0)
	{
		struct proc *proc = getproc(pid);
		if (!proc)
			return -ESRCH;
		return send_signal(self, proc, sig);
	}
	if (pid == 0)
	{
		struct proc *proc;
		pgrp_lock(self->pgrp);
		TAILQ_FOREACH(proc, &self->pgrp->processes, pgrp_chain)
			send_signal(self, proc, sig);
		pgrp_unlock(self->pgrp);
		return 0;
	}
	if (pid == -1)
	{
		struct proc *proc;
		spinlock_lock(&g_proc_list_lock);
		TAILQ_FOREACH(proc, &g_proc_list, chain)
		{
			if (proc->pid == 1 || proc == self)
				continue;
			send_signal(self, proc, sig);
		}
		spinlock_unlock(&g_proc_list_lock);
		return 0;
	}
	if (pid < 0)
	{
		struct pgrp *pgrp = getpgrp(-pid);
		if (!pgrp)
			return -ESRCH;
		pgrp_lock(pgrp);
		struct proc *proc;
		TAILQ_FOREACH(proc, &pgrp->processes, pgrp_chain)
		{
			if (proc->pid == 1 || proc == self)
				continue;
			send_signal(self, proc, sig);
		}
		pgrp_unlock(pgrp);
		pgrp_free(pgrp);
		return 0;
	}
	return -ENOSYS;
}

ssize_t sys_getsid(pid_t pid)
{
	struct proc *proc;

	if (!pid)
		return curcpu()->thread->proc->pgrp->sess->id;
	proc = getproc(pid);
	if (!proc)
		return -ESRCH;
	return proc->pgrp->sess->id;
}

ssize_t sys_socket(int domain, int type, int protocol)
{
	struct thread *thread = curcpu()->thread;
	struct file *file;
	struct sock *sock;
	ssize_t ret;

	ret = sock_open(domain, type, protocol, &sock);
	if (ret < 0)
		return ret;
	ret = file_fromsock(sock, 0, &file);
	sock_free(sock);
	if (ret < 0)
		return ret;
	ret = proc_allocfd(thread->proc, file, 0);
	file_free(file);
	return ret;
}

ssize_t sys_bind(int fd, const struct sockaddr *uaddr, socklen_t addrlen)
{
	struct thread *thread = curcpu()->thread;
	union sockaddr_union addr;
	struct sock *sock = NULL;
	ssize_t ret;

	if (!addrlen || addrlen > sizeof(addr))
		return -EINVAL;
	ret = vm_copyin(thread->proc->vm_space, &addr, uaddr, addrlen);
	if (ret < 0)
		return ret;
	ret = getsock(thread, fd, &sock);
	if (ret < 0)
		return ret;
	if (addr.sa.sa_family != sock->domain)
	{
		ret = -EAFNOSUPPORT;
		goto end;
	}
	ret = test_sockaddr_len(addr.sa.sa_family, addrlen);
	if (ret < 0)
		goto end;
	sock_lock(sock);
	if (sock->dst_addrlen) /* XXX != SOCK_ST_NONE */
	{
		sock_unlock(sock);
		ret = -EISCONN;
		goto end;
	}
	sock_unlock(sock);
	ret = sock_bind(sock, &addr.sa, addrlen);

end:
	sock_free(sock);
	return ret;
}

ssize_t sys_accept(int fd, struct sockaddr *uaddr, socklen_t *uaddrlen)
{
	struct thread *thread = curcpu()->thread;
	struct file *file;
	struct sock *child;
	struct sock *sock;
	socklen_t addrlen;
	ssize_t ret;

	ret = getsock(thread, fd, &sock);
	if (ret < 0)
		return ret;
	ret = sock_accept(sock, &child);
	sock_free(sock);
	if (ret < 0)
		return ret;
	if (child->dst_addrlen)
	{
		ret = vm_copyin(thread->proc->vm_space, &addrlen, uaddrlen,
		                sizeof(addrlen));
		if (ret < 0)
		{
			sock_free(child);
			goto end;
		}
		ret = vm_copyout(thread->proc->vm_space, uaddr,
		                 &child->dst_addr,
		                 addrlen < child->dst_addrlen
		               ? addrlen : child->dst_addrlen);
		if (ret < 0)
		{
			sock_free(child);
			goto end;
		}
		ret = vm_copyout(thread->proc->vm_space, uaddrlen,
		                 &child->dst_addrlen, sizeof(*uaddrlen));
		if (ret < 0)
		{
			sock_free(child);
			goto end;
		}
	}
	else
	{
		addrlen = 0;
		ret = vm_copyout(thread->proc->vm_space, uaddrlen, &addrlen,
		                 sizeof(addrlen));
		if (ret < 0)
		{
			sock_free(child);
			goto end;
		}
	}
	ret = file_fromsock(child, 0, &file);
	sock_free(child);
	if (ret < 0)
		goto end;
	ret = proc_allocfd(thread->proc, file, 0);
	file_free(file);

end:
	return ret;
}

ssize_t sys_connect(int fd, const struct sockaddr *uaddr,
                    socklen_t addrlen)
{
	struct thread *thread = curcpu()->thread;
	union sockaddr_union addr;
	struct sock *sock;
	ssize_t ret;

	if (!addrlen || addrlen > sizeof(addr))
		return -EINVAL;
	ret = vm_copyin(thread->proc->vm_space, &addr, uaddr, addrlen);
	if (ret < 0)
		return ret;
	ret = getsock(thread, fd, &sock);
	if (ret < 0)
		return ret;
	if (addr.sa.sa_family != sock->domain)
	{
		ret = -EAFNOSUPPORT;
		goto end;
	}
	ret = test_sockaddr_len(addr.sa.sa_family, addrlen);
	if (ret < 0)
		goto end;
	sock_lock(sock);
	if (sock->dst_addrlen) /* XXX != SOCK_ST_NONE */
	{
		sock_unlock(sock);
		ret = -EISCONN;
		goto end;
	}
	sock_unlock(sock);
	ret = sock_connect(sock, &addr.sa, addrlen);

end:
	sock_free(sock);
	return ret;
}

ssize_t sys_listen(int fd, int backlog)
{
	struct thread *thread = curcpu()->thread;
	struct sock *sock;
	ssize_t ret;

	ret = getsock(thread, fd, &sock);
	if (ret < 0)
		return ret;
	/* XXX != SOCK_ST_NONE */
	ret = sock_listen(sock, backlog);
	sock_free(sock);
	return ret;
}

ssize_t sys_mprotect(void *addr, size_t len, int prot)
{
	struct thread *thread = curcpu()->thread;

	if ((uintptr_t)addr % PAGE_SIZE)
		return -EINVAL;
	if (len % PAGE_SIZE)
		return -EINVAL;
	if (prot & ~(PROT_READ | PROT_WRITE | PROT_EXEC))
		return -EINVAL;
	if ((prot & (PROT_EXEC | PROT_WRITE)) == (PROT_EXEC | PROT_WRITE))
		return -EINVAL;
	int kprot = 0;
	if (prot & PROT_READ)
		kprot |= VM_PROT_R;
	if (prot & PROT_WRITE)
		kprot |= VM_PROT_W;
	if (prot & PROT_EXEC)
		kprot |= VM_PROT_X;
	return vm_space_protect(thread->proc->vm_space, (uintptr_t)addr, len,
	                        kprot);
}

ssize_t sys_msync(void *addr, size_t length, int flags)
{
	(void)addr;
	(void)length;
	(void)flags;
	/* XXX */
	return -ENOSYS;
}

ssize_t sys_unlinkat(int dirfd, const char *upathname, int flags)
{
	struct thread *thread = curcpu()->thread;
	char pathname[MAXPATHLEN];
	struct node *node;
	char *end_fn;
	ssize_t ret;

	if (flags & ~AT_REMOVEDIR)
		return -EINVAL;
	ret = vm_copystr(thread->proc->vm_space, pathname, upathname,
	                 sizeof(pathname));
	if (ret < 0)
		return ret;
	ret = getdirat(thread, dirfd, pathname, flags | AT_SYMLINK_NOFOLLOW,
	               &node, &end_fn);
	if (ret < 0)
		return ret;
	if (is_node_rofs(node))
	{
		node_free(node);
		return -EROFS;
	}
	if (flags & AT_REMOVEDIR)
		ret = node_rmdir(node, end_fn);
	else
		ret = node_unlink(node, end_fn);
	node_free(node);
	return ret;
}

ssize_t sys_clock_getres(clockid_t clk_id, struct timespec *uts)
{
	struct thread *thread = curcpu()->thread;
	struct timespec ts;
	ssize_t ret;

	ret = clock_getres(clk_id, &ts);
	if (ret < 0)
		return ret;
	return vm_copyout(thread->proc->vm_space, uts, &ts, sizeof(ts));
}

ssize_t sys_clock_gettime(clockid_t clk_id, struct timespec *uts)
{
	struct thread *thread = curcpu()->thread;
	struct timespec ts;
	ssize_t ret;

	ret = clock_gettime(clk_id, &ts);
	if (ret)
		return ret;
	return vm_copyout(thread->proc->vm_space, uts, &ts, sizeof(ts));
}

ssize_t sys_clock_settime(clockid_t clk_id, const struct timespec *ts)
{
	struct thread *thread = curcpu()->thread;
	struct timespec tm;
	ssize_t ret;

	ret = vm_copyin(thread->proc->vm_space, &tm, ts, sizeof(tm));
	if (ret < 0)
		return ret;
	ret = timespec_validate(&tm);
	if (ret < 0)
		return ret;
	(void)clk_id;
	/* XXX */
	return -ENOSYS;
}

ssize_t sys_pipe2(int *ufds, int flags)
{
	struct thread *thread = curcpu()->thread;
	struct file *files[2] = {NULL, NULL};
	struct pipe *pipe;
	struct node *node;
	int fds[2] = {-1, -1};
	ssize_t ret;

	if (flags & ~O_CLOEXEC)
		return -EINVAL;
	ret = fifo_alloc(&pipe, &node, &files[0], &files[1]);
	if (ret < 0)
		return ret;
	ret = proc_allocfd(thread->proc, files[0],
	                   (flags & O_CLOEXEC) ? FD_CLOEXEC : 0);
	if (ret < 0)
		goto end;
	fds[0] = ret;
	ret = proc_allocfd(thread->proc, files[1],
	                   (flags & O_CLOEXEC) ? FD_CLOEXEC : 0);
	if (ret < 0)
		goto end;
	fds[1] = ret;
	ret = vm_copyout(thread->proc->vm_space, ufds, fds, sizeof(fds));

end:
	for (size_t i = 0; i < 2; ++i)
	{
		if (ret && fds[i] > 0)
			proc_freefd(thread->proc, fds[i]);
		if (files[i])
			file_free(files[i]);
	}
	return ret;
}

ssize_t sys_utimensat(int dirfd, const char *upathname,
                      const struct timespec *utimes, int flags)
{
	struct thread *thread = curcpu()->thread;
	char pathname[MAXPATHLEN];
	struct timespec times[2];
	struct node *node;
	struct fs_attr attr;
	fs_attr_mask_t mask = 0;
	ssize_t ret;

	if (flags & ~(AT_SYMLINK_NOFOLLOW | AT_EMPTY_PATH))
		return -EINVAL;
	ret = vm_copystr(thread->proc->vm_space, pathname, upathname,
	                 sizeof(pathname));
	if (ret < 0)
		return ret;
	if (utimes)
	{
		ret = vm_copyin(thread->proc->vm_space, times, utimes, sizeof(times));
		if (ret < 0)
			return ret;
	}
	ret = getnodeat(thread, dirfd, pathname, flags, &node);
	if (ret < 0)
		return ret;
	if (is_node_rofs(node))
	{
		node_free(node);
		return -EROFS;
	}
	if (!utimes || times[0].tv_nsec == UTIME_NOW)
	{
		ret = clock_gettime(CLOCK_REALTIME, &attr.atime);
		if (ret < 0)
		{
			node_free(node);
			return ret;
		}
		mask |= FS_ATTR_ATIME;
	}
	else if (times[0].tv_nsec != UTIME_OMIT)
	{
		ret = timespec_validate(&times[0]);
		if (ret < 0)
		{
			node_free(node);
			return ret;
		}
		attr.atime = times[0];
		mask |= FS_ATTR_ATIME;
	}
	if (!utimes || times[1].tv_nsec == UTIME_NOW)
	{
		ret = clock_gettime(CLOCK_REALTIME, &attr.mtime);
		if (ret < 0)
		{
			node_free(node);
			return ret;
		}
		mask |= FS_ATTR_MTIME;
	}
	else if (times[1].tv_nsec != UTIME_OMIT)
	{
		ret = timespec_validate(&times[1]);
		if (ret < 0)
		{
			node_free(node);
			return ret;
		}
		attr.mtime = times[1];
		mask |= FS_ATTR_MTIME;
	}
	ret = node_setattr(node, mask, &attr);
	node_free(node);
	return ret;
}

ssize_t sys_nanosleep(const struct timespec *ureq,
                      struct timespec *urem)
{
	struct thread *thread = curcpu()->thread;
	struct timespec start;
	struct timespec diff;
	struct timespec end;
	struct timespec req;
	struct timespec rem;
	ssize_t ret;
	int sleep_ret;

	ret = vm_copyin(thread->proc->vm_space, &req, ureq, sizeof(req));
	if (ret < 0)
		return ret;
	ret = timespec_validate(&req);
	if (ret < 0)
		return ret;
	if (urem)
	{
		ret = clock_gettime(CLOCK_MONOTONIC, &start);
		if (ret)
			return ret;
	}
	sleep_ret = thread_sleep(&req);
	if (urem)
	{
		ret = clock_gettime(CLOCK_MONOTONIC, &end);
		if (ret < 0)
			return ret;
		timespec_sub(&diff, &end, &start);
		if (timespec_cmp(&diff, &req) > 0)
		{
			rem.tv_sec = 0;
			rem.tv_nsec = 0;
		}
		else
		{
			timespec_sub(&rem, &req, &diff);
		}
		ret = vm_copyout(thread->proc->vm_space, urem, &rem, sizeof(rem));
		if (ret < 0)
			return ret;
	}
	return sleep_ret;
}

ssize_t sys_sigaction(int signum, const struct sigaction *uact,
                      struct sigaction *uoldact)
{
	struct thread *thread = curcpu()->thread;
	struct sigaction act;
	ssize_t ret;

	if (signum < 1
	 || signum > SIGLAST)
		return -EINVAL;
	if (uoldact)
	{
		ret = vm_copyout(thread->proc->vm_space, uoldact,
		                 &thread->proc->sigactions[signum],
		                 sizeof(*uoldact));
		if (ret < 0)
			return ret;
	}
	if (uact)
	{
		ret = vm_copyin(thread->proc->vm_space, &act, uact, sizeof(act));
		if (ret < 0)
			return ret;
		if (act.sa_flags & ~(SA_NOCLDSTOP | SA_NODEFER | SA_ONSTACK
		                   | SA_RESTART | SA_RESTORER | SA_SIGINFO))
			return -EINVAL;
		if (!(act.sa_flags & SA_RESTORER))
		{
			if (act.sa_handler != (sighandler_t)SIG_IGN
			 && act.sa_handler != (sighandler_t)SIG_DFL)
				return -EINVAL; /* XXX trampoline not supported for now */
		}
		if (signum != SIGKILL && signum != SIGSTOP)
			thread->proc->sigactions[signum] = act;
	}
	return 0;
}

ssize_t sys_mknodat(int dirfd, const char *upathname, mode_t mode,
                    dev_t dev)
{
	struct thread *thread = curcpu()->thread;
	char pathname[MAXPATHLEN];
	fs_attr_mask_t mask;
	struct fs_attr attr;
	struct node *dir;
	char *end_fn;
	ssize_t ret;

	mode &= 07777 | S_IFMT;
	switch (mode & S_IFMT)
	{
		case 0:
		case S_IFREG:
		case S_IFCHR:
		case S_IFBLK:
		case S_IFIFO:
		case S_IFDIR:
			break;
		default:
			return -EINVAL;
	}
	ret = vm_copystr(thread->proc->vm_space, pathname, upathname,
	                 sizeof(pathname));
	if (ret < 0)
		return ret;
	ret = getdirat(thread, dirfd, pathname, 0, &dir, &end_fn);
	if (ret < 0)
		return ret;
	if (!*end_fn)
	{
		node_free(dir);
		return -EISDIR;
	}
	if (is_node_rofs(dir))
	{
		node_free(dir);
		return -EROFS;
	}
	mask = FS_ATTR_UID | FS_ATTR_GID | FS_ATTR_MODE;
	attr.mode = mode & ~thread->proc->umask;
	attr.uid = thread->proc->cred.euid;
	attr.gid = thread->proc->cred.egid;
	ret = node_mknode(dir, end_fn, strlen(end_fn), mask, &attr, dev);
	node_free(dir);
	return ret;
}

ssize_t sys_chdir(const char *upathname)
{
	struct thread *thread = curcpu()->thread;
	char pathname[MAXPATHLEN];
	struct node *node;
	ssize_t ret;

	ret = vm_copystr(thread->proc->vm_space, pathname, upathname,
	                 sizeof(pathname));
	if (ret < 0)
		return ret;
	ret = vfs_getnode(NULL, pathname, 0, &node);
	if (ret < 0)
		return ret;
	if (!S_ISDIR(node->attr.mode))
	{
		node_free(node);
		return -ENOTDIR;
	}
	node_free(thread->proc->cwd);
	thread->proc->cwd = node;
	return 0;
}

ssize_t sys_fchdir(int fd)
{
	struct thread *thread = curcpu()->thread;
	struct node *node;
	ssize_t ret;

	ret = getnode(thread, fd, &node);
	if (ret < 0)
		return ret;
	if (!S_ISDIR(node->attr.mode))
	{
		node_free(node);
		return -ENOTDIR;
	}
	struct node *oldcwd = thread->proc->cwd;
	thread->proc->cwd = node;
	node_free(oldcwd);
	return 0;
}

ssize_t sys_fchmodat(int dirfd, const char *upathname, mode_t mode, int flags)
{
	struct thread *thread = curcpu()->thread;
	char pathname[MAXPATHLEN];
	struct node *node;
	struct fs_attr attr;
	ssize_t ret;

	mode &= 07777;
	if (flags & ~(AT_SYMLINK_NOFOLLOW | AT_EMPTY_PATH))
		return -EINVAL;
	ret = vm_copystr(thread->proc->vm_space, pathname, upathname,
	                 sizeof(pathname));
	if (ret < 0)
		return ret;
	ret = getnodeat(thread, dirfd, pathname, flags, &node);
	if (ret < 0)
		return ret;
	if (is_node_rofs(node))
	{
		node_free(node);
		return -EROFS;
	}
	fs_attr_mask_t mask = FS_ATTR_MODE | FS_ATTR_CTIME;
	ret = clock_gettime(CLOCK_REALTIME, &attr.ctime);
	if (ret < 0)
	{
		node_free(node);
		return ret;
	}
	attr.mode = mode;
	ret = node_setattr(node, mask, &attr);
	node_free(node);
	return ret;
}

ssize_t sys_fchownat(int dirfd, const char *upathname, uid_t uid,
                     gid_t gid, int flags)
{
	struct thread *thread = curcpu()->thread;
	char pathname[MAXPATHLEN];
	struct node *node;
	struct fs_attr attr;
	ssize_t ret;

	if (flags & ~(AT_SYMLINK_NOFOLLOW | AT_EMPTY_PATH))
		return -EINVAL;
	ret = vm_copystr(thread->proc->vm_space, pathname, upathname,
	                 sizeof(pathname));
	if (ret < 0)
		return ret;
	ret = getnodeat(thread, dirfd, pathname, flags, &node);
	if (ret < 0)
		return ret;
	if (is_node_rofs(node))
	{
		node_free(node);
		return -EROFS;
	}
	fs_attr_mask_t mask = FS_ATTR_CTIME;
	if (uid != (uid_t)-1)
	{
		mask |= FS_ATTR_UID;
		attr.uid = uid;
	}
	if (gid != (gid_t)-1)
	{
		mask |= FS_ATTR_GID;
		attr.gid = gid;
	}
	ret = clock_gettime(CLOCK_REALTIME, &attr.ctime);
	if (ret < 0)
	{
		node_free(node);
		return ret;
	}
	ret = node_setattr(node, mask, &attr);
	node_free(node);
	return ret;
}

ssize_t sys_linkat(int olddirfd, const char *uoldpath, int newdirfd,
                   const char *unewpath, int flags)
{
	struct thread *thread = curcpu()->thread;
	char oldpath[MAXPATHLEN];
	char newpath[MAXPATHLEN];
	struct node *oldnode;
	struct node *newnode;
	char *newend_fn;
	ssize_t ret;

	if (flags & ~(AT_SYMLINK_FOLLOW | AT_EMPTY_PATH))
		return -EINVAL;
	ret = vm_copystr(thread->proc->vm_space, oldpath, uoldpath,
	                 sizeof(oldpath));
	if (ret < 0)
		return ret;
	ret = vm_copystr(thread->proc->vm_space, newpath, unewpath,
	                 sizeof(newpath));
	if (ret < 0)
		return ret;
	ret = getnodeat(thread, olddirfd, oldpath,
	                (flags & AT_SYMLINK_FOLLOW) ? 0 : AT_SYMLINK_NOFOLLOW,
	                &oldnode);
	if (ret < 0)
		return ret;
	ret = getdirat(thread, newdirfd, newpath, 0, &newnode, &newend_fn);
	if (ret < 0)
	{
		node_free(oldnode);
		return ret;
	}
	if (oldnode->sb != newnode->sb)
	{
		node_free(oldnode);
		node_free(newnode);
		return -EXDEV;
	}
	if (is_node_rofs(oldnode))
	{
		node_free(oldnode);
		node_free(newnode);
		return -EROFS;
	}
	if (S_ISDIR(oldnode->attr.mode))
	{
		node_free(oldnode);
		node_free(newnode);
		return -EPERM;
	}
	ret = node_link(newnode, oldnode, newend_fn);
	node_free(oldnode);
	node_free(newnode);
	return ret;
}

ssize_t sys_faccessat(int dirfd, const char *upathname, int mode,
                      int flags)
{
	struct thread *thread = curcpu()->thread;
	char pathname[MAXPATHLEN];
	struct node *node;
	ssize_t ret;

	if (flags & ~(AT_EACCESS | AT_SYMLINK_NOFOLLOW | AT_EMPTY_PATH))
		return -EINVAL;
	if (mode & ~(R_OK | W_OK | X_OK))
		return -EINVAL;
	ret = vm_copystr(thread->proc->vm_space, pathname, upathname,
	                 sizeof(pathname));
	if (ret < 0)
		return ret;
	ret = getnodeat(thread, dirfd, pathname, flags, &node);
	if (ret < 0)
		return ret;
	uid_t uid;
	gid_t gid;
	if (flags & AT_EACCESS)
	{
		uid = thread->proc->cred.euid;
		gid = thread->proc->cred.egid;
	}
	else
	{
		uid = thread->proc->cred.uid;
		gid = thread->proc->cred.gid;
	}
	if (!(vfs_getperm(node, uid, gid) & mode))
		ret = -EACCES;
	node_free(node);
	return ret;
}

ssize_t sys_umask(mode_t mask)
{
	struct thread *thread = curcpu()->thread;
	mode_t prev = thread->proc->umask;
	thread->proc->umask = mask & 0777;
	return prev;
}

ssize_t sys_symlinkat(const char *utarget, int newdirfd,
                      const char *ulinkpath)
{
	struct thread *thread = curcpu()->thread;
	char target[MAXPATHLEN];
	char linkpath[MAXPATHLEN];
	char *end_fn;
	fs_attr_mask_t mask;
	struct node *node;
	struct fs_attr attr;
	ssize_t ret;

	ret = vm_copystr(thread->proc->vm_space, target, utarget, sizeof(target));
	if (ret < 0)
		return ret;
	ret = vm_copystr(thread->proc->vm_space, linkpath, ulinkpath,
	                 sizeof(linkpath));
	if (ret < 0)
		return ret;
	ret = getdirat(thread, newdirfd, linkpath, 0, &node, &end_fn);
	if (ret < 0)
		return ret;
	mask = FS_ATTR_UID | FS_ATTR_GID | FS_ATTR_MODE;
	attr.uid = thread->proc->cred.euid;
	attr.gid = thread->proc->cred.egid;
	attr.mode = 0777;
	ret = node_symlink(node, end_fn, strlen(end_fn), target, mask, &attr);
	node_free(node);
	return ret;
}

ssize_t sys_getpagesize()
{
	return PAGE_SIZE;
}

ssize_t sys_renameat(int olddirfd, const char *uoldpath, int newdirfd,
                     const char *unewpath)
{
	struct thread *thread = curcpu()->thread;
	char oldpath[MAXPATHLEN];
	char newpath[MAXPATHLEN];
	struct node *olddir;
	char *oldend_fn;
	struct node *newdir;
	char *newend_fn;
	ssize_t ret;

	ret = vm_copystr(thread->proc->vm_space, oldpath, uoldpath,
	                 sizeof(oldpath));
	if (ret < 0)
		return ret;
	ret = vm_copystr(thread->proc->vm_space, newpath, unewpath,
	                 sizeof(newpath));
	if (ret < 0)
		return ret;
	ret = getdirat(thread, olddirfd, oldpath, 0, &olddir, &oldend_fn);
	if (ret < 0)
		return ret;
	ret = getdirat(thread, newdirfd, newpath, 0, &newdir, &newend_fn);
	if (ret < 0)
	{
		node_free(olddir);
		return ret;
	}
	if (olddir->sb != newdir->sb)
	{
		node_free(olddir);
		node_free(newdir);
		return -EXDEV;
	}
	if (is_node_rofs(olddir))
	{
		node_free(olddir);
		node_free(newdir);
		return -EROFS;
	}
	ret = node_rename(olddir, oldend_fn, newdir, newend_fn);
	node_free(olddir);
	node_free(newdir);
	return ret;
}

ssize_t sys_uname(struct utsname *buf)
{
	struct thread *thread = curcpu()->thread;
	struct utsname tmp;

	strlcpy(tmp.sysname, "eklatos", sizeof(tmp.sysname));
	strlcpy(tmp.nodename, "eklatos", sizeof(tmp.nodename));
	strlcpy(tmp.release, "0.0.1", sizeof(tmp.release));
	strlcpy(tmp.version, "0.0.1", sizeof(tmp.version));
#if defined(__i386__)
	strlcpy(tmp.machine, "x86", sizeof(tmp.machine));
#elif defined(__x86_64__)
	strlcpy(tmp.machine, "x86_64", sizeof(tmp.machine));
#elif defined(__arm__)
	strlcpy(tmp.machine, "arm", sizeof(tmp.machine));
#elif defined(__aarch64__)
	strlcpy(tmp.machine, "aarch64", sizeof(tmp.machine));
#elif defined(__riscv) && __riscv_xlen == 32
	strlcpy(tmp.machine, "riscv32", sizeof(tmp.machine));
#elif defined(__riscv) && __riscv_xlen == 64
	strlcpy(tmp.machine, "riscv64", sizeof(tmp.machine));
#elif defined(__mips) && defined(__mips64)
	strlcpy(tmp.machine, "mipsel64", sizeof(tmp.machine));
#elif defined(__mips)
	strlcpy(tmp.machine, "mipsel", sizeof(tmp.machine));
#else
# error "unknown arch"
#endif
	return vm_copyout(thread->proc->vm_space, buf, &tmp, sizeof(tmp));
}

ssize_t sys_mount(const char *usource, const char *utarget,
                  const char *utype, unsigned long flags,
                  const void *data)
{
	struct thread *thread = curcpu()->thread;
	char source[MAXPATHLEN];
	char target[MAXPATHLEN];
	char type[256];
	const struct fs_type *fs_type;
	struct node *source_node = NULL;
	struct node *target_node;
	ssize_t ret;

	if (flags) /* XXX */
		return -EINVAL;
	if (usource)
	{
		ret = vm_copystr(thread->proc->vm_space, source, usource,
		                 sizeof(source));
		if (ret < 0)
			return ret;
	}
	ret = vm_copystr(thread->proc->vm_space, target, utarget,
	                 sizeof(target));
	if (ret < 0)
		return ret;
	ret = vm_copystr(thread->proc->vm_space, type, utype, sizeof(type));
	if (ret < 0)
		return ret;
	fs_type = vfs_get_fs_type(type);
	if (!fs_type)
		return -ENODEV;
	if (usource)
	{
		ret = vfs_getnode(thread->proc->cwd, source, 0, &source_node);
		if (ret)
			return ret;
	}
	ret = vfs_getnode(thread->proc->cwd, target, 0, &target_node);
	if (ret < 0)
	{
		if (source_node)
			node_free(source_node);
		return ret;
	}
	ret = vfs_mount(target_node, source_node, fs_type, flags, data, NULL);
	if (source_node)
		node_free(source_node);
	node_free(target_node);
	return 0;
}

ssize_t sys_recvmsg(int fd, struct msghdr *umsg, int flags)
{
	struct thread *thread = curcpu()->thread;
	struct msghdr msg;
	struct iovec iov[IOV_MAX];
	struct file *file;
	struct sock *sock;
	ssize_t len;
	ssize_t ret;

	ret = vm_copyin(thread->proc->vm_space, &msg, umsg, sizeof(msg));
	if (ret < 0)
		return ret;
	if (msg.msg_iovlen > IOV_MAX)
		return -EINVAL;
	if (msg.msg_flags & ~(MSG_DONTWAIT))
		return -EINVAL;
	ret = vm_copyin(thread->proc->vm_space, iov, msg.msg_iov,
	                sizeof(*iov) * msg.msg_iovlen);
	if (ret < 0)
		return ret;
	msg.msg_iov = iov;
	ret = proc_getfile(thread->proc, fd, &file);
	if (ret < 0)
		return ret;
	sock = file->sock;
	if (!sock)
	{
		file_free(file);
		return -ENOTSOCK;
	}
	if (file->flags & O_NONBLOCK)
		flags |= MSG_DONTWAIT;
	len = sock_recv(sock, &msg, flags);
	file_free(file);
	if (len < 0)
		return len;
	ret = vm_copyout(thread->proc->vm_space, umsg, &msg, sizeof(msg));
	if (ret < 0)
		return ret;
	return len;
}

ssize_t sys_sendmsg(int fd, struct msghdr *umsg, int flags)
{
	struct thread *thread = curcpu()->thread;
	struct msghdr msg;
	struct iovec iov[IOV_MAX];
	struct file *file;
	struct sock *sock;
	ssize_t ret;

	ret = vm_copyin(thread->proc->vm_space, &msg, umsg, sizeof(msg));
	if (ret < 0)
		return ret;
	if (msg.msg_iovlen > IOV_MAX)
		return -EINVAL;
	if (msg.msg_flags & ~(MSG_DONTWAIT))
		return -EINVAL;
	ret = vm_copyin(thread->proc->vm_space, iov, msg.msg_iov, sizeof(*iov) * msg.msg_iovlen);
	if (ret < 0)
		return ret;
	msg.msg_iov = iov;
	ret = proc_getfile(thread->proc, fd, &file);
	if (ret < 0)
		return ret;
	sock = file->sock;
	if (!sock)
	{
		file_free(file);
		return -ENOTSOCK;
	}
	if (file->flags & O_NONBLOCK)
		flags |= MSG_DONTWAIT;
	ret = sock_send(sock, &msg, flags);
	file_free(file);
	return ret;
}

ssize_t sys_fstatvfsat(int dirfd, const char *upathname,
                       struct statvfs *ubuf, int flags)
{
	struct thread *thread = curcpu()->thread;
	char pathname[MAXPATHLEN];
	struct node *node;
	struct statvfs buf;
	ssize_t ret;

	if (flags & ~AT_EMPTY_PATH)
		return -EINVAL;
	ret = vm_copystr(thread->proc->vm_space, pathname, upathname,
	                 sizeof(pathname));
	if (ret < 0)
		return ret;
	ret = getnodeat(thread, dirfd, pathname, flags, &node);
	if (ret < 0)
		return ret;
	if (!node->sb->type->op->stat)
	{
		node_free(node);
		return -ENOSYS;
	}
	ret = node->sb->type->op->stat(node->sb, &buf);
	node_free(node);
	if (ret < 0)
		return ret;
	return vm_copyout(thread->proc->vm_space, ubuf, &buf, sizeof(buf));
}

ssize_t sys_ftruncateat(int dirfd, const char *upathname, off_t *ulength,
                        int flags)
{
	struct thread *thread = curcpu()->thread;
	char pathname[MAXPATHLEN];
	struct node *node;
	struct fs_attr attr;
	off_t length;
	ssize_t ret;

	if (flags & ~AT_EMPTY_PATH)
		return -EINVAL;
	ret = vm_copyin(thread->proc->vm_space, &length, ulength, sizeof(length));
	if (ret < 0)
		return ret;
	if (length < 0)
		return -EINVAL;
	ret = vm_copystr(thread->proc->vm_space, pathname, upathname,
	                 sizeof(pathname));
	if (ret < 0)
		return ret;
	ret = getnodeat(thread, dirfd, pathname, flags, &node);
	if (ret < 0)
		return ret;
	switch (node->attr.mode & S_IFMT)
	{
		case S_IFREG:
			break;
		case S_IFDIR:
			node_free(node);
			return -EISDIR;
		case S_IFBLK:
		case S_IFCHR:
		case S_IFIFO:
		case S_IFLNK:
		case S_IFSOCK:
			node_free(node);
			return 0;
		default:
			node_free(node);
			return -EINVAL;
	}
	attr.size = length;
	ret = node_setattr(node, FS_ATTR_SIZE, &attr);
	node_free(node);
	return ret;
}

ssize_t sys_fcntl(int fd, int cmd, uintptr_t data)
{
	struct thread *thread = curcpu()->thread;
	struct file *file;
	ssize_t ret;

	switch (cmd)
	{
		case F_DUPFD:
			ret = proc_getfile(thread->proc, fd, &file);
			if (ret < 0)
				return ret;
			ret = proc_allocfd(thread->proc, file, 0);
			file_free(file);
			return ret;
		case F_GETFD:
		{
			rwlock_wrlock(&thread->proc->files_lock);
			if (fd < 0 || (size_t)fd >= thread->proc->files_nb)
			{
				rwlock_unlock(&thread->proc->files_lock);
				return -EBADF;
			}
			ret = thread->proc->files[fd].cloexec;
			rwlock_unlock(&thread->proc->files_lock);
			return ret;
		}
		case F_SETFD:
		{
			rwlock_wrlock(&thread->proc->files_lock);
			if (fd < 0 || (size_t)fd >= thread->proc->files_nb)
			{
				rwlock_unlock(&thread->proc->files_lock);
				return -EBADF;
			}
			thread->proc->files[fd].cloexec = (thread->proc->files[fd].cloexec & ~FD_CLOEXEC)
			                                | (data & FD_CLOEXEC);
			rwlock_unlock(&thread->proc->files_lock);
			return 0;
		}
		case F_GETFL:
			ret = proc_getfile(thread->proc, fd, &file);
			if (ret < 0)
				return ret;
			ret = file->flags;
			file_free(file);
			return ret;
		case F_SETFL:
		{
			ret = proc_getfile(thread->proc, fd, &file);
			if (ret < 0)
				return ret;
			static const int setfl_mask = O_APPEND | O_ASYNC | O_TRUNC | O_NONBLOCK;
			file->flags = (file->flags & ~setfl_mask)
			            | (data & setfl_mask);
			file_free(file);
			return 0;
		}
		default:
			return -EINVAL;
	}
}

ssize_t sys_sched_yield()
{
	sched_yield();
	return 0;
}

ssize_t sys_ptrace(enum __ptrace_request request, pid_t pid, void *addr,
                   void *data)
{
	struct thread *thread = curcpu()->thread;

	switch (request)
	{
		case PTRACE_TRACEME:
			if (thread->ptrace_state != PTRACE_ST_NONE)
				return -EINVAL;
			thread_trace(thread->proc->parent, thread);
			thread_ptrace_stop(thread, SIGTRAP);
			break;
		case PTRACE_SYSCALL:
		case PTRACE_CONT:
		case PTRACE_SINGLESTEP:
		case PTRACE_DETACH:
		{
			int signum;
			if (data)
			{
				if ((uintptr_t)data > SIGLAST)
					return -EIO;
				signum = (int)(intptr_t)data;
			}
			else
			{
				signum = 0;
			}
			struct thread *tracee = getthread(pid); /* XXX limit to child ?*/
			if (!tracee
			 || tracee->ptrace_tracer != thread->proc
			 || tracee->ptrace_state != PTRACE_ST_STOPPED)
			{
				thread_free(tracee);
				return -ESRCH;
			}
			spinlock_lock(&tracee->ptrace_waitq_lock);
			if (signum)
				thread_signal(tracee, signum);
			switch (request)
			{
				case PTRACE_SYSCALL:
					tracee->ptrace_state = PTRACE_ST_SYSCALL;
					break;
				case PTRACE_CONT:
					tracee->ptrace_state = PTRACE_ST_RUNNING;
					break;
				case PTRACE_SINGLESTEP:
					tracee->ptrace_state = PTRACE_ST_ONESTEP;
					break;
				case PTRACE_DETACH:
					thread_untrace(tracee);
					break;
				default:
					panic("dead\n");
			}
			waitq_signal(&tracee->ptrace_waitq, 0);
			spinlock_unlock(&tracee->ptrace_waitq_lock);
			thread_free(tracee);
			break;
		}
		case PTRACE_GETREGS:
		{
			struct thread *tracee = getthread(pid);
			if (!tracee
			 || tracee->ptrace_tracer != thread->proc
			 || tracee->ptrace_state != PTRACE_ST_STOPPED)
			{
				thread_free(tracee);
				return -ESRCH;
			}
			ssize_t ret = vm_copyout(thread->proc->vm_space, data,
			                         &tracee->tf_user.regs,
			                         sizeof(struct user_regs));
			thread_free(tracee);
			return ret;
		}
		case PTRACE_SETREGS:
		{
			struct thread *tracee = getthread(pid);
			if (!tracee
			 || tracee->ptrace_tracer != thread->proc
			 || tracee->ptrace_state != PTRACE_ST_STOPPED)
			{
				thread_free(tracee);
				return -ESRCH;
			}
			ssize_t ret = vm_copyin(thread->proc->vm_space,
			                        &tracee->tf_user.regs,
			                        data, sizeof(struct user_regs));
			thread_free(tracee);
			return ret;
		}
		case PTRACE_GETFPREGS:
		{
			struct thread *tracee = getthread(pid);
			if (!tracee
			 || tracee->ptrace_tracer != thread->proc
			 || tracee->ptrace_state != PTRACE_ST_STOPPED)
			{
				thread_free(tracee);
				return -ESRCH;
			}
			ssize_t ret = vm_copyout(thread->proc->vm_space, data,
			                         &tracee->tf_user.fpu_data,
			                         sizeof(struct user_fpu));
			thread_free(tracee);
			return ret;
		}
		case PTRACE_SETFPREGS:
		{
			struct thread *tracee = getthread(pid);
			if (!tracee
			 || tracee->ptrace_tracer != thread->proc
			 || tracee->ptrace_state != PTRACE_ST_STOPPED)
			{
				thread_free(tracee);
				return -ESRCH;
			}
			ssize_t ret = vm_copyin(thread->proc->vm_space,
			                        &tracee->tf_user.fpu_data,
			                        data, sizeof(struct user_fpu));
			thread_free(tracee);
			return ret;
		}
		case PTRACE_PEEKDATA:
		case PTRACE_PEEKTEXT:
		{
			struct thread *tracee = getthread(pid);
			if (!tracee
			 || tracee->ptrace_tracer != thread->proc
			 || tracee->ptrace_state != PTRACE_ST_STOPPED)
			{
				thread_free(tracee);
				return -ESRCH;
			}
			size_t v;
			ssize_t ret = vm_copyin(tracee->proc->vm_space, &v,
			                        addr, sizeof(v));
			if (ret < 0)
			{
				thread_free(tracee);
				return ret;
			}
			ret = vm_copyout(thread->proc->vm_space, data, &v,
			                 sizeof(v));
			thread_free(tracee);
			return ret;
		}
		case PTRACE_POKEDATA:
		case PTRACE_POKETEXT:
		{
			struct thread *tracee = getthread(pid);
			if (!tracee
			 || tracee->ptrace_tracer != thread->proc
			 || tracee->ptrace_state != PTRACE_ST_STOPPED)
			{
				thread_free(tracee);
				return -ESRCH;
			}
			ssize_t ret = vm_copyout(tracee->proc->vm_space, addr,
			                         &data, sizeof(data));
			thread_free(tracee);
			return ret;
		}
		case PTRACE_GETSIGINFO:
		{
			struct thread *tracee = getthread(pid);
			if (!tracee
			 || tracee->ptrace_tracer != thread->proc
			 || tracee->ptrace_state != PTRACE_ST_STOPPED)
			{
				thread_free(tracee);
				return -ESRCH;
			}
			/* XXX */
			siginfo_t siginfo;
			memset(&siginfo, 0, sizeof(siginfo));
			ssize_t ret = vm_copyout(thread->proc->vm_space, data,
			                         &siginfo, sizeof(siginfo));
			thread_free(tracee);
			return ret;
		}
		case PTRACE_GET_THREAD_AREA:
		{
			struct thread *tracee = getthread(pid);
			if (!tracee
			 || tracee->ptrace_tracer != thread->proc
			 || tracee->ptrace_state != PTRACE_ST_STOPPED)
			{
				thread_free(tracee);
				return -ESRCH;
			}
			ssize_t ret = vm_copyout(thread->proc->vm_space, data,
			                         &thread->tls_addr,
			                         sizeof(uintptr_t));
			thread_free(tracee);
			return ret;
		}
		case PTRACE_SET_THREAD_AREA:
		{
			struct thread *tracee = getthread(pid);
			if (!tracee
			 || tracee->ptrace_tracer != thread->proc
			 || tracee->ptrace_state != PTRACE_ST_STOPPED)
			{
				thread_free(tracee);
				return -ESRCH;
			}
			tracee->tls_addr = (uintptr_t)data;
			thread_free(tracee);
			break;
		}
		case PTRACE_SETOPTIONS:
		{
			uintptr_t options = (uintptr_t)data;
			if (options & ~(PTRACE_O_TRACESYSGOOD
			              | PTRACE_O_EXITKILL
			              | PTRACE_O_TRACECLONE
			              | PTRACE_O_TRACEFORK
			              | PTRACE_O_TRACEVFORK))
				return -EIO;
			struct thread *tracee = getthread(pid);
			if (!tracee
			 || tracee->ptrace_tracer != thread->proc
			 || tracee->ptrace_state != PTRACE_ST_STOPPED)
			{
				thread_free(tracee);
				return -ESRCH;
			}
			tracee->ptrace_options = options;
			thread_free(tracee);
			break;
		}
		case PTRACE_GETSIGMASK:
		{
			struct thread *tracee = getthread(pid);
			if (!tracee
			 || tracee->ptrace_tracer != thread->proc
			 || tracee->ptrace_state != PTRACE_ST_STOPPED)
			{
				thread_free(tracee);
				return -ESRCH;
			}
			sigset_t set;
			le64enc(set.set, tracee->sigmask);
			ssize_t ret = vm_copyout(thread->proc->vm_space, data,
			                         &set, sizeof(set));
			thread_free(tracee);
			return ret;
		}
		case PTRACE_SETSIGMASK:
		{
			struct thread *tracee = getthread(pid);
			if (!tracee
			 || tracee->ptrace_tracer != thread->proc
			 || tracee->ptrace_state != PTRACE_ST_STOPPED)
			{
				thread_free(tracee);
				return -ESRCH;
			}
			sigset_t set;
			ssize_t ret = vm_copyin(thread->proc->vm_space, &set,
			                        data, sizeof(set));
			if (ret)
			{
				thread_free(tracee);
				return ret;
			}
			tracee->sigmask = le64dec(set.set);
			tracee->sigmask &= ~(1 << SIGKILL);
			tracee->sigmask &= ~(1 << SIGSTOP);
			thread_free(tracee);
			break;
		}
		default:
			return -EIO;
	}
	return 0;
}

ssize_t sys_sigprocmask(int how, const sigset_t *uset, sigset_t *uoldset)
{
	struct thread *thread = curcpu()->thread;
	uint64_t old_mask;
	uint64_t new_mask;
	uint64_t mask;
	sigset_t set;
	ssize_t ret;

	if (how != SIG_BLOCK && how != SIG_UNBLOCK && how != SIG_SETMASK)
		return -EINVAL;
	old_mask = thread->sigmask;
	if (uset)
	{
		ret = vm_copyin(thread->proc->vm_space, &set, uset, sizeof(set));
		if (ret < 0)
			return ret;
		mask = le64dec(set.set);
		switch (how)
		{
			case SIG_BLOCK:
				new_mask = old_mask | mask;
				break;
			case SIG_UNBLOCK:
				new_mask = old_mask & ~mask;
				break;
			case SIG_SETMASK:
				new_mask = mask;
				break;
		}
		new_mask &= ~(1 << SIGKILL);
		new_mask &= ~(1 << SIGSTOP);
		thread->sigmask = new_mask;
	}
	if (!uoldset)
		return 0;
	le64enc(set.set, old_mask);
	return vm_copyout(thread->proc->vm_space, uoldset, &set, sizeof(set));
}

ssize_t sys_kmload(int fd, const char *params, int flags)
{
	struct thread *thread = curcpu()->thread;
	struct file *file = NULL;
	struct node *node = NULL;
	struct kmod *kmod;
	ssize_t ret;

	(void)params; /* XXX */
	(void)flags; /* XXX */
	if (thread->proc->cred.euid)
		return -EPERM;
	ret = getnode(thread, fd, &node);
	if (ret < 0)
		return ret;
	ret = file_fromnode(node, O_RDONLY, &file);
	node_free(node);
	if (ret < 0)
		return ret;
	ret = file_open(file, node);
	if (ret < 0)
	{
		file_free(file);
		return ret;
	}
	ret = kmod_load(file, &kmod);
	file_free(file);
	if (ret < 0)
		return ret;
	return 0;
}

ssize_t sys_kmunload(const char *uname, int flags)
{
	struct thread *thread = curcpu()->thread;
	struct kmod *kmod;
	char name[KMOD_NAME_SIZE];
	ssize_t ret;

	(void)flags; /* XXX */
	if (thread->proc->cred.euid)
		return -EPERM;
	ret = vm_copystr(thread->proc->vm_space, name, uname, sizeof(name));
	if (ret < 0)
		return ret;
	kmod = kmod_get(name);
	if (!kmod)
		return -ENOENT;
	ret = kmod_unload(kmod);
	kmod_free(kmod);
	if (ret < 0)
		return ret;
	return 0;
}

ssize_t sys_getpriority(int which, id_t who, int *uprio)
{
	struct thread *thread = curcpu()->thread;
	switch (which)
	{
		case PRIO_PROCESS:
			if (!who)
				who = thread->tid;
			if (who == thread->tid)
			{
				int prio = thread->pri - PRI_USER;
				return vm_copyout(thread->proc->vm_space,
				                  uprio, &prio, sizeof(prio));
			}
			/* XXX */
			return -ENOSYS;
		case PRIO_PGRP:
			/* XXX */
			return -ENOSYS;
		case PRIO_USER:
			/* XXX */
			return -ENOSYS;
		default:
			return -EINVAL;
	}
}

ssize_t sys_setpriority(int which, id_t who, int prio)
{
	if (prio < -20)
		prio = -20;
	if (prio > 19)
		prio = 19;
	struct thread *thread = curcpu()->thread;
	switch (which)
	{
		case PRIO_PROCESS:
			if (!who)
				who = thread->tid;
			if (who == thread->tid)
			{
				pri_t new_prio = PRI_USER + prio;
				if (thread->proc->cred.euid
				 && new_prio < thread->pri)
					return -EPERM;
				thread->pri = new_prio;
				return 0;
			}
			/* XXX */
			return -ENOSYS;
		case PRIO_PGRP:
			/* XXX */
			return -ENOSYS;
		case PRIO_USER:
			/* XXX */
			return -ENOSYS;
		default:
			return -EINVAL;
	}
}

ssize_t sys_pselect(int fds, fd_set *urfds, fd_set *uwfds, fd_set *uefds,
                    const struct timespec *utimeout, const sigset_t *usigmask)
{
	struct thread *thread = curcpu()->thread;
	struct poll_entry entries[FD_SETSIZE]; /* XXX come on.. */
	struct poll_entry *entry;
	struct timespec timeout;
	struct poller poller;
	uint64_t old_mask = thread->sigmask;
	sigset_t sigmask;
	fd_set rfds;
	fd_set wfds;
	fd_set efds;
	ssize_t ret;

	if (fds < 0 || fds > FD_SETSIZE)
		return -EINVAL;
	if (utimeout)
	{
		ret = vm_copyin(thread->proc->vm_space, &timeout, utimeout,
		                sizeof(timeout));
		if (ret < 0)
			return ret;
		ret = timespec_validate(&timeout);
		if (ret < 0)
			return ret;
	}
	if (usigmask)
	{
		ret = vm_copyin(thread->proc->vm_space, &sigmask, usigmask,
		                sizeof(sigmask));
		if (ret < 0)
			return ret;
	}
	if ((unsigned)fds > thread->proc->files_nb)
		fds = thread->proc->files_nb;
	if (!fds)
	{
		if (usigmask)
		{
			uint64_t new_mask = le64dec(sigmask.set);
			new_mask &= ~(1 << SIGKILL);
			new_mask &= ~(1 << SIGSTOP);
			thread->sigmask = new_mask;
		}
		ret = thread_sleep(utimeout ? &timeout : NULL);
		if (ret == -EAGAIN)
			ret = 0;
		if (usigmask)
			thread->sigmask = old_mask;
		return ret;
	}
	if (urfds)
	{
		ret = vm_copyin(thread->proc->vm_space, &rfds, urfds,
		                (fds + FDSET_BPW - 1) / 8);
		if (ret < 0)
			return ret;
	}
	if (uwfds)
	{
		ret = vm_copyin(thread->proc->vm_space, &wfds, uwfds,
		                (fds + FDSET_BPW - 1) / 8);
		if (ret < 0)
			return ret;
	}
	if (uefds)
	{
		ret = vm_copyin(thread->proc->vm_space, &efds, uefds,
		                (fds + FDSET_BPW - 1) / 8);
		if (ret < 0)
			return ret;
	}
	ret = poller_init(&poller);
	if (ret < 0)
		return ret;
	int ndata = 0;
	for (int i = 0; i < fds; ++i)
	{
		int rf = urfds ? FD_ISSET(i, &rfds) : 0;
		int wf = uwfds ? FD_ISSET(i, &wfds) : 0;
		int ef = uefds ? FD_ISSET(i, &efds) : 0;
		if (!rf && !wf && !ef)
		{
			entries[i].file = NULL;
			continue;
		}
		struct file *file;
		ret = proc_getfile(thread->proc, i, &file);
		if (ret < 0)
			goto end;
		entries[i].poller = &poller;
		entries[i].file = file;
		entries[i].events = 0;
		if (rf)
			entries[i].events |= POLLIN_SET;
		if (wf)
			entries[i].events |= POLLOUT_SET;
		if (ef)
			entries[i].events |= POLLEX_SET;
		ret = file_poll(file, &entries[i]);
		file_free(file);
		if (ret < 0)
			goto end;
		entries[i].revents = ret;
		if (rf && !(ret & POLLIN_SET))
			FD_CLR(i, &rfds);
		if (wf && !(ret & POLLOUT_SET))
			FD_CLR(i, &wfds);
		if (ef && !(ret & POLLEX_SET))
			FD_CLR(i, &efds);
		if (ret)
			++ndata;
	}
	if (ndata)
		goto copyend;
	if (usigmask)
	{
		uint64_t new_mask = le64dec(sigmask.set);
		new_mask &= ~(1 << SIGKILL);
		new_mask &= ~(1 << SIGSTOP);
		thread->sigmask = new_mask;
	}
	ret = poller_wait(&poller, utimeout ? &timeout : NULL);
	if (usigmask)
		thread->sigmask = old_mask;
	if (ret == -EAGAIN)
	{
		ndata = 0;
		goto copyend;
	}
	if (ret < 0)
		goto end;
	poller_spinlock(&poller);
	TAILQ_FOREACH(entry, &poller.ready_entries, poller_chain)
	{
		size_t index = entry - &entries[0];
		if (urfds && (entry->revents & POLLIN_SET))
			FD_SET(index, &rfds);
		if (uwfds && (entry->revents & POLLOUT_SET))
			FD_SET(index, &wfds);
		if (uefds && (entry->revents & POLLEX_SET))
			FD_SET(index, &efds);
		++ndata;
	}
	poller_unlock(&poller);

copyend:
	if (urfds)
	{
		ret = vm_copyout(thread->proc->vm_space, urfds, &rfds,
		                 (fds + FDSET_BPW - 1) / 8);
		if (ret < 0)
			goto end;
	}
	if (uwfds)
	{
		ret = vm_copyout(thread->proc->vm_space, uwfds, &wfds,
		                 (fds + FDSET_BPW - 1) / 8);
		if (ret < 0)
			goto end;
	}
	if (uefds)
	{
		ret = vm_copyout(thread->proc->vm_space, uefds, &efds,
		                 (fds + FDSET_BPW - 1) / 8);
		if (ret < 0)
			goto end;
	}
	ret = ndata;

end:
	poller_destroy(&poller);
	return ret;
}

ssize_t sys_ppoll(struct pollfd *ufds, nfds_t nfds,
                  const struct timespec *utimeout, const sigset_t *usigmask)
{
	struct thread *thread = curcpu()->thread;
	struct poll_entry entries[FD_SETSIZE]; /* XXX come on.. */
	struct pollfd fds[FD_SETSIZE];
	struct poll_entry *entry;
	struct timespec timeout;
	struct poller poller;
	uint64_t old_mask = thread->sigmask;
	sigset_t sigmask;
	ssize_t ret;

	if (nfds > FD_SETSIZE)
		return -EINVAL;
	if (utimeout)
	{
		ret = vm_copyin(thread->proc->vm_space, &timeout, utimeout,
		                sizeof(timeout));
		if (ret < 0)
			return ret;
		ret = timespec_validate(&timeout);
		if (ret < 0)
			return ret;
	}
	if (usigmask)
	{
		ret = vm_copyin(thread->proc->vm_space, &sigmask, usigmask,
		                sizeof(sigmask));
		if (ret < 0)
			return ret;
	}
	if (!nfds)
	{
		if (usigmask)
		{
			uint64_t new_mask = le64dec(sigmask.set);
			new_mask &= ~(1 << SIGKILL);
			new_mask &= ~(1 << SIGSTOP);
			thread->sigmask = new_mask;
		}
		ret = thread_sleep(utimeout ? &timeout : NULL);
		if (ret == -EAGAIN)
			ret = 0;
		if (usigmask)
			thread->sigmask = old_mask;
		return ret;
	}
	ret = vm_copyin(thread->proc->vm_space, fds, ufds, sizeof(*fds) * nfds);
	if (ret < 0)
		return ret;
	ret = poller_init(&poller);
	if (ret < 0)
		return ret;
	int ndata = 0;
	for (nfds_t i = 0; i < nfds; ++i)
	{
		struct pollfd *pollfd = &fds[i];
		if (pollfd->fd < 0)
		{
			pollfd->revents = POLLNVAL;
			++ndata;
			continue;
		}
		struct file *file;
		ret = proc_getfile(thread->proc, pollfd->fd, &file);
		if (ret == -EBADF)
		{
			pollfd->revents = POLLNVAL;
			++ndata;
			continue;
		}
		if (ret < 0)
			goto end;
		entries[i].poller = &poller;
		entries[i].file = file;
		entries[i].events = pollfd->events & (POLLIN | POLLPRI | POLLOUT);
		entries[i].events |= POLLERR | POLLHUP;
		ret = file_poll(file, &entries[i]);
		file_free(file);
		if (ret < 0)
			goto end;
		entries[i].revents = ret;
		pollfd->revents = ret;
		if (ret)
			++ndata;
	}
	if (ndata)
		goto copyend;
	if (usigmask)
	{
		uint64_t new_mask = le64dec(sigmask.set);
		new_mask &= ~(1 << SIGKILL);
		new_mask &= ~(1 << SIGSTOP);
		thread->sigmask = new_mask;
	}
	ret = poller_wait(&poller, utimeout ? &timeout : NULL);
	if (usigmask)
		thread->sigmask = old_mask;
	if (ret == -EAGAIN)
	{
		ndata = 0;
		goto copyend;
	}
	if (ret < 0)
		goto end;
	poller_spinlock(&poller);
	TAILQ_FOREACH(entry, &poller.ready_entries, poller_chain)
	{
		size_t index = entry - &entries[0];
		fds[index].revents = entry->revents;
		++ndata;
	}
	poller_unlock(&poller);

copyend:
	ret = vm_copyout(thread->proc->vm_space, ufds, fds, sizeof(*fds) * nfds);
	if (ret < 0)
		goto end;
	ret = ndata;

end:
	poller_destroy(&poller);
	return ret;
}

ssize_t sys_getsockopt(int fd, int level, int opt, void *uval,
                       socklen_t *ulen)
{
	struct thread *thread = curcpu()->thread;
	struct sock *sock;
	ssize_t ret;

	ret = getsock(thread, fd, &sock);
	if (ret < 0)
		return ret;
	ret = sock_getopt(sock, level, opt, uval, ulen);
	sock_free(sock);
	return ret;
}

ssize_t sys_setsockopt(int fd, int level, int opt, const void *uval,
                       socklen_t len)
{
	struct thread *thread = curcpu()->thread;
	struct sock *sock;
	ssize_t ret;

	ret = getsock(thread, fd, &sock);
	if (ret < 0)
		return ret;
	ret = sock_setopt(sock, level, opt, uval, len);
	sock_free(sock);
	return ret;
}

ssize_t sys_getpeername(int fd, struct sockaddr *uaddr, socklen_t *ulen)
{
	struct thread *thread = curcpu()->thread;
	struct sock *sock;
	socklen_t len;
	ssize_t ret;

	ret = getsock(thread, fd, &sock);
	if (ret < 0)
		return ret;
	if (!sock->dst_addrlen)
	{
		ret = -ENOTCONN;
		goto end;
	}
	ret = vm_copyin(thread->proc->vm_space, &len, ulen, sizeof(len));
	if (ret < 0)
		goto end;
	if (!len)
	{
		ret = -EINVAL;
		goto end;
	}
	ret = vm_copyout(thread->proc->vm_space, uaddr, &sock->dst_addr,
	                 len < sock->dst_addrlen ? len : sock->dst_addrlen);
	if (ret < 0)
		goto end;
	ret = vm_copyout(thread->proc->vm_space, ulen, &sock->dst_addrlen,
	                 sizeof(*ulen));
	if (ret < 0)
		goto end;
	ret = 0;

end:
	sock_free(sock);
	return ret;
}

ssize_t sys_getsockname(int fd, struct sockaddr *uaddr, socklen_t *ulen)
{
	struct thread *thread = curcpu()->thread;
	struct sock *sock;
	socklen_t len;
	ssize_t ret;

	ret = getsock(thread, fd, &sock);
	if (ret < 0)
		return ret;
	if (!sock->src_addrlen)
	{
		ret = -EINVAL;
		goto end;
	}
	ret = vm_copyin(thread->proc->vm_space, &len, ulen, sizeof(len));
	if (ret < 0)
		goto end;
	if (!len)
	{
		ret = -EINVAL;
		goto end;
	}
	ret = vm_copyout(thread->proc->vm_space, uaddr, &sock->src_addr,
	                 len < sock->src_addrlen ? len : sock->src_addrlen);
	if (ret < 0)
		goto end;
	ret = vm_copyout(thread->proc->vm_space, ulen, &sock->src_addrlen,
	                 sizeof(*ulen));
	if (ret < 0)
		goto end;
	ret = 0;

end:
	sock_free(sock);
	return ret;
}

ssize_t sys_shutdown(int fd, int how)
{
	struct thread *thread = curcpu()->thread;
	struct sock *sock;
	ssize_t ret;

	switch (how)
	{
		case SHUT_RD:
		case SHUT_WR:
		case SHUT_RDWR:
			break;
		default:
			return -EINVAL;
	}
	ret = getsock(thread, fd, &sock);
	if (ret < 0)
		return ret;
	ret = sock_shutdown(sock, how);
	sock_free(sock);
	if (ret < 0)
		return ret;
	return 0;
}

ssize_t sys_fsync(int fd)
{
	struct thread *thread = curcpu()->thread;
	struct file *file;
	ssize_t ret;

	ret = proc_getfile(thread->proc, fd, &file);
	if (ret < 0)
		return ret;
	/* XXX */
	file_free(file);
	return 0;
}

ssize_t sys_fdatasync(int fd)
{
	struct thread *thread = curcpu()->thread;
	struct file *file;
	ssize_t ret;

	ret = proc_getfile(thread->proc, fd, &file);
	if (ret < 0)
		return ret;
	/* XXX */
	file_free(file);
	return 0;
}

ssize_t sys_getrusage(int who, struct rusage *urusage)
{
	struct thread *thread = curcpu()->thread;
	struct rusage rusage;
	struct procstat *stats;

	switch (who)
	{
		case RUSAGE_SELF:
			stats = &thread->proc->stats;
			break;
		case RUSAGE_CHILDREN:
			stats = &thread->proc->cstats;
			break;
		case RUSAGE_THREAD:
			stats = &thread->stats;
			break;
		default:
			return -EINVAL;
	}
	rusage_from_stats(&rusage, stats);
	return vm_copyout(thread->proc->vm_space, urusage, &rusage,
	                  sizeof(rusage));
}

ssize_t sys_sigreturn()
{
	struct thread *thread = curcpu()->thread;
	uintptr_t sp = arch_get_stack_pointer(&thread->tf_user);
	int onstack;
	if (thread->sigaltstack_nest
	 && sp >= (uintptr_t)thread->sigaltstack.ss_sp
	 && sp < (uintptr_t)thread->sigaltstack.ss_sp
	                  + thread->sigaltstack.ss_size)
		onstack = 1;
	else
		onstack = 0;
	if (ARCH_REGISTER_PARAMETERS < 3)
		sp += sizeof(size_t) * (3 - ARCH_REGISTER_PARAMETERS);
	size_t map_base = sp & ~PAGE_MASK;
	size_t map_size = PAGE_SIZE;
	if (PAGE_SIZE - (sp & PAGE_MASK) < sizeof(struct trapframe)
	                                 + sizeof(thread->sigmask))
		map_size += PAGE_SIZE;
	void *ptr;
	ssize_t ret = vm_map_user(thread->proc->vm_space, map_base,
	                      map_size, VM_PROT_R, &ptr);
	if (ret < 0)
		return ret;
	struct trapframe *src = (struct trapframe*)((uint8_t*)ptr + (sp - map_base));
	ret = arch_validate_user_trapframe(src);
	if (ret)
	{
		vm_unmap(ptr, map_size);
		return ret;
	}
	memcpy(&thread->tf_user, src, sizeof(*src));
	thread->sigmask = le64dec(&src[1]);
	thread->sigmask &= ~(1 << SIGSTOP);
	thread->sigmask &= ~(1 << SIGKILL);
	vm_unmap(ptr, map_size);
	ret = arch_get_syscall_retval(&thread->tf_user);
	/* XXX get a better way to handle SS_ONSTACK */
	if (!ret && onstack)
	{
		if (!--thread->sigaltstack_nest)
			thread->sigaltstack.ss_flags &= ~SS_ONSTACK;
	}
	return ret;
}

ssize_t sys_gettid()
{
	return curcpu()->thread->tid;
}

ssize_t sys_settls(void *uaddr)
{
	/* don't bother checking addr, paging will fault if invalid address
	 * is given
	 */
	curcpu()->thread->tls_addr = (uintptr_t)uaddr;
	return 0;
}

void *sys_gettls(void)
{
	return (void*)curcpu()->thread->tls_addr;
}

ssize_t sys_exit_group(int code)
{
	struct thread *oldthread = curcpu()->thread;
	proc_exit(oldthread->proc, (code & 0xFF) << 8);
	return arch_get_syscall_retval(&curcpu()->thread->tf_user);
}

ssize_t sys_futex(int *uaddr, int op, int val,
                  const struct timespec *utimeout)
{
	struct thread *thread = curcpu()->thread;
	struct timespec timeout;
	ssize_t ret;
	int flags = 0;

	if (op & FUTEX_PRIVATE_FLAG)
	{
		flags |= FUTEX_PRIVATE_FLAG;
		op &= ~FUTEX_PRIVATE_FLAG;
	}
	if (op & FUTEX_CLOCK_REALTIME)
	{
		flags |= FUTEX_CLOCK_REALTIME;
		op &= ~FUTEX_CLOCK_REALTIME;
	}
	if (utimeout)
	{
		ret = vm_copyin(thread->proc->vm_space, &timeout, utimeout,
		                sizeof(timeout));
		if (ret < 0)
			return ret;
		ret = timespec_validate(&timeout);
		if (ret < 0)
			return ret;
	}
	if (!(flags & FUTEX_PRIVATE_FLAG)) /* XXX */
		return -EINVAL;
	switch (op)
	{
		case FUTEX_WAIT:
		{
			int cur; /* XXX should be using vmap_user + atomic load
			          * instead of vm_copyin */
			ret = vm_copyin(thread->proc->vm_space, &cur, uaddr,
			                sizeof(cur));
			if (ret < 0)
				return ret;
			if (cur != val)
				return -EAGAIN;
			if ((flags & FUTEX_CLOCK_REALTIME) && utimeout)
			{
				struct timespec current_time;
				ret = clock_gettime(CLOCK_REALTIME, &current_time);
				if (ret)
					return ret;
				if (timespec_cmp(&timeout, &current_time) < 0)
					return -EWOULDBLOCK;
				struct timespec diff;
				timespec_sub(&diff, &timeout, &current_time);
				timeout = diff;
			}
			thread->futex_addr = uaddr;
			ret = thread_sleep(utimeout ? &timeout : NULL);
			thread->futex_addr = NULL;
			return ret;
		}
		case FUTEX_WAKE:
		{
			if (flags & FUTEX_CLOCK_REALTIME)
				return -EINVAL;
			if (val <= 0)
				return 0;
			struct thread *other;
			TAILQ_FOREACH(other, &thread->proc->threads, thread_chain)
			{
				if (!other->waitq)
					continue;
				if (other->futex_addr != uaddr)
					continue;
				waitq_wakeup_thread(other->waitq, other, 0);
				if (!--val)
					break;
			}
			return 0;
		}
		default:
			return -EINVAL;
	}
}

ssize_t sys_times(struct tms *utms, clock_t *uclk)
{
	struct thread *thread = curcpu()->thread;
	ssize_t ret;

	if (utms)
	{
		struct tms tms;
		tms.tms_utime = thread->proc->stats.utime.tv_sec * 1000000
		              + thread->proc->stats.utime.tv_nsec / 1000;
		tms.tms_stime = thread->proc->stats.stime.tv_sec * 1000000
		              + thread->proc->stats.stime.tv_nsec / 1000;
		tms.tms_cutime = thread->proc->cstats.utime.tv_sec * 1000000
		               + thread->proc->cstats.utime.tv_nsec / 1000;
		tms.tms_cstime = thread->proc->cstats.stime.tv_sec * 1000000
		               + thread->proc->cstats.stime.tv_nsec / 1000;
		ret = vm_copyout(thread->proc->vm_space, utms, &tms, sizeof(tms));
		if (ret < 0)
			return ret;
	}
	if (uclk)
	{
		struct timespec ts;
		ret = clock_gettime(CLOCK_MONOTONIC, &ts);
		if (ret)
			return ret;
		clock_t clk;
		clk = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
		ret = vm_copyout(thread->proc->vm_space, uclk, &clk, sizeof(*uclk));
		if (ret < 0)
			return ret;
	}
	return 0;
}

ssize_t sys_chroot(const char *upathname)
{
	struct thread *thread = curcpu()->thread;
	char pathname[MAXPATHLEN];
	struct node *node;
	ssize_t ret;

	ret = vm_copystr(thread->proc->vm_space, pathname, upathname,
	                 sizeof(pathname));
	if (ret < 0)
		return ret;
	ret = vfs_getnode(NULL, pathname, 0, &node);
	if (ret < 0)
		return ret;
	if (!S_ISDIR(node->attr.mode))
	{
		node_free(node);
		return -ENOTDIR;
	}
	node_free(thread->proc->root);
	thread->proc->root = node;
	return 0;
}

ssize_t sys_sigsuspend(const sigset_t *uset)
{
	struct thread *thread = curcpu()->thread;
	sigset_t set;
	uint64_t oldmask;
	uint64_t mask;
	ssize_t ret;

	if (!uset)
		return thread_sleep(NULL);
	ret = vm_copyin(thread->proc->vm_space, &set, uset, sizeof(set));
	if (ret < 0)
		return ret;
	mask  = (uint64_t)set.set[0] << 0;
	mask |= (uint64_t)set.set[1] << 8;
	mask |= (uint64_t)set.set[2] << 16;
	mask |= (uint64_t)set.set[3] << 24;
	mask |= (uint64_t)set.set[4] << 32;
	mask |= (uint64_t)set.set[5] << 40;
	mask |= (uint64_t)set.set[6] << 48;
	mask |= (uint64_t)set.set[7] << 56;
	mask &= ~(1 << SIGKILL);
	mask &= ~(1 << SIGSTOP);
	oldmask = thread->sigmask;
	thread->sigmask = mask;
	ret = thread_sleep(NULL);
	thread->sigmask = oldmask;
	return ret;
}

ssize_t sys_sigaltstack(const stack_t *uss, stack_t *uold_ss)
{
	struct thread *thread = curcpu()->thread;
	stack_t ss;
	ssize_t ret;

	if (uold_ss)
	{
		ret = vm_copyout(thread->proc->vm_space, uold_ss,
		                 &thread->sigaltstack, sizeof(*uold_ss));
		if (ret < 0)
			return ret;
	}
	if (uss)
	{
		if (thread->sigaltstack.ss_flags & SS_ONSTACK)
			return -EPERM;
		ret = vm_copyin(thread->proc->vm_space, &ss,
		                uss, sizeof(ss));
		if (ret < 0)
			return ret;
		if (ss.ss_size < MINSIGSTKSZ)
			return -EINVAL;
		if (ss.ss_flags)
			return -EINVAL;
		thread->sigaltstack = ss;
	}
	return 0;
}

ssize_t sys_madvise(void *uaddr, size_t len, int advise)
{
	(void)uaddr;
	(void)len;
	(void)advise;
	/* XXX */
	return 0;
}

ssize_t sys_getrlimit(int res, struct rlimit *ulimit)
{
	(void)res;
	(void)ulimit;
	/* XXX */
	return -ENOSYS;
}

ssize_t sys_setrlimit(int res, const struct rlimit *ulimit)
{
	(void)res;
	(void)ulimit;
	/* XXX */
	return -ENOSYS;
}

ssize_t sys_socketpair(int domain, int type, int protocol, int *ufds)
{
	struct thread *thread = curcpu()->thread;
	struct sock *socks[2] = {NULL, NULL};
	struct file *files[2] = {NULL, NULL};
	int fds[2] = {-1, -1};
	ssize_t ret;

	ret = sock_openpair(domain, type, protocol, socks);
	if (ret < 0)
		goto end;
	ret = file_fromsock(socks[0], 0, &files[0]);
	if (ret < 0)
		goto end;
	ret = file_fromsock(socks[1], 0, &files[1]);
	if (ret < 0)
		goto end;
	ret = proc_allocfd(thread->proc, files[0], 0);
	if (ret < 0)
		goto end;
	fds[0] = ret;
	ret = proc_allocfd(thread->proc, files[1], 0);
	if (ret < 0)
		goto end;
	fds[1] = ret;
	ret = vm_copyout(thread->proc->vm_space, ufds, fds, sizeof(fds));

end:
	for (size_t i = 0; i < 2; ++i)
	{
		if (socks[i])
			sock_free(socks[i]);
		if (files[i])
			file_free(files[i]);
		if (ret && fds[i] > 0)
			proc_freefd(thread->proc, fds[i]);
	}
	return ret;
}

ssize_t sys_sigpending(sigset_t *uset)
{
	struct thread *thread = curcpu()->thread;
	sigset_t set;

	le64enc(&set, thread->sigqueue);
	return vm_copyout(thread->proc->vm_space, uset, &set, sizeof(set));
}

/* XXX there's some weird bug? here that make GCC produce invalid riscv code
 * for REBOOT_REBOOT comparison when cmd is a 32 bit integer
 * make it a 64 bit integer for now as a workaround
 */
ssize_t sys_reboot(uintptr_t cmd)
{
	switch (cmd)
	{
		case REBOOT_SHUTDOWN:
			return arch_shutdown();
		case REBOOT_REBOOT:
			return arch_reboot();
		case REBOOT_SUSPEND:
			return arch_suspend();
		case REBOOT_HIBERNATE:
			return arch_hibernate();
		default:
			return -EINVAL;
	}
}

ssize_t sys_getitimer(int which, struct itimerval *ucur)
{
	struct thread *thread = curcpu()->thread;
	struct itimerval *itimer;

	switch (which)
	{
		case ITIMER_REAL:
			itimer = &thread->proc->itimer_real;
			break;
		case ITIMER_VIRTUAL:
			itimer = &thread->proc->itimer_virt;
			break;
		case ITIMER_PROF:
			itimer = &thread->proc->itimer_prof;
			break;
		default:
			return -EINVAL;
	}
	return vm_copyout(thread->proc->vm_space, ucur, itimer, sizeof(*ucur));
}

ssize_t sys_setitimer(int which, const struct itimerval *uval,
                      struct itimerval *uold)
{
	struct thread *thread = curcpu()->thread;
	struct itimerval *itimer;
	struct itimerval val;
	struct itimerval old;
	int ret;

	if (!uval)
		return -EFAULT;
	switch (which)
	{
		case ITIMER_REAL:
			itimer = &thread->proc->itimer_real;
			break;
		case ITIMER_VIRTUAL:
			itimer = &thread->proc->itimer_virt;
			break;
		case ITIMER_PROF:
			itimer = &thread->proc->itimer_prof;
			break;
		default:
			return -EINVAL;
	}
	ret = vm_copyin(thread->proc->vm_space, &val, uval, sizeof(val));
	if (ret)
		return ret;
	ret = timeval_validate(&val.it_interval);
	if (ret)
		return ret;
	ret = timeval_validate(&val.it_value);
	if (ret)
		return ret;
	old = *itimer;
	if (uold)
	{
		ret = vm_copyout(thread->proc->vm_space, uold, &old, sizeof(*uold));
		if (ret)
			return ret;
	}
	*itimer = val;
	proc_test_itimer(thread->proc, which);
	return 0;
}

static const struct
{
	const char *name;
	uintptr_t (*fn)();
} syscalls[] =
{
#define SYSCALL_DEF(name) [SYS_##name] = {#name, (void*)sys_##name}
	SYSCALL_DEF(exit),
	SYSCALL_DEF(clone),
	SYSCALL_DEF(readv),
	SYSCALL_DEF(writev),
	SYSCALL_DEF(openat),
	SYSCALL_DEF(close),
	SYSCALL_DEF(time),
	SYSCALL_DEF(getpid),
	SYSCALL_DEF(getuid),
	SYSCALL_DEF(getgid),
	SYSCALL_DEF(setuid),
	SYSCALL_DEF(setgid),
	SYSCALL_DEF(geteuid),
	SYSCALL_DEF(getegid),
	SYSCALL_DEF(setpgid),
	SYSCALL_DEF(getppid),
	SYSCALL_DEF(getpgrp),
	SYSCALL_DEF(setsid),
	SYSCALL_DEF(setreuid),
	SYSCALL_DEF(setregid),
	SYSCALL_DEF(getgroups),
	SYSCALL_DEF(setgroups),
	SYSCALL_DEF(getpgid),
	SYSCALL_DEF(ioctl),
	SYSCALL_DEF(fstatat),
	SYSCALL_DEF(getdents),
	SYSCALL_DEF(execveat),
	SYSCALL_DEF(lseek),
	SYSCALL_DEF(mmap),
	SYSCALL_DEF(munmap),
	SYSCALL_DEF(readlinkat),
	SYSCALL_DEF(dup),
	SYSCALL_DEF(dup3),
	SYSCALL_DEF(wait4),
	SYSCALL_DEF(kill),
	SYSCALL_DEF(getsid),
	SYSCALL_DEF(socket),
	SYSCALL_DEF(bind),
	SYSCALL_DEF(accept),
	SYSCALL_DEF(connect),
	SYSCALL_DEF(listen),
	SYSCALL_DEF(mprotect),
	SYSCALL_DEF(msync),
	SYSCALL_DEF(unlinkat),
	SYSCALL_DEF(clock_gettime),
	SYSCALL_DEF(clock_getres),
	SYSCALL_DEF(clock_settime),
	SYSCALL_DEF(pipe2),
	SYSCALL_DEF(utimensat),
	SYSCALL_DEF(nanosleep),
	SYSCALL_DEF(sigaction),
	SYSCALL_DEF(mknodat),
	SYSCALL_DEF(chdir),
	SYSCALL_DEF(fchdir),
	SYSCALL_DEF(fchmodat),
	SYSCALL_DEF(fchownat),
	SYSCALL_DEF(linkat),
	SYSCALL_DEF(faccessat),
	SYSCALL_DEF(umask),
	SYSCALL_DEF(symlinkat),
	SYSCALL_DEF(getpagesize),
	SYSCALL_DEF(renameat),
	SYSCALL_DEF(uname),
	SYSCALL_DEF(mount),
	SYSCALL_DEF(sendmsg),
	SYSCALL_DEF(recvmsg),
	SYSCALL_DEF(fstatvfsat),
	SYSCALL_DEF(ftruncateat),
	SYSCALL_DEF(fcntl),
	SYSCALL_DEF(sched_yield),
	SYSCALL_DEF(ptrace),
	SYSCALL_DEF(sigprocmask),
	SYSCALL_DEF(kmload),
	SYSCALL_DEF(kmunload),
	SYSCALL_DEF(shmget),
	SYSCALL_DEF(shmat),
	SYSCALL_DEF(shmdt),
	SYSCALL_DEF(shmctl),
	SYSCALL_DEF(semget),
	SYSCALL_DEF(semtimedop),
	SYSCALL_DEF(semctl),
	SYSCALL_DEF(msgget),
	SYSCALL_DEF(msgsnd),
	SYSCALL_DEF(msgrcv),
	SYSCALL_DEF(msgctl),
	SYSCALL_DEF(getpriority),
	SYSCALL_DEF(setpriority),
	SYSCALL_DEF(pselect),
	SYSCALL_DEF(ppoll),
	SYSCALL_DEF(getsockopt),
	SYSCALL_DEF(setsockopt),
	SYSCALL_DEF(getpeername),
	SYSCALL_DEF(getsockname),
	SYSCALL_DEF(shutdown),
	SYSCALL_DEF(fsync),
	SYSCALL_DEF(fdatasync),
	SYSCALL_DEF(getrusage),
	SYSCALL_DEF(sigreturn),
	SYSCALL_DEF(gettid),
	SYSCALL_DEF(settls),
	SYSCALL_DEF(gettls),
	SYSCALL_DEF(exit_group),
	SYSCALL_DEF(futex),
	SYSCALL_DEF(times),
	SYSCALL_DEF(chroot),
	SYSCALL_DEF(sigsuspend),
	SYSCALL_DEF(sigaltstack),
	SYSCALL_DEF(madvise),
	SYSCALL_DEF(getrlimit),
	SYSCALL_DEF(setrlimit),
	SYSCALL_DEF(socketpair),
	SYSCALL_DEF(sigpending),
	SYSCALL_DEF(reboot),
	SYSCALL_DEF(getitimer),
	SYSCALL_DEF(setitimer),
#undef SYSCALL_DEF
};

uintptr_t syscall(uintptr_t id, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3,
                  uintptr_t arg4, uintptr_t arg5, uintptr_t arg6)
{
	struct thread *thread = curcpu()->thread;
	int was_ptraced;
	if (thread->ptrace_state == PTRACE_ST_SYSCALL)
	{
		int signum = SIGTRAP;
		if (thread->ptrace_options & PTRACE_O_TRACESYSGOOD)
			signum |= 0x80;
		thread_ptrace_stop(thread, signum);
		was_ptraced = 1;
	}
	else
	{
		was_ptraced = 0;
	}
#if DEBUG_SYSCALL == 1
#if DEBUG_SYSCALL_VMSIZE == 1
	uintptr_t vm_avail_start = vm_available_size();
#endif
#if DEBUG_SYSCALL_PREFIX == 1
	printf("[%p PID %" PRId32 " TID %" PRId32 "] syscall %zd (%s) from proc %s\n",
	       thread, thread->proc->pid, thread->tid, id,
	       id < sizeof(syscalls) / sizeof(*syscalls) && syscalls[id].name
	     ? syscalls[id].name : "",
	       thread->proc->name);
#endif
#if DEBUG_SYSCALL_TIME == 1
	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC, &start);
#endif
#endif
	uintptr_t ret;
	if (id < sizeof(syscalls) / sizeof(*syscalls)
	 && syscalls[id].fn)
		ret = syscalls[id].fn(arg1, arg2, arg3, arg4, arg5, arg6);
	else
		ret = -ENOSYS;

#if DEBUG_SYSCALL == 1
#if DEBUG_SYSCALL_TIME == 1
	struct timespec end;
	clock_gettime(CLOCK_MONOTONIC, &end);
	struct timespec diff;
	timespec_sub(&diff, &end, &start);
#endif
#if DEBUG_SYSCALL_VMSIZE == 1
	uintptr_t vm_avail_end = vm_available_size();
#endif
	printf("[%p PID %" PRId32 " TID %" PRId32 " %s] syscall %zd (%s) returned 0x%zx"
#if DEBUG_SYSCALL_TIME == 1
	" in %" PRIu64 ".%03" PRId64 "us"
#endif
#if DEBUG_SYSCALL_VMSIZE == 1
	" allocated 0x%lx bytes"
#endif
	"\n", thread, thread->proc->pid, thread->tid, thread->proc->name, id,
	id < sizeof(syscalls) / sizeof(*syscalls) && syscalls[id].name
	? syscalls[id].name : "", ret
#if DEBUG_SYSCALL_TIME == 1
	, diff.tv_sec * 1000000 + (diff.tv_nsec / 1000), diff.tv_nsec % 1000
#endif
#if DEBUG_SYSCALL_VMSIZE == 1
	, vm_avail_start - vm_avail_end
#endif
	);
#endif

	arch_set_syscall_retval(&thread->tf_user, ret);
	if (was_ptraced && thread->ptrace_state == PTRACE_ST_SYSCALL)
	{
		int signum = SIGTRAP;
		if (thread->ptrace_options & PTRACE_O_TRACESYSGOOD)
			signum |= 0x80;
		thread_ptrace_stop(thread, signum);
	}
	return ret;
}
