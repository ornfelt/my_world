#include <ramfile.h>
#include <mutex.h>
#include <errno.h>
#include <time.h>
#include <proc.h>
#include <file.h>
#include <ipc.h>
#include <std.h>
#include <uio.h>
#include <sma.h>
#include <vfs.h>
#include <cpu.h>
#include <mem.h>

#define IPC_STAT 1
#define IPC_SET  2
#define IPC_RMID 3

#define IPC_PRIVATE 0

#define IPC_CREAT   (1 << 9)
#define IPC_EXCL    (1 << 10)
#define IPC_NOWAIT  (1 << 11)
#define SHM_RND     (1 << 12)
#define SHM_RDONLY  (1 << 13)
#define SEM_UNDO    (1 << 14)
#define MSG_NOERROR (1 << 15)
#define MSG_EXCEPT  (1 << 16)

#define GETVAL  10
#define SETVAL  11
#define GETPID  12
#define GETNCNT 13
#define GETZCNT 14
#define GETALL  15
#define SETALL  16

#define SHMALL (SIZE_MAX / PAGE_SIZE) /* max system shm pages */ /* XXX to be used */
#define SHMMIN PAGE_SIZE /* minimum shm size */
#define SHMMAX (SIZE_MAX - PAGE_SIZE) /* maximum shm size */
#define SHMLBA PAGE_SIZE /* low boundary address */ /* XXX to be used */
#define SHMSEG (SIZE_MAX - PAGE_SIZE) /* max per-process segments */ /* XXX to be used */
#define SHMMNI 4096 /* max shm */

#define SEMOPM 256 /* max nsops per semop */
#define SEMMSL 256 /* max nsems per sem */
#define SEMMNI 4096 /* max sem */
#define SEMMNS 256 /* max nsems */ /* XXX to be used */
#define SEMVMX INT16_MAX /* max sem value */

#define MSGMNI 4096 /* max msg */
#define MSGMAX PAGE_SIZE /* max msg msg size */
#define MSGMNB PAGE_SIZE /* max msg total size */

struct ipc_perm
{
	key_t key;
	uid_t uid;
	gid_t gid;
	uid_t cuid;
	gid_t cgid;
	unsigned short mode;
	unsigned short seq;
};

struct shmid_ds
{
	struct ipc_perm shm_perm;
	size_t shm_segsz;
	time_t shm_atime;
	time_t shm_dtime;
	time_t shm_ctime;
	pid_t shm_cpid;
	pid_t shm_lpid;
	shmatt_t shm_nattch;
};

struct semid_ds
{
	struct ipc_perm sem_perm;
	time_t sem_otime;
	time_t sem_ctime;
	unsigned long sem_nsems;
};

struct msgid_ds
{
	struct ipc_perm msg_perm;
	time_t msg_stime;
	time_t msg_rtime;
	time_t msg_ctime;
	unsigned long msg_cbytes;
	msgqnum_t msg_qnum;
	msglen_t msg_qbytes;
	pid_t msg_lspid;
	pid_t msg_lrpid;
};

union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

struct sembuf
{
	unsigned short sem_num;
	short sem_op;
	short sem_flg;
};

struct semval
{
	uint16_t value;
	uint16_t ncnt;
	uint16_t zcnt;
	pid_t pid;
};

struct msgbuf
{
	TAILQ_ENTRY(msgbuf) chain;
	size_t msgsz;
	long mtype;
	char mtext[];
};

struct sysv_ipc
{
	TAILQ_ENTRY(sysv_ipc) chain;
	TAILQ_ENTRY(sysv_ipc) key_chain;
	TAILQ_ENTRY(sysv_ipc) list_chain;
	int id;
	struct mutex mutex;
	refcount_t refcount;
	int removed;
	struct ipc_perm perm;
};

struct sysv_shm
{
	TAILQ_ENTRY(sysv_shm) chain;
	TAILQ_ENTRY(sysv_shm) key_chain;
	TAILQ_ENTRY(sysv_shm) list_chain;
	int id;
	struct mutex mutex;
	refcount_t refcount;
	int removed;
	struct shmid_ds ds;
	struct ramfile ramfile;
};

struct sysv_sem
{
	TAILQ_ENTRY(sysv_sem) chain;
	TAILQ_ENTRY(sysv_sem) key_chain;
	TAILQ_ENTRY(sysv_sem) list_chain;
	int id;
	struct mutex mutex;
	refcount_t refcount;
	int removed;
	struct semid_ds ds;
	struct semval *values;
	struct waitq waitq;
};

struct sysv_msg
{
	TAILQ_ENTRY(sysv_msg) chain;
	TAILQ_ENTRY(sysv_msg) key_chain;
	TAILQ_ENTRY(sysv_msg) list_chain;
	int id;
	struct mutex mutex;
	refcount_t refcount;
	int removed;
	struct msgid_ds ds;
	TAILQ_HEAD(, msgbuf) msgs;
	struct waitq rwaitq;
	struct waitq wwaitq;
};

static void shm_vm_open(struct vm_zone *zone);
static void shm_vm_close(struct vm_zone *zone);
static int shm_vm_fault(struct vm_zone *zone, off_t off, struct page **page);

static struct vm_zone_op shm_vm_op =
{
	.open = shm_vm_open,
	.close = shm_vm_close,
	.fault = shm_vm_fault,
};

TAILQ_HEAD(ipc_head, sysv_ipc);
TAILQ_HEAD(shm_head, sysv_shm);
TAILQ_HEAD(sem_head, sysv_sem);
TAILQ_HEAD(msg_head, sysv_msg);

#define IPC_HTABLE_SIZE 512

static struct mutex shms_mutex; /* XXX rwlock */
static struct mutex sems_mutex; /* XXX rwlock */
static struct mutex msgs_mutex; /* XXX rwlock */
static unsigned shm_seq;
static unsigned sem_seq;
static unsigned msg_seq;
static struct shm_head shms[IPC_HTABLE_SIZE];
static struct sem_head sems[IPC_HTABLE_SIZE];
static struct msg_head msgs[IPC_HTABLE_SIZE];
static struct shm_head shms_keys[IPC_HTABLE_SIZE];
static struct sem_head sems_keys[IPC_HTABLE_SIZE];
static struct msg_head msgs_keys[IPC_HTABLE_SIZE];
static struct shm_head shms_list;
static struct sem_head sems_list;
static struct msg_head msgs_list;
static uint32_t shms_count;
static uint32_t sems_count;
static uint32_t msgs_count;
static struct sma shm_sma;
static struct sma sem_sma;
static struct sma msg_sma;

#define SHM_HEAD(id) (&shms[(id) % IPC_HTABLE_SIZE])
#define SEM_HEAD(id) (&sems[(id) % IPC_HTABLE_SIZE])
#define MSG_HEAD(id) (&msgs[(id) % IPC_HTABLE_SIZE])

#define SHM_KEY_HEAD(key) (&shms_keys[(key) % IPC_HTABLE_SIZE])
#define SEM_KEY_HEAD(key) (&sems_keys[(key) % IPC_HTABLE_SIZE])
#define MSG_KEY_HEAD(key) (&msgs_keys[(key) % IPC_HTABLE_SIZE])

void sysv_ipc_init(void)
{
	sma_init(&shm_sma, sizeof(struct sysv_shm), NULL, NULL, "sysv_shm");
	sma_init(&sem_sma, sizeof(struct sysv_sem), NULL, NULL, "sysv_sem");
	sma_init(&msg_sma, sizeof(struct sysv_msg), NULL, NULL, "sysv_msg");
}

static int ipc_hasperm(struct ipc_perm *ipc_perm, struct cred *cred,
                       mode_t perm)
{
	if (!cred->euid)
		return 0;
	uint32_t mode = ipc_perm->mode & 7;
	if (ipc_perm->uid == cred->euid)
		mode |= (ipc_perm->mode >> 6) & 7;
	if (ipc_perm->gid == cred->egid)
		mode |= (ipc_perm->mode >> 3) & 7;
	if ((mode & perm) != perm)
		return -EACCES;
	return 0;
}

static int ipc_isowner(struct ipc_perm *ipc_perm, struct cred *cred)
{
	if (cred->euid
	 && cred->euid != ipc_perm->uid
	 && cred->euid != ipc_perm->cuid)
		return -EPERM;
	return 0;
}

static ssize_t shmlist_read(struct file *file, struct uio *uio)
{
	(void)file;
	struct thread *thread = curcpu()->thread;
	size_t count = uio->count;
	off_t off = uio->off;
	mutex_lock(&shms_mutex);
	struct sysv_shm *shm;
	TAILQ_FOREACH(shm, &shms_list, list_chain)
	{
		if (!ipc_hasperm(&shm->ds.shm_perm,
		                 &thread->proc->cred,
		                 04))
			uprintf(uio, "%d\n", shm->id);
	}
	mutex_unlock(&shms_mutex);
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op shmlist_fop =
{
	.read = shmlist_read,
};

static ssize_t semlist_read(struct file *file, struct uio *uio)
{
	(void)file;
	struct thread *thread = curcpu()->thread;
	size_t count = uio->count;
	off_t off = uio->off;
	mutex_lock(&sems_mutex);
	struct sysv_sem *sem;
	TAILQ_FOREACH(sem, &sems_list, list_chain)
	{
		if (!ipc_hasperm(&sem->ds.sem_perm,
		                 &thread->proc->cred,
		                 04))
			uprintf(uio, "%d\n", sem->id);
	}
	mutex_unlock(&sems_mutex);
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op semlist_fop =
{
	.read = semlist_read,
};

static ssize_t msglist_read(struct file *file, struct uio *uio)
{
	(void)file;
	struct thread *thread = curcpu()->thread;
	size_t count = uio->count;
	off_t off = uio->off;
	mutex_lock(&msgs_mutex);
	struct sysv_msg *msg;
	TAILQ_FOREACH(msg, &msgs_list, list_chain)
	{
		if (!ipc_hasperm(&msg->ds.msg_perm,
		                 &thread->proc->cred,
		                 04))
			uprintf(uio, "%d\n", msg->id);
	}
	mutex_unlock(&msgs_mutex);
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op msglist_fop =
{
	.read = msglist_read,
};

static ssize_t limits_read(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
#define PRINT_LIMIT(name) uprintf(uio, #name " %lu\n", (unsigned long)name)
	PRINT_LIMIT(SHMALL);
	PRINT_LIMIT(SHMMIN);
	PRINT_LIMIT(SHMMAX);
	PRINT_LIMIT(SHMLBA);
	PRINT_LIMIT(SHMSEG);
	PRINT_LIMIT(SHMMNI);
	PRINT_LIMIT(SEMOPM);
	PRINT_LIMIT(SEMMSL);
	PRINT_LIMIT(SEMMNI);
	PRINT_LIMIT(SEMMNS);
	PRINT_LIMIT(SEMVMX);
	PRINT_LIMIT(MSGMNI);
	PRINT_LIMIT(MSGMAX);
	PRINT_LIMIT(MSGMNB);
#undef PRINT_LIMIT
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op limits_fop =
{
	.read = limits_read,
};

int ipc_init(void)
{
	mutex_init(&shms_mutex, 0);
	mutex_init(&sems_mutex, 0);
	mutex_init(&msgs_mutex, 0);
	for (size_t i = 0; i < IPC_HTABLE_SIZE; ++i)
		TAILQ_INIT(&shms[i]);
	for (size_t i = 0; i < IPC_HTABLE_SIZE; ++i)
		TAILQ_INIT(&sems[i]);
	for (size_t i = 0; i < IPC_HTABLE_SIZE; ++i)
		TAILQ_INIT(&msgs[i]);
	for (size_t i = 0; i < IPC_HTABLE_SIZE; ++i)
		TAILQ_INIT(&shms_keys[i]);
	for (size_t i = 0; i < IPC_HTABLE_SIZE; ++i)
		TAILQ_INIT(&sems_keys[i]);
	for (size_t i = 0; i < IPC_HTABLE_SIZE; ++i)
		TAILQ_INIT(&msgs_keys[i]);
	TAILQ_INIT(&shms_list);
	TAILQ_INIT(&sems_list);
	TAILQ_INIT(&msgs_list);
	if (sysfs_mknode("sysv/shmlist", 0, 0, 0444, &shmlist_fop, NULL))
		panic("failed to create sysv/shmlist\n");
	if (sysfs_mknode("sysv/semlist", 0, 0, 0444, &semlist_fop, NULL))
		panic("failed to create sysv/semlist\n");
	if (sysfs_mknode("sysv/msglist", 0, 0, 0444, &msglist_fop, NULL))
		panic("failed to create sysv/msglist\n");
	if (sysfs_mknode("sysv/limits", 0, 0, 0444, &limits_fop, NULL))
		panic("failed to create sysv/limits\n");
	return 0;
}

static struct sysv_ipc *getipc_locked(struct ipc_head *head, int id)
{
	struct sysv_ipc *ipc;
	TAILQ_FOREACH(ipc, head, chain)
	{
		if (ipc->id != id)
			continue;
		refcount_inc(&ipc->refcount);
		return ipc;
	}
	return NULL;
}

static struct sysv_ipc *getipc(struct ipc_head *head, struct mutex *mutex,
                               int id)
{
	mutex_lock(mutex);
	struct sysv_ipc *ipc = getipc_locked(head, id);
	mutex_unlock(mutex);
	return ipc;
}

static struct sysv_shm *getshm(int id)
{
	return (struct sysv_shm*)getipc((struct ipc_head*)SHM_HEAD(id),
	                                &shms_mutex, id);
}

static struct sysv_sem *getsem(int id)
{
	return (struct sysv_sem*)getipc((struct ipc_head*)SEM_HEAD(id),
	                                &sems_mutex, id);
}

static struct sysv_msg *getmsg(int id)
{
	return (struct sysv_msg*)getipc((struct ipc_head*)MSG_HEAD(id),
	                                &msgs_mutex, id);
}

static unsigned ipc_hash(unsigned id)
{
	id = ((id >> 16) ^ id) * 0x45D9F3B;
	id = ((id >> 16) ^ id) * 0x45D9F3B;
	return ((id >> 16) ^ id) & 0x7FFFFFFF;
}

static int alloc_ipcid(struct ipc_head *ipc_head, unsigned seq)
{
	while (1)
	{
		seq = ipc_hash(seq);
		struct sysv_ipc *ipc = getipc_locked(&ipc_head[seq % IPC_HTABLE_SIZE],
		                                     seq);
		if (!ipc)
			return seq;
	}
}

static int shm_alloc(key_t key, size_t size, int flags, struct sysv_shm **shmp)
{
	if (!size || size < SHMMIN || size > SHMMAX)
		return -EINVAL;
	if (size < SHMMIN || size > SHMMAX)
		return -EINVAL;
	mutex_lock(&shms_mutex);
	int ret;
	if (shms_count >= SHMMNI)
	{
		ret = -ENOSPC;
		goto end;
	}
	struct thread *thread = curcpu()->thread;
	struct sysv_shm *shm = sma_alloc(&shm_sma, 0);
	if (!shm)
	{
		ret = -ENOMEM;
		goto end;
	}
	ramfile_init(&shm->ramfile);
	shm->ds.shm_perm.seq = ++shm_seq;
	shm->id = alloc_ipcid((struct ipc_head*)shms, shm->ds.shm_perm.seq);
	shm->removed = 0;
	shm->ds.shm_perm.key = key;
	shm->ds.shm_perm.uid = thread->proc->cred.euid;
	shm->ds.shm_perm.gid = thread->proc->cred.egid;
	shm->ds.shm_perm.cuid = thread->proc->cred.euid;
	shm->ds.shm_perm.cgid = thread->proc->cred.egid;
	shm->ds.shm_perm.mode = flags & 0777;
	shm->ds.shm_segsz = size;
	shm->ds.shm_atime = 0;
	shm->ds.shm_dtime = 0;
	shm->ds.shm_ctime = realtime_seconds();
	shm->ds.shm_cpid = thread->proc->pid;
	shm->ds.shm_lpid = 0;
	shm->ds.shm_nattch = 0;
	mutex_init(&shm->mutex, 0);
	refcount_init(&shm->refcount, 1);
	TAILQ_INSERT_TAIL(SHM_HEAD(shm->id), shm, chain);
	if (key)
		TAILQ_INSERT_TAIL(SHM_KEY_HEAD(key), shm, key_chain);
	TAILQ_INSERT_TAIL(&shms_list, shm, list_chain);
	shms_count++;
	*shmp = shm;
	ret = 0;

end:
	mutex_unlock(&shms_mutex);
	return ret;
}

static void shm_free(struct sysv_shm *shm)
{
	ramfile_destroy(&shm->ramfile);
	mutex_destroy(&shm->mutex);
	TAILQ_REMOVE(SHM_HEAD(shm->id), shm, chain);
	if (shm->ds.shm_perm.key)
		TAILQ_REMOVE(SHM_KEY_HEAD(shm->ds.shm_perm.key), shm, key_chain);
	TAILQ_REMOVE(&shms_list, shm, list_chain);
	sma_free(&shm_sma, shm);
	shms_count--;
}

static void shm_decref(struct sysv_shm *shm)
{
	mutex_lock(&shms_mutex);
	if (!refcount_dec(&shm->refcount) && shm->removed && !shm->ds.shm_nattch)
		shm_free(shm);
	mutex_unlock(&shms_mutex);
}

int sys_shmget(key_t key, size_t size, int flags)
{
	size = (size + PAGE_MASK) & ~PAGE_MASK;
	if (flags & ~(IPC_CREAT | IPC_EXCL | 0777))
		return -EINVAL;
	struct sysv_shm *shm = NULL;
	int ret;
	if (key == IPC_PRIVATE)
	{
		ret = shm_alloc(key, size, flags, &shm);
		if (ret)
			goto end;
		ret = shm->id;
		goto end;
	}
	mutex_lock(&shms_mutex);
	struct shm_head *head = SHM_KEY_HEAD(key);
	TAILQ_FOREACH(shm, head, key_chain)
	{
		if (shm->ds.shm_perm.key == key)
		{
			refcount_inc(&shm->refcount);
			break;
		}
	}
	mutex_unlock(&shms_mutex);
	if (!shm)
	{
		if (!(flags & IPC_CREAT))
		{
			ret = -ENOENT;
			goto end;
		}
		ret = shm_alloc(key, size, flags, &shm);
		if (ret)
			goto end;
		ret = shm->id;
		goto end;
	}
	if ((flags & (IPC_CREAT | IPC_EXCL)) == (IPC_CREAT | IPC_EXCL))
	{
		ret = -EEXIST;
		goto end;
	}
	struct thread *thread = curcpu()->thread;
	mutex_lock(&shm->mutex);
	ret = ipc_hasperm(&shm->ds.shm_perm,
	                  &thread->proc->cred,
	                  04);
	mutex_unlock(&shm->mutex);
	if (ret)
		goto end;
	ret = shm->id;

end:
	if (shm)
		shm_decref(shm);
	return ret;
}

void *sys_shmat(int shmid, const void *shmaddr, int flags)
{
	if (flags & ~(SHM_RDONLY | SHM_RND))
		return (void*)-EINVAL;
	struct sysv_shm *shm = getshm(shmid);
	if (!shm)
		return (void*)-EINVAL;
	intptr_t ret;
	mutex_lock(&shm->mutex);
	/* XXX POSIX specified -IDRM if deleted, linux doesn't */
#if 0
	if (shm->removed)
	{
		ret = -EIDRM;
		goto end;
	}
#endif
	if (shmaddr)
	{
		/* XXX */
		ret = -EINVAL;
		goto end;
	}
	struct thread *thread = curcpu()->thread;
	int kprot = VM_PROT_R;
	if (!(flags & SHM_RDONLY))
		kprot |= VM_PROT_W;
	struct vm_space *vm_space = thread->proc->vm_space;
	mutex_lock(&vm_space->mutex);
	struct vm_zone *zone;
	ret = vm_alloc(vm_space, (uintptr_t)shmaddr, 0,
	               shm->ds.shm_segsz, 0, kprot, 0, NULL, &zone);
	if (ret)
	{
		mutex_unlock(&vm_space->mutex);
		goto end;
	}
	zone->op = &shm_vm_op;
	zone->userdata = shm;
	struct vm_shm *vm_shm;
	ret = vm_shm_alloc(zone->addr, zone->size, shm->id, &vm_shm);
	if (ret)
	{
		vm_free(vm_space, zone->addr, zone->size);
		mutex_unlock(&vm_space->mutex);
		goto end;
	}
	TAILQ_INSERT_TAIL(&vm_space->shms, vm_shm, chain);
	shm->ds.shm_atime = realtime_seconds();
	shm->ds.shm_lpid = thread->proc->pid;
	shm->ds.shm_nattch++;
	ret = zone->addr;
	mutex_unlock(&vm_space->mutex);

end:
	if (shm)
	{
		mutex_unlock(&shm->mutex);
		shm_decref(shm);
	}
	return (void*)ret;
}

int sys_shmdt(const void *shmaddr)
{
	if ((uintptr_t)shmaddr & PAGE_MASK)
		return -EINVAL;
	struct thread *thread = curcpu()->thread;
	struct vm_space *vm_space = thread->proc->vm_space;
	int id = -1;
	int ret;
	struct sysv_shm *shm = NULL;
	struct vm_shm *vm_shm;
	ret = vm_shm_find(vm_space, (uintptr_t)shmaddr, &vm_shm);
	if (ret)
		goto end;
	id = vm_shm->shm;
	if (id == -1)
	{
		ret = -EINVAL;
		goto end;
	}
	shm = getshm(id);
	if (!shm)
	{
		/* XXX shouldn't happen */
		ret = -EINVAL;
		goto end;
	}
	mutex_lock(&shm->mutex);
	if (!shm->ds.shm_nattch)
	{
		/* XXX shouldn't happen */
		ret = -EINVAL;
		goto end;
	}
	mutex_unlock(&shm->mutex);
	/* XXX filter zone by userdata == shm */
	ret = vm_free(vm_space, vm_shm->addr, vm_shm->size);
	mutex_lock(&shm->mutex);
	if (ret)
		goto end;
	shm->ds.shm_dtime = realtime_seconds();
	shm->ds.shm_lpid = thread->proc->pid;
	ret = 0;

end:
	if (shm)
	{
		mutex_unlock(&shm->mutex);
		shm_decref(shm);
	}
	return ret;
}

int sys_shmctl(int shmid, int cmd, struct shmid_ds *ubuf)
{
	struct sysv_shm *shm = getshm(shmid);
	if (!shm)
		return -EINVAL;
	struct thread *thread = curcpu()->thread;
	int ret;
	mutex_lock(&shm->mutex);
	if (shm->removed)
	{
		ret = -EIDRM;
		goto end;
	}
	switch (cmd)
	{
		case IPC_STAT:
		{
			ret = ipc_hasperm(&shm->ds.shm_perm,
			                  &thread->proc->cred,
			                  04);
			if (ret)
				goto end;
			ret = vm_copyout(thread->proc->vm_space, ubuf,
			                 &shm->ds, sizeof(*ubuf));
			break;
		}
		case IPC_SET:
		{
			ret = ipc_isowner(&shm->ds.shm_perm, &thread->proc->cred);
			if (ret)
				goto end;
			struct shmid_ds buf;
			ret = vm_copyin(thread->proc->vm_space, &buf,
			                ubuf, sizeof(buf));
			if (ret)
				goto end;
			shm->ds.shm_perm.uid = buf.shm_perm.uid;
			shm->ds.shm_perm.gid = buf.shm_perm.gid;
			shm->ds.shm_perm.mode = (shm->ds.shm_perm.mode & ~0777)
			                      | (buf.shm_perm.uid & 0777);
			shm->ds.shm_ctime = realtime_seconds();
			ret = 0;
			break;
		}
		case IPC_RMID:
		{
			ret = ipc_isowner(&shm->ds.shm_perm, &thread->proc->cred);
			if (ret)
				goto end;
			shm->removed = 1;
			ret = 0;
			break;
		}
		default:
			ret = -EINVAL;
			break;
	}

end:
	if (shm)
	{
		mutex_unlock(&shm->mutex);
		shm_decref(shm);
	}
	return ret;
}

static void shm_vm_open(struct vm_zone *zone)
{
	struct sysv_shm *shm = zone->userdata;
	mutex_lock(&shm->mutex);
	shm->ds.shm_nattch++;
	mutex_unlock(&shm->mutex);
}

static void shm_vm_close(struct vm_zone *zone)
{
	struct sysv_shm *shm = zone->userdata;
	mutex_lock(&shm->mutex);
	shm->ds.shm_nattch--;
	mutex_unlock(&shm->mutex);
	mutex_lock(&shms_mutex);
	if (!refcount_get(&shm->refcount) && shm->removed && !shm->ds.shm_nattch)
		shm_free(shm);
	mutex_unlock(&shms_mutex);
}

static int shm_vm_fault(struct vm_zone *zone, off_t off, struct page **page)
{
	struct sysv_shm *shm = zone->userdata;
	mutex_lock(&shm->mutex);
	off /= PAGE_SIZE;
	int ret;
	if ((size_t)off >= shm->ds.shm_segsz)
	{
		ret = -EOVERFLOW;
		goto end;
	}
	*page = ramfile_getpage(&shm->ramfile, off, RAMFILE_ALLOC | RAMFILE_ZERO);
	if (!*page)
	{
		ret = -ENOMEM;
		goto end;
	}
	ret = 0;

end:
	mutex_unlock(&shm->mutex);
	return ret;
}

static int sem_alloc(key_t key, int nsems, int flags, struct sysv_sem **semp)
{
	int ret;
	mutex_lock(&sems_mutex);
	if (sems_count >= SEMMNI)
	{
		ret = -ENOSPC;
		goto end;
	}
	struct thread *thread = curcpu()->thread;
	struct sysv_sem *sem = sma_alloc(&sem_sma, 0);
	if (!sem)
	{
		ret = -ENOMEM;
		goto end;
	}
	struct semval *values = malloc(sizeof(*values) * nsems, M_ZERO);
	if (!values)
	{
		sma_free(&sem_sma, sem);
		ret = -ENOMEM;
		goto end;
	}
	sem->ds.sem_perm.seq = ++sem_seq;
	sem->id = alloc_ipcid((struct ipc_head*)sems, sem->ds.sem_perm.seq);
	sem->values = values;
	sem->removed = 0;
	sem->ds.sem_perm.key = key;
	sem->ds.sem_perm.uid = thread->proc->cred.euid;
	sem->ds.sem_perm.gid = thread->proc->cred.egid;
	sem->ds.sem_perm.cuid = thread->proc->cred.euid;
	sem->ds.sem_perm.cgid = thread->proc->cred.egid;
	sem->ds.sem_perm.mode = flags & 0777;
	sem->ds.sem_otime = 0;
	sem->ds.sem_ctime = realtime_seconds();
	sem->ds.sem_nsems = nsems;
	waitq_init(&sem->waitq);
	mutex_init(&sem->mutex, 0);
	refcount_init(&sem->refcount, 1);
	TAILQ_INSERT_TAIL(SEM_HEAD(sem->id), sem, chain);
	if (key)
		TAILQ_INSERT_TAIL(SEM_KEY_HEAD(key), sem, key_chain);
	TAILQ_INSERT_TAIL(&sems_list, sem, list_chain);
	sems_count++;
	*semp = sem;
	ret = 0;

end:
	mutex_unlock(&sems_mutex);
	return ret;
}

static void sem_free(struct sysv_sem *sem)
{
	free(sem->values);
	TAILQ_REMOVE(SEM_HEAD(sem->id), sem, chain);
	if (sem->ds.sem_perm.key)
		TAILQ_REMOVE(SEM_KEY_HEAD(sem->ds.sem_perm.key), sem, key_chain);
	TAILQ_REMOVE(&sems_list, sem, list_chain);
	mutex_destroy(&sem->mutex);
	waitq_destroy(&sem->waitq);
	sma_free(&sem_sma, sem);
	sems_count--;
}

static void sem_decref(struct sysv_sem *sem)
{
	mutex_lock(&sems_mutex);
	if (!refcount_dec(&sem->refcount) && sem->removed)
		sem_free(sem);
	mutex_unlock(&sems_mutex);
}

int sys_semget(key_t key, int nsems, int flags)
{
	if (nsems < 0 || nsems > SEMMSL)
		return -EINVAL;
	if (flags & ~(IPC_CREAT | IPC_EXCL | 0777))
		return -EINVAL;
	struct sysv_sem *sem = NULL;
	int ret;
	if (key == IPC_PRIVATE)
	{
		ret = sem_alloc(key, nsems, flags, &sem);
		if (ret)
			goto end;
		ret = sem->id;
		goto end;
	}
	mutex_lock(&sems_mutex);
	struct sem_head *head = SEM_KEY_HEAD(key);
	TAILQ_FOREACH(sem, head, key_chain)
	{
		if (sem->ds.sem_perm.key == key)
		{
			refcount_inc(&sem->refcount);
			break;
		}
	}
	mutex_unlock(&sems_mutex);
	if (!sem)
	{
		if (!(flags & IPC_CREAT))
		{
			ret = -ENOENT;
			goto end;
		}
		ret = sem_alloc(key, nsems, flags, &sem);
		if (ret)
			goto end;
		ret = sem->id;
		goto end;
	}
	if ((flags & (IPC_CREAT | IPC_EXCL)) == (IPC_CREAT | IPC_EXCL))
	{
		ret = -EEXIST;
		goto end;
	}
	struct thread *thread = curcpu()->thread;
	mutex_lock(&sem->mutex);
	ret = ipc_hasperm(&sem->ds.sem_perm,
	                  &thread->proc->cred,
	                  04);
	mutex_unlock(&sem->mutex);
	if (ret)
		goto end;
	ret = sem->id;

end:
	if (sem)
		sem_decref(sem);
	return ret;
}

static int process_sops(struct sysv_sem *sem, struct sembuf *sops,
                        size_t nsops, size_t *processed)
{
	for (size_t i = 0; i < nsops; ++i)
	{
		struct sembuf *sop = &sops[i];
		struct semval *semval = &sem->values[sop->sem_num];
		if (sop->sem_op == 0)
		{
			if (semval->value)
			{
				*processed = i;
				return -EAGAIN;
			}
			continue;
		}
		if (sop->sem_op < 0)
		{
			if (semval->value < -sop->sem_op)
			{
				*processed = i;
				return -EAGAIN;
			}
			semval->value += sop->sem_op;
			continue;
		}
		if (sop->sem_op > 0)
		{
			if (sop->sem_op > SEMVMX - semval->value)
			{
				*processed = i;
				return -ERANGE;
			}
			semval->value += sop->sem_op;
		}
	}
	return 0;
}

static void rollback_sops(struct sysv_sem *sem, struct sembuf *sops,
                          size_t nsops)
{
	for (size_t i = nsops; i > 0; --i)
	{
		struct sembuf *sop = &sops[i - 1];
		struct semval *semval = &sem->values[sop->sem_num];
		semval->value -= sop->sem_op;
	}
}

int sys_semtimedop(int semid, struct sembuf *usops, size_t nsops,
                   const struct timespec *utimeout)
{
	if (!nsops)
		return -EINVAL;
	if (nsops > SEMOPM)
		return -E2BIG;
	struct thread *thread = curcpu()->thread;
	struct sembuf sops[SEMOPM];
	int ret = vm_copyin(thread->proc->vm_space, sops, usops,
	                    sizeof(*sops) * nsops);
	if (ret)
		return ret;
	struct timespec timeout;
	if (utimeout)
	{
		ret = vm_copyin(thread->proc->vm_space, &timeout, utimeout,
		                sizeof(timeout));
		if (ret)
			return ret;
	}
	struct sysv_sem *sem = getsem(semid);
	if (!sem)
		return -EINVAL;
	mutex_lock(&sem->mutex);
	if (sem->removed)
	{
		ret = -EIDRM;
		goto end;
	}
	mode_t perms = 0;
	for (size_t i = 0; i < nsops; ++i)
	{
		struct sembuf *sop = &sops[i];
		if (sop->sem_num >= sem->ds.sem_nsems)
		{
			ret = -EFBIG;
			goto end;
		}
		perms |= sop->sem_op ? 02 : 04;
		if (sop->sem_flg & ~(IPC_NOWAIT | SEM_UNDO))
		{
			ret = -EINVAL;
			goto end;
		}
		/* XXX handle SEM_UNDO */
	}
	ret = ipc_hasperm(&sem->ds.sem_perm,
	                  &thread->proc->cred,
	                  perms);
	if (ret)
		goto end;
	while (1)
	{
		size_t processed;
		ret = process_sops(sem, sops, nsops, &processed);
		if (!ret)
		{
			waitq_broadcast(&sem->waitq, 0);
			goto end;
		}
		rollback_sops(sem, sops, processed);
		if (ret != -EAGAIN)
			goto end;
		struct sembuf *blocked = &sops[processed];
		if (blocked->sem_flg & IPC_NOWAIT)
		{
			ret = -EAGAIN;
			goto end;
		}
		if (blocked->sem_op)
			sem->values[blocked->sem_num].ncnt++;
		else
			sem->values[blocked->sem_num].zcnt++;
		ret = waitq_wait_tail_mutex(&sem->waitq, &sem->mutex,
		                            utimeout ? &timeout : NULL);
		if (blocked->sem_op)
			sem->values[blocked->sem_num].ncnt--;
		else
			sem->values[blocked->sem_num].zcnt--;
		if (ret)
			goto end;
	}

end:
	if (sem)
	{
		mutex_unlock(&sem->mutex);
		sem_decref(sem);
	}
	return ret;
}

int sys_semctl(int semid, int semnum, int cmd, intptr_t data)
{
	union semun *semun = (union semun*)&data;
	struct sysv_sem *sem = getsem(semid);
	if (!sem)
		return -EINVAL;
	int ret;
	struct thread *thread = curcpu()->thread;
	mutex_lock(&sem->mutex);
	if (sem->removed)
	{
		ret = -EIDRM;
		goto end;
	}
	switch (cmd)
	{
		case IPC_STAT:
		{
			ret = ipc_hasperm(&sem->ds.sem_perm,
			                  &thread->proc->cred,
			                  04);
			if (ret)
				goto end;
			ret = vm_copyout(thread->proc->vm_space, semun->buf,
			                 &sem->ds, sizeof(*semun->buf));
			break;
		}
		case IPC_SET:
		{
			ret = ipc_isowner(&sem->ds.sem_perm, &thread->proc->cred);
			if (ret)
				goto end;
			struct semid_ds buf;
			ret = vm_copyin(thread->proc->vm_space, &buf,
			                semun->buf, sizeof(buf));
			if (ret)
				goto end;
			sem->ds.sem_perm.uid = buf.sem_perm.uid;
			sem->ds.sem_perm.gid = buf.sem_perm.gid;
			sem->ds.sem_perm.mode = (sem->ds.sem_perm.mode & ~0777)
			                      | (buf.sem_perm.uid & 0777);
			sem->ds.sem_ctime = realtime_seconds();
			ret = 0;
			break;
		}
		case IPC_RMID:
		{
			ret = ipc_isowner(&sem->ds.sem_perm, &thread->proc->cred);
			if (ret)
				goto end;
			waitq_broadcast(&sem->waitq, -EIDRM);
			sem->removed = 1;
			ret = 0;
			break;
		}
		case GETALL:
		{
			ret = ipc_hasperm(&sem->ds.sem_perm,
			                  &thread->proc->cred,
			                  04);
			if (ret)
				goto end;
			size_t values_pages = (sizeof(uint16_t)
			                     * sem->ds.sem_nsems
			                     + PAGE_MASK) & ~PAGE_MASK;
			void *ptr;
			ret = vm_map_user(thread->proc->vm_space,
			                  (uintptr_t)semun->array,
			                  values_pages,
			                  VM_PROT_W,
			                  &ptr);
			if (ret)
				goto end;
			for (size_t i = 0; i < sem->ds.sem_nsems; ++i)
				((uint16_t*)ptr)[i] = sem->values[i].value;
			vm_unmap(ptr, values_pages);
			ret = 0;
			break;
		}
		case GETNCNT:
		{
			ret = ipc_hasperm(&sem->ds.sem_perm,
			                  &thread->proc->cred,
			                  04);
			if (ret)
				goto end;
			if (semnum < 0 || (size_t)semnum >= sem->ds.sem_nsems)
			{
				ret = -EINVAL;
				goto end;
			}
			int val = sem->values[semnum].ncnt;
			ret = vm_copyout(thread->proc->vm_space, &semun->val,
			                 &val, sizeof(val));
			break;
		}
		case GETPID:
		{
			ret = ipc_hasperm(&sem->ds.sem_perm,
			                  &thread->proc->cred,
			                  04);
			if (ret)
				goto end;
			if (semnum < 0 || (size_t)semnum >= sem->ds.sem_nsems)
			{
				ret = -EINVAL;
				goto end;
			}
			int val = sem->values[semnum].pid;
			ret = vm_copyout(thread->proc->vm_space, &semun->val,
			                 &val, sizeof(val));
			break;
		}
		case GETVAL:
		{
			ret = ipc_hasperm(&sem->ds.sem_perm,
			                  &thread->proc->cred,
			                  04);
			if (ret)
				goto end;
			if (semnum < 0 || (size_t)semnum >= sem->ds.sem_nsems)
			{
				ret = -EINVAL;
				goto end;
			}
			int val = sem->values[semnum].value;
			ret = vm_copyout(thread->proc->vm_space, &semun->val,
			                 &val, sizeof(val));
			break;
		}
		case GETZCNT:
		{
			ret = ipc_hasperm(&sem->ds.sem_perm,
			                  &thread->proc->cred,
			                  04);
			if (ret)
				goto end;
			if (semnum < 0 || (size_t)semnum >= sem->ds.sem_nsems)
			{
				ret = -EINVAL;
				goto end;
			}
			int val = sem->values[semnum].zcnt;
			ret = vm_copyout(thread->proc->vm_space, &semun->val,
			                 &val, sizeof(val));
			break;
		}
		case SETALL:
		{
			ret = ipc_hasperm(&sem->ds.sem_perm,
			                  &thread->proc->cred,
			                  02);
			if (ret)
				goto end;
			size_t values_pages = (sizeof(uint16_t)
			                     * sem->ds.sem_nsems
			                     + PAGE_MASK) & ~PAGE_MASK;
			void *ptr;
			ret = vm_map_user(thread->proc->vm_space,
			                  (uintptr_t)semun->array,
			                  values_pages,
			                  VM_PROT_W,
			                  &ptr);
			if (ret)
				goto end;
			int changed = 0;
			for (size_t i = 0; i < sem->ds.sem_nsems; ++i)
			{
				if (sem->values[i].value != ((uint16_t*)ptr)[i])
				{
					sem->values[i].value = ((uint16_t*)ptr)[i];
					changed = 1;
				}
			}
			vm_unmap(ptr, values_pages);
			if (changed)
				waitq_broadcast(&sem->waitq, 0);
			ret = 0;
			break;
		}
		case SETVAL:
		{
			ret = ipc_hasperm(&sem->ds.sem_perm,
			                  &thread->proc->cred,
			                  02);
			if (ret)
				goto end;
			if (semnum < 0 || (size_t)semnum >= sem->ds.sem_nsems)
			{
				ret = -EINVAL;
				goto end;
			}
			int val;
			ret = vm_copyin(thread->proc->vm_space, &val,
			                 &semun->val, sizeof(val));
			if (ret)
				goto end;
			if (val < 0 || val > SEMVMX)
			{
				ret = -EINVAL;
				goto end;
			}
			if (sem->values[semnum].value != val)
			{
				sem->values[semnum].value = val;
				waitq_broadcast(&sem->waitq, 0);
			}
			ret = 0;
			break;
		}
		default:
			ret = -EINVAL;
			break;
	}

end:
	if (sem)
	{
		mutex_unlock(&sem->mutex);
		sem_decref(sem);
	}
	return ret;
}

static int msg_alloc(key_t key, int flags, struct sysv_msg **msgp)
{
	mutex_lock(&msgs_mutex);
	int ret;
	if (msgs_count >= MSGMNI)
	{
		ret = -ENOSPC;
		goto end;
	}
	struct thread *thread = curcpu()->thread;
	struct sysv_msg *msg = sma_alloc(&msg_sma, 0);
	if (!msg)
	{
		ret = -ENOMEM;
		goto end;
	}
	msg->ds.msg_perm.seq = ++msg_seq;
	msg->id = alloc_ipcid((struct ipc_head*)msgs, msg->ds.msg_perm.seq);
	msg->removed = 0;
	msg->ds.msg_perm.key = key;
	msg->ds.msg_perm.uid = thread->proc->cred.euid;
	msg->ds.msg_perm.gid = thread->proc->cred.egid;
	msg->ds.msg_perm.cuid = thread->proc->cred.euid;
	msg->ds.msg_perm.cgid = thread->proc->cred.egid;
	msg->ds.msg_perm.mode = flags & 0777;
	msg->ds.msg_stime = 0;
	msg->ds.msg_rtime = 0;
	msg->ds.msg_ctime = realtime_seconds();
	msg->ds.msg_cbytes = 0;
	msg->ds.msg_qnum = 0;
	msg->ds.msg_qbytes = MSGMNB;
	msg->ds.msg_lspid = 0;
	msg->ds.msg_lrpid = 0;
	waitq_init(&msg->rwaitq);
	waitq_init(&msg->wwaitq);
	mutex_init(&msg->mutex, 0);
	refcount_init(&msg->refcount, 1);
	TAILQ_INIT(&msg->msgs);
	TAILQ_INSERT_TAIL(MSG_HEAD(msg->id), msg, chain);
	if (key)
		TAILQ_INSERT_TAIL(MSG_KEY_HEAD(key), msg, key_chain);
	TAILQ_INSERT_TAIL(&msgs_list, msg, list_chain);
	msgs_count++;
	*msgp = msg;
	ret = 0;

end:
	mutex_unlock(&msgs_mutex);
	return ret;
}

static void msg_free(struct sysv_msg *msg)
{
	struct msgbuf *buf, *nxt;
	TAILQ_FOREACH_SAFE(buf, &msg->msgs, chain, nxt)
	{
		free(buf);
	}
	TAILQ_REMOVE(MSG_HEAD(msg->id), msg, chain);
	if (msg->ds.msg_perm.key)
		TAILQ_REMOVE(MSG_KEY_HEAD(msg->ds.msg_perm.key), msg, key_chain);
	TAILQ_REMOVE(&msgs_list, msg, list_chain);
	mutex_destroy(&msg->mutex);
	waitq_destroy(&msg->rwaitq);
	waitq_destroy(&msg->wwaitq);
	sma_free(&msg_sma, msg);
	msgs_count--;
}

static void msg_decref(struct sysv_msg *msg)
{
	mutex_lock(&msgs_mutex);
	if (!refcount_dec(&msg->refcount) && msg->removed)
		msg_free(msg);
	mutex_unlock(&msgs_mutex);
}

int sys_msgget(key_t key, int flags)
{
	if (flags & ~(IPC_CREAT | IPC_EXCL | 0777))
		return -EINVAL;
	struct sysv_msg *msg = NULL;
	int ret;
	if (key == IPC_PRIVATE)
	{
		ret = msg_alloc(key, flags, &msg);
		if (ret)
			goto end;
		ret = msg->id;
		goto end;
	}
	mutex_lock(&msgs_mutex);
	struct msg_head *head = MSG_KEY_HEAD(key);
	TAILQ_FOREACH(msg, head, key_chain)
	{
		if (msg->ds.msg_perm.key == key)
		{
			refcount_inc(&msg->refcount);
			break;
		}
	}
	mutex_unlock(&msgs_mutex);
	if (!msg)
	{
		if (!(flags & IPC_CREAT))
		{
			ret = -ENOENT;
			goto end;
		}
		ret = msg_alloc(key, flags, &msg);
		if (ret)
			goto end;
		ret = msg->id;
		goto end;
	}
	if ((flags & (IPC_CREAT | IPC_EXCL)) == (IPC_CREAT | IPC_EXCL))
	{
		ret = -EEXIST;
		goto end;
	}
	struct thread *thread = curcpu()->thread;
	mutex_lock(&msg->mutex);
	ret = ipc_hasperm(&msg->ds.msg_perm,
	                  &thread->proc->cred,
	                  04);
	mutex_unlock(&msg->mutex);
	if (ret)
		goto end;
	ret = msg->id;

end:
	if (msg)
		msg_decref(msg);
	return ret;
}

int sys_msgsnd(int msgid, const void *msgp, size_t msgsz, int flags)
{
	if (flags & ~IPC_NOWAIT)
		return -EINVAL;
	if (msgsz > MSGMAX)
		return -EINVAL;
	struct sysv_msg *msg = getmsg(msgid);
	if (!msg)
		return -EINVAL;
	int ret;
	struct thread *thread = curcpu()->thread;
	mutex_lock(&msg->mutex);
	if (msg->removed)
	{
		ret = -EIDRM;
		goto end;
	}
	ret = ipc_hasperm(&msg->ds.msg_perm,
	                  &thread->proc->cred,
	                  02);
	if (ret)
		goto end;
	while (msg->ds.msg_qbytes - msg->ds.msg_cbytes < msgsz)
	{
		if (flags & IPC_NOWAIT)
		{
			ret = -EAGAIN;
			goto end;
		}
		ret = waitq_wait_tail_mutex(&msg->wwaitq, &msg->mutex, NULL);
		if (ret)
			goto end;
	}
	struct msgbuf *buf = malloc(sizeof(*buf) + msgsz, 0);
	if (!buf)
	{
		ret = -ENOMEM;
		goto end;
	}
	buf->msgsz = msgsz;
	ret = vm_copyin(thread->proc->vm_space, &buf->mtype, msgp,
	                sizeof(long) + msgsz);
	if (ret)
		goto end;
	TAILQ_INSERT_TAIL(&msg->msgs, buf, chain);
	msg->ds.msg_cbytes += msgsz;
	msg->ds.msg_qnum++;
	msg->ds.msg_lspid = thread->proc->pid;
	msg->ds.msg_stime = realtime_seconds();
	waitq_broadcast(&msg->rwaitq, 0);
	thread->stats.msgsnd++;
	thread->proc->stats.msgsnd++;
	ret = 0;

end:
	if (msg)
	{
		mutex_unlock(&msg->mutex);
		msg_decref(msg);
	}
	return ret;
}

ssize_t sys_msgrcv(int msgid, void *msgp, size_t msgsz, long msgtyp,
                   int flags)
{
	if (flags & ~(IPC_NOWAIT | MSG_NOERROR | MSG_EXCEPT))
		return -EINVAL;
	struct sysv_msg *msg = getmsg(msgid);
	if (!msg)
		return -EINVAL;
	ssize_t ret;
	struct thread *thread = curcpu()->thread;
	mutex_lock(&msg->mutex);
	if (msg->removed)
	{
		ret = -EIDRM;
		goto end;
	}
	ret = ipc_hasperm(&msg->ds.msg_perm,
	                  &thread->proc->cred,
	                  04);
	if (ret)
		goto end;
	struct msgbuf *buf;
	do
	{
		while (!msg->ds.msg_qnum)
		{
			if (flags & IPC_NOWAIT)
			{
				ret = -ENOMSG;
				goto end;
			}
			ret = waitq_wait_tail_mutex(&msg->rwaitq, &msg->mutex,
			                            NULL);
			if (ret)
				goto end;
		}
		if (msgtyp > 0)
		{
			if (flags & MSG_EXCEPT)
			{
				TAILQ_FOREACH(buf, &msg->msgs, chain)
				{
					if (buf->mtype != msgtyp)
						break;
				}
			}
			else
			{
				TAILQ_FOREACH(buf, &msg->msgs, chain)
				{
					if (buf->mtype == msgtyp)
						break;
				}
			}
		}
		else if (msgtyp < 0)
		{
			long best = LONG_MAX;
			TAILQ_FOREACH(buf, &msg->msgs, chain)
			{
				if (buf->mtype <= -msgtyp && buf->mtype < best)
					break;
			}
		}
		else
		{
			buf = TAILQ_FIRST(&msg->msgs);
		}
	} while (!buf);
	size_t cpy_len;
	if (buf->msgsz > msgsz)
	{
		if (!(flags & MSG_NOERROR))
		{
			ret = -E2BIG;
			goto end;
		}
		cpy_len = msgsz;
	}
	else
	{
		cpy_len = buf->msgsz;
	}
	ret = vm_copyout(thread->proc->vm_space, msgp, &buf->mtype,
	                 cpy_len + sizeof(long));
	if (ret)
		goto end;
	TAILQ_REMOVE(&msg->msgs, buf, chain);
	free(buf);
	msg->ds.msg_cbytes -= buf->msgsz;
	msg->ds.msg_lrpid = thread->proc->pid;
	msg->ds.msg_qnum--;
	msg->ds.msg_rtime = realtime_seconds();
	waitq_broadcast(&msg->wwaitq, 0);
	thread->stats.msgrcv++;
	thread->proc->stats.msgrcv++;
	ret = cpy_len;

end:
	if (msg)
	{
		mutex_unlock(&msg->mutex);
		msg_decref(msg);
	}
	return ret;
}

int sys_msgctl(int msgid, int cmd, struct msgid_ds *ubuf)
{
	struct sysv_msg *msg = getmsg(msgid);
	if (!msg)
		return -EINVAL;
	int ret;
	struct thread *thread = curcpu()->thread;
	mutex_lock(&msg->mutex);
	if (msg->removed)
	{
		ret = -EIDRM;
		goto end;
	}
	switch (cmd)
	{
		case IPC_STAT:
		{
			ret = ipc_hasperm(&msg->ds.msg_perm,
			                  &thread->proc->cred,
			                  04);
			if (ret)
				goto end;
			ret = vm_copyout(thread->proc->vm_space, ubuf,
			                 &msg->ds, sizeof(*ubuf));
			break;
		}
		case IPC_SET:
		{
			ret = ipc_isowner(&msg->ds.msg_perm, &thread->proc->cred);
			if (ret)
				goto end;
			struct msgid_ds buf;
			ret = vm_copyin(thread->proc->vm_space, &buf,
			                ubuf, sizeof(ubuf));
			if (ret)
				goto end;
			msg->ds.msg_perm.uid = buf.msg_perm.uid;
			msg->ds.msg_perm.gid = buf.msg_perm.gid;
			msg->ds.msg_perm.mode = (msg->ds.msg_perm.mode & ~0777)
			                      | (buf.msg_perm.uid & 0777);
			msg->ds.msg_qbytes = msg->ds.msg_qbytes; /* XXX from buf */
			msg->ds.msg_ctime = realtime_seconds();
			ret = 0;
			break;
		}
		case IPC_RMID:
		{
			ret = ipc_isowner(&msg->ds.msg_perm, &thread->proc->cred);
			if (ret)
				goto end;
			waitq_broadcast(&msg->rwaitq, -EIDRM);
			waitq_broadcast(&msg->wwaitq, -EIDRM);
			msg->removed = 1;
			ret = 0;
			break;
		}
		default:
			ret = -EINVAL;
			break;
	}

end:
	if (msg)
	{
		mutex_unlock(&msg->mutex);
		msg_decref(msg);
	}
	return ret;
}
