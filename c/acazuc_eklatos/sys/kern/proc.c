#define ENABLE_TRACE

#if defined(__aarch64__)
#include <arch/asm.h>
#endif

#include <endian.h>
#include <ptrace.h>
#include <random.h>
#include <errno.h>
#include <sched.h>
#include <proc.h>
#include <file.h>
#include <auxv.h>
#include <std.h>
#include <vfs.h>
#include <uio.h>
#include <sma.h>
#include <mem.h>

#define PANIC_ON_PFR 0
#define PANIC_ON_PFW 0
#define PANIC_ON_PFX 0
#define PANIC_ON_ILL 0

#define AUXV_SIZE 14

static struct spinlock g_sess_list_lock = SPINLOCK_INITIALIZER(); /* XXX rwlock */
static struct sess_head g_sess_list = TAILQ_HEAD_INITIALIZER(g_sess_list);
struct spinlock g_proc_list_lock = SPINLOCK_INITIALIZER(); /* XXX rwlock */
struct proc_head g_proc_list = TAILQ_HEAD_INITIALIZER(g_proc_list);
struct spinlock g_thread_list_lock = SPINLOCK_INITIALIZER(); /* XXX rwlock */
struct thread_head g_thread_list = TAILQ_HEAD_INITIALIZER(g_thread_list);

static struct thread *g_initthread;

static pid_t g_pid;

static struct sma sess_sma;
static struct sma pgrp_sma;
static struct sma thread_sma;
static struct sma proc_sma;

void proc_init(void)
{
	sma_init(&sess_sma, sizeof(struct sess), NULL, NULL, "sess");
	sma_init(&pgrp_sma, sizeof(struct pgrp), NULL, NULL, "pgrp");
	sma_init(&thread_sma, sizeof(struct thread), NULL, NULL, "thread");
	sma_init(&proc_sma, sizeof(struct proc), NULL, NULL, "proc");
}

struct sess *sess_alloc(pid_t id)
{
	struct sess *sess = sma_alloc(&sess_sma, 0);
	if (!sess)
		return NULL;
	refcount_init(&sess->refcount, 1);
	mutex_init(&sess->mutex, 0);
	sess->id = id;
	TAILQ_INIT(&sess->groups);
	spinlock_lock(&g_sess_list_lock);
	TAILQ_INSERT_TAIL(&g_sess_list, sess, chain);
	spinlock_unlock(&g_sess_list_lock);
	return sess;
}

void sess_free(struct sess *sess)
{
	if (!sess)
		return;
	if (refcount_dec(&sess->refcount))
		return;
	spinlock_lock(&g_sess_list_lock);
	if (refcount_get(&sess->refcount))
	{
		spinlock_unlock(&g_sess_list_lock);
		return;
	}
	TAILQ_REMOVE(&g_sess_list, sess, chain);
	spinlock_unlock(&g_sess_list_lock);
	sma_free(&sess_sma, sess);
}

void sess_ref(struct sess *sess)
{
	refcount_inc(&sess->refcount);
}

void sess_lock(struct sess *sess)
{
	mutex_lock(&sess->mutex);
}

void sess_unlock(struct sess *sess)
{
	mutex_unlock(&sess->mutex);
}

struct pgrp *pgrp_alloc(pid_t id, struct sess *sess)
{
	struct pgrp *pgrp = sma_alloc(&pgrp_sma, 0);
	if (!pgrp)
		return NULL;
	refcount_init(&pgrp->refcount, 1);
	mutex_init(&pgrp->mutex, 0);
	pgrp->sess = sess;
	pgrp->id = id;
	TAILQ_INIT(&pgrp->processes);
	sess_lock(sess);
	TAILQ_INSERT_TAIL(&sess->groups, pgrp, chain);
	sess_unlock(sess);
	sess_ref(sess);
	return pgrp;
}

void pgrp_free(struct pgrp *pgrp)
{
	if (!pgrp)
		return;
	if (refcount_dec(&pgrp->refcount))
		return;
	sess_lock(pgrp->sess);
	if (refcount_get(&pgrp->refcount))
	{
		sess_unlock(pgrp->sess);
		return;
	}
	TAILQ_REMOVE(&pgrp->sess->groups, pgrp, chain);
	sess_unlock(pgrp->sess);
	sess_free(pgrp->sess);
	sma_free(&pgrp_sma, pgrp);
}

void pgrp_ref(struct pgrp *pgrp)
{
	refcount_inc(&pgrp->refcount);
}

struct pgrp *getpgrp(pid_t id)
{
	struct sess *sess;
	spinlock_lock(&g_sess_list_lock);
	TAILQ_FOREACH(sess, &g_sess_list, chain)
	{
		sess_lock(sess);
		struct pgrp *pgrp;
		TAILQ_FOREACH(pgrp, &sess->groups, chain)
		{
			if (pgrp->id == id)
			{
				pgrp_ref(pgrp);
				sess_unlock(sess);
				spinlock_unlock(&g_sess_list_lock);
				return pgrp;
			}
		}
		sess_unlock(sess);
	}
	spinlock_unlock(&g_sess_list_lock);
	return NULL;
}

void pgrp_lock(struct pgrp *pgrp)
{
	mutex_lock(&pgrp->mutex);
}

void pgrp_unlock(struct pgrp *pgrp)
{
	mutex_unlock(&pgrp->mutex);
}

void proc_setpgrp(struct proc *proc, struct pgrp *pgrp)
{
	struct pgrp *old_pgrp = proc->pgrp;
	pgrp_lock(old_pgrp);
	TAILQ_REMOVE(&proc->pgrp->processes, proc, pgrp_chain);
	pgrp_unlock(old_pgrp);
	pgrp_free(old_pgrp);
	mutex_lock(&pgrp->mutex);
	TAILQ_INSERT_TAIL(&pgrp->processes, proc, pgrp_chain);
	proc->pgrp = pgrp;
	pgrp_ref(pgrp);
	mutex_unlock(&pgrp->mutex);
}

static int thread_alloc(struct thread **threadp)
{
	struct thread *thread = sma_alloc(&thread_sma, M_ZERO);
	if (!thread)
	{
		TRACE("failed to allocate thread");
		return -ENOMEM;
	}
	spinlock_init(&thread->ptrace_waitq_lock);
	waitq_init(&thread->ptrace_waitq);
	thread->int_stack_size = KSTACK_SIZE;
	thread->int_stack = vmalloc(thread->int_stack_size);
	if (!thread->int_stack)
	{
		TRACE("failed to allocate thread interrupt stack");
		sma_free(&thread_sma, thread);
		return -ENOMEM;
	}
	thread->running_cpuid = -1;
	refcount_init(&thread->refcount, 1);
	*threadp = thread;
	return 0;
}

static int thread_create(struct proc *proc, struct thread **threadp)
{
	struct thread *thread;
	int ret = thread_alloc(&thread);
	if (ret)
		return ret;
	thread->proc = proc;
	thread->tid = proc->pid;
	CPUMASK_FILL(&thread->affinity);
	thread->state = THREAD_PAUSED;
	thread->stack_size = 1024 * 1024;
	struct vm_zone *zone;
	mutex_lock(&proc->vm_space->mutex);
	ret = vm_alloc(proc->vm_space,
	               proc->vm_space->region.addr
	             + proc->vm_space->region.size
	             - thread->stack_size,
	               0, thread->stack_size, 0, VM_PROT_RW, MAP_PRIVATE,
	               NULL, &zone);
	mutex_unlock(&proc->vm_space->mutex);
	if (ret)
	{
		TRACE("failed to allocate thread stack");
		sma_free(&thread_sma, thread);
		return ret;
	}
	thread->stack = (void*)zone->addr;
	assert(thread->stack, "can't allocate thread stack\n");
	*threadp = thread;
	return 0;
}

void thread_free(struct thread *thread)
{
	if (!thread)
		return;
	if (refcount_dec(&thread->refcount))
		return;
	if (thread->waitq)
		panic("killing waiting thread\n");
	waitq_destroy(&thread->ptrace_waitq);
	spinlock_destroy(&thread->ptrace_waitq_lock);
	vfree(thread->int_stack, thread->int_stack_size);
	sched_dequeue(thread);
	spinlock_lock(&g_thread_list_lock);
	TAILQ_REMOVE(&g_thread_list, thread, chain);
	spinlock_unlock(&g_thread_list_lock);
	TAILQ_REMOVE(&thread->proc->threads, thread, thread_chain);
	sma_free(&thread_sma, thread);
}

void thread_ref(struct thread *thread)
{
	refcount_inc(&thread->refcount);
}

static int proc_alloc(struct proc **procp)
{
	struct proc *proc = sma_alloc(&proc_sma, M_ZERO);
	if (!proc)
	{
		TRACE("failed to allocate proc");
		return -ENOMEM;
	}
	spinlock_init(&proc->wait_waitq_lock);
	waitq_init(&proc->wait_waitq);
	spinlock_init(&proc->vfork_waitq_lock);
	waitq_init(&proc->vfork_waitq);
	mutex_init(&proc->mutex, 0);
	TAILQ_INIT(&proc->ptrace_tracees);
	TAILQ_INIT(&proc->threads);
	TAILQ_INIT(&proc->childs);
	rwlock_init(&proc->files_lock);
	*procp = proc;
	return 0;
}

void proc_free(struct proc *proc)
{
	if (!proc)
		return;
	struct thread *thread, *next;
	TAILQ_FOREACH_SAFE(thread, &proc->threads, thread_chain, next)
		thread_free(thread);
	vm_space_free(proc->vm_space);
	free(proc->name);
	free(proc->files);
	if (proc->parent)
		TAILQ_REMOVE(&proc->parent->childs, proc, child_chain);
	spinlock_lock(&g_proc_list_lock);
	TAILQ_REMOVE(&g_proc_list, proc, chain);
	spinlock_unlock(&g_proc_list_lock);
	pgrp_lock(proc->pgrp);
	TAILQ_REMOVE(&proc->pgrp->processes, proc, pgrp_chain);
	pgrp_unlock(proc->pgrp);
	pgrp_free(proc->pgrp);
	mutex_destroy(&proc->mutex);
	waitq_destroy(&proc->wait_waitq);
	spinlock_destroy(&proc->wait_waitq_lock);
	waitq_destroy(&proc->vfork_waitq);
	spinlock_destroy(&proc->vfork_waitq_lock);
	node_free(proc->root);
	node_free(proc->cwd);
	sma_free(&proc_sma, proc);
}

static int proc_create(const char *name, struct vm_space *vm_space, void *entry,
                       struct thread **threadp)
{
	struct proc *proc;
	int ret = proc_alloc(&proc);
	if (ret)
		return ret;
	proc->name = strdup(name);
	if (!proc->name)
	{
		TRACE("failed to allocate proc name");
		proc_free(proc);
		return -ENOMEM;
	}
	pid_t pid = __atomic_add_fetch(&g_pid, 1, __ATOMIC_SEQ_CST);
	struct sess *sess = sess_alloc(pid);
	if (!sess)
	{
		TRACE("failed to create proc sid");
		proc_free(proc);
		return -ENOMEM;
	}
	struct pgrp *pgrp = pgrp_alloc(pid, sess);
	sess_free(sess);
	if (!pgrp)
	{
		TRACE("failed to create proc pgrp");
		proc_free(proc);
		return -ENOMEM;
	}
	TAILQ_INSERT_TAIL(&pgrp->processes, proc, pgrp_chain);
	proc->root = g_vfs_root;
	node_ref(proc->root);
	proc->cwd = proc->root;
	node_ref(proc->root);
	proc->files = NULL;
	proc->files_nb = 0;
	proc->vm_space = vm_space;
	proc->entrypoint = entry;
	proc->pid = pid;
	proc->pgrp = pgrp;
	proc->cred.uid = 0;
	proc->cred.euid = 0;
	proc->cred.suid = 0;
	proc->cred.gid = 0;
	proc->cred.egid = 0;
	proc->cred.sgid = 0;
	proc->umask = 022;
	proc->state = PROC_ALIVE;
	for (size_t i = 0; i < SIGLAST; ++i)
		proc->sigactions[i].sa_handler = (void*)SIG_DFL;
	struct thread *thread;
	ret = thread_create(proc, &thread);
	if (ret)
	{
		TRACE("failed to create thread");
		proc_free(proc);
		return ret;
	}
	TAILQ_INSERT_HEAD(&proc->threads, thread, thread_chain);
	spinlock_lock(&g_proc_list_lock);
	TAILQ_INSERT_TAIL(&g_proc_list, proc, chain);
	spinlock_unlock(&g_proc_list_lock);
	*threadp = thread;
	return 0;
}

static size_t stack_array_size(const char * const *array, size_t *size)
{
	size_t total = 0;
	for (*size = 0; array[*size]; ++*size)
		total += strlen(array[*size]) + 1;
	return total;
}

static void stack_push_array(const char * const *array, size_t size,
                             char **stackp)
{
	for (size_t i = size; i > 0; --i)
	{
		size_t len = strlen(array[i - 1]) + 1;
		*stackp -= len;
		memcpy(*stackp, array[i - 1], len);
	}
}

static void stack_push_pointers(const char * const *array, size_t size,
                                char **org, char **stackp)
{
	for (size_t i = size; i > 0; --i)
	{
		*org -= strlen(array[i - 1]) + 1;
		*stackp -= sizeof(char*);
		**(char***)stackp = *org;
	}
}

static void stack_push_auxv(const size_t *auxv, size_t auxc,
                            char **stackp)
{
	for (size_t i = auxc; i > 0; --i)
	{
		*stackp -= sizeof(size_t) * 2;
		((size_t*)*stackp)[0] = auxv[(i - 1) * 2 + 0];
		((size_t*)*stackp)[1] = auxv[(i - 1) * 2 + 1];
	}
}

static size_t get_init_args_size(const char * const *pre_argv,
                                 const char * const *argv,
                                 const char * const *envp,
                                 const size_t *auxv,
                                 size_t *pre_argc,
                                 size_t *argc,
                                 size_t *envc,
                                 size_t auxc)
{
	(void)auxv;
	size_t pre_arg_size;
	if (pre_argv)
	{
		pre_arg_size = stack_array_size(pre_argv, pre_argc);
	}
	else
	{
		pre_arg_size = 0;
		*pre_argc = 0;
	}
	size_t arg_size = stack_array_size(argv, argc);
	size_t env_size = stack_array_size(envp, envc);
	size_t total_size = pre_arg_size + arg_size + env_size 
	                  + (*pre_argc + *argc + 1 + *envc + 1) * sizeof(char*)
	                  + auxc * sizeof(size_t) * 2;
#if ARCH_REGISTER_PARAMETERS < 1
	total_size += sizeof(size_t);
#endif
#if ARCH_REGISTER_PARAMETERS < 2
	total_size += sizeof(size_t);
#endif
#if ARCH_REGISTER_PARAMETERS < 3
	total_size += sizeof(size_t);
#endif
#if ARCH_REGISTER_PARAMETERS < 4
	total_size += sizeof(size_t);
#endif
	size_t alignment_pad = total_size % ARCH_STACK_ALIGNMENT;
	if (alignment_pad)
		total_size += ARCH_STACK_ALIGNMENT - alignment_pad;
#if ARCH_STACK_RETURN_ADDR
	total_size += sizeof(void*);
#endif
	return total_size;
}

static size_t forge_init_args(void *ustack, char *stack, size_t stack_size,
                              size_t pre_argc, const char * const *pre_argv,
                              size_t argc, const char * const *argv,
                              size_t envc, const char * const *envp,
                              size_t auxc, const size_t *auxv,
                              int *stack_argcp,
                              char ***stack_argvp,
                              char ***stack_envpp,
                              size_t **stack_auxvp)
{
	char *ustack_top = (char*)ustack + stack_size;
	char *stackp = stack;
	stack_push_array(argv, argc, &stackp);
	stack_push_array(pre_argv, pre_argc, &stackp);
	stack_push_array(envp, envc, &stackp);
	char *org = (char*)ustack + stack_size;
	char **stack_argv;
	char **stack_envp;
	size_t *stack_auxv;

	stackp -= sizeof(char*);
	*(char**)stackp = NULL;
	stack_push_pointers(argv, argc, &org, &stackp);
	stack_push_pointers(pre_argv, pre_argc, &org, &stackp);
	stack_argv = (char**)(ustack_top - (stack - stackp));

	stackp -= sizeof(char*);
	*(char**)stackp = NULL;
	stack_push_pointers(envp, envc, &org, &stackp);
	stack_envp = (char**)(ustack_top - (stack - stackp));

	stack_push_auxv(auxv, auxc, &stackp);
	stack_auxv = (size_t*)(ustack_top - (stack - stackp));

#if ARCH_REGISTER_PARAMETERS < 1
	stackp -= sizeof(size_t*);
	*(size_t**)stackp = stack_auxv;
#endif
#if ARCH_REGISTER_PARAMETERS < 2
	stackp -= sizeof(char*);
	*(char***)stackp = stack_envp;
#endif
#if ARCH_REGISTER_PARAMETERS < 3
	stackp -= sizeof(char*);
	*(char***)stackp = stack_argv;
#endif
#if ARCH_REGISTER_PARAMETERS < 4
	stackp -= sizeof(int);
	*(int*)stackp = argc + pre_argc;
#endif

	stackp -= (uintptr_t)stackp % ARCH_STACK_ALIGNMENT;

	*stack_argcp = argc + pre_argc;
	*stack_argvp = stack_argv;
	*stack_envpp = stack_envp;
	*stack_auxvp = stack_auxv;

#if ARCH_STACK_RETURN_ADDR
	stackp -= sizeof(void*); /* stack return value */
	*(void**)stackp = 0;
#endif

	return stack - stackp;
}

static int mem_push_init_args(struct vm_space *vm_space, void *stack,
                              size_t stack_size, const char * const *pre_argv,
                              const char * const *argv,
                              const char * const *envp,
                              const size_t *auxv, size_t auxc,
                              int *stack_argcp,
                              char ***stack_argvp,
                              char ***stack_envpp,
                              size_t **stack_auxvp,
                              uintptr_t *sp)
{
	size_t pre_argc;
	size_t argc;
	size_t envc;
	size_t total_size = get_init_args_size(pre_argv, argv, envp, auxv,
	                                        &pre_argc, &argc, &envc, auxc);
	if (total_size >= stack_size)
		return -E2BIG;
	uintptr_t map_base = (uintptr_t)stack + stack_size - total_size;
	size_t map_align = (uintptr_t)map_base & PAGE_MASK;
	map_base -= map_align;
	size_t map_size = (total_size + map_align + PAGE_MASK) & ~PAGE_MASK;
	char *stackorg;
	int ret = vm_map_user(vm_space, map_base, map_size,
	                      VM_PROT_RW, (void**)&stackorg);
	if (ret)
		return ret;
	size_t bytes = forge_init_args(stack, stackorg + map_size, stack_size,
	                               pre_argc, pre_argv,
	                               argc, argv,
	                               envc, envp,
	                               auxc, auxv,
	                               stack_argcp,
	                               stack_argvp,
	                               stack_envpp,
	                               stack_auxvp);
	vm_unmap(stackorg, map_size);
	assert(bytes == total_size, "%zu != %zu\n", bytes, total_size);
	*sp = (uintptr_t)stack + stack_size - total_size;
	return 0;
}

static int proc_push_init_args(struct thread *thread,
                               const char * const *argv,
                               const char * const *envp,
                               size_t *auxv, size_t auxc)
{
	uintptr_t sp;
	int stack_argc;
	char **stack_argv;
	char **stack_envp;
	size_t *stack_auxv;
	int ret = mem_push_init_args(thread->proc->vm_space, thread->stack,
	                              thread->stack_size, NULL,
	                              argv, envp, auxv, auxc,
	                              &stack_argc, &stack_argv,
	                              &stack_envp, &stack_auxv,
	                              &sp);
	if (ret)
	{
		TRACE("failed to push args");
		return ret;
	}
	arch_set_stack_pointer(&thread->tf_user, sp);
#if ARCH_REGISTER_PARAMETERS >= 1
	arch_set_argument0(&thread->tf_user, (uintptr_t)stack_argc);
#endif
#if ARCH_REGISTER_PARAMETERS >= 2
	arch_set_argument1(&thread->tf_user, (uintptr_t)stack_argv);
#endif
#if ARCH_REGISTER_PARAMETERS >= 3
	arch_set_argument2(&thread->tf_user, (uintptr_t)stack_envp);
#endif
#if ARCH_REGISTER_PARAMETERS >= 4
	arch_set_argument3(&thread->tf_user, (uintptr_t)stack_auxv);
#endif
	return 0;
}

static int create_auxv(size_t auxv[AUXV_SIZE * 2],
                       const struct thread *thread,
                       const struct elf_info *info)
{
	auxv[0 * 2 + 0] = AT_ENTRY;
	auxv[0 * 2 + 1] = info->entry;
	auxv[1 * 2 + 0] = AT_BASE;
	auxv[1 * 2 + 1] = info->base_addr;
	auxv[2 * 2 + 0] = AT_PAGESZ;
	auxv[2 * 2 + 1] = PAGE_SIZE;
	auxv[3 * 2 + 0] = AT_PHDR;
	auxv[3 * 2 + 1] = info->phaddr;
	auxv[4 * 2 + 0] = AT_PHNUM;
	auxv[4 * 2 + 1] = info->phnum;
	auxv[5 * 2 + 0] = AT_PHENT;
	auxv[5 * 2 + 1] = info->phent;
	auxv[6 * 2 + 0] = AT_UID;
	auxv[6 * 2 + 1] = thread->proc->cred.uid;
	auxv[7 * 2 + 0] = AT_EUID;
	auxv[7 * 2 + 1] = thread->proc->cred.euid;
	auxv[8 * 2 + 0] = AT_GID;
	auxv[8 * 2 + 1] = thread->proc->cred.gid;
	auxv[9 * 2 + 0] = AT_EGID;
	auxv[9 * 2 + 1] = thread->proc->cred.egid;
	auxv[10 * 2 + 0] = AT_RANDOM;
	ssize_t ret = random_get(&auxv[10 * 2 + 1], sizeof(*auxv));
	if (ret < 0)
	{

		TRACE("failed to get random");
		return ret;
	}
	if ((size_t)ret != sizeof(*auxv))
	{
		TRACE("failed to get full random");
		return -ENOMEM;
	}
	auxv[11 * 2 + 0] = AT_HWCAP;
	auxv[11 * 2 + 1] = 0;
#if defined(__aarch64__)
	size_t *hwcap = &auxv[11 * 2 + 1];
	uint64_t id_aa64pfr0_el1 = get_id_aa64pfr0_el1();
	uint64_t id_aa64pfr1_el1 = get_id_aa64pfr1_el1();
#if 0
	uint64_t id_aa64pfr2_el1 = get_id_aa64pfr2_el1();
#endif
	uint64_t id_aa64isar0_el1 = get_id_aa64isar0_el1();
	uint64_t id_aa64isar1_el1 = get_id_aa64isar1_el1();
	uint64_t id_aa64isar2_el1 = get_id_aa64isar2_el1();
	uint64_t id_aa64isar3_el1 = get_id_aa64isar3_el1();
	uint64_t id_aa64mmfr0_el1 = get_id_aa64mmfr0_el1();
	uint64_t id_aa64mmfr1_el1 = get_id_aa64mmfr1_el1();
	uint64_t id_aa64mmfr2_el1 = get_id_aa64mmfr2_el1();
#if 0
	uint64_t id_aa64zfr0_el1 = get_id_aa64zfr0_el1();
#endif
#if 0
	uint64_t id_aa64smfr0_el1 = get_id_aa64smfr0_el1();
#endif
#if 0
	uint64_t id_aa64fpfr0_el1 = get_id_aa64fpfr0_el1();
#endif
	if (((id_aa64pfr0_el1 >> 16) & 0xF) == 0)
		*hwcap |= HWCAP_FP;
	if (((id_aa64pfr0_el1 >> 20) & 0xF) == 0)
		*hwcap |= HWCAP_ASIMD;
	if (((id_aa64isar0_el1 >> 4) & 0xF) == 1)
		*hwcap |= HWCAP_AES;
	if (((id_aa64isar0_el1 >> 4) & 0xF) == 2)
		*hwcap |= HWCAP_PMULL;
	if (((id_aa64isar0_el1 >> 8) & 0xF) == 1)
		*hwcap |= HWCAP_SHA1;
	if (((id_aa64isar0_el1 >> 12) & 0xF) == 1)
		*hwcap |= HWCAP_SHA2;
	if (((id_aa64isar0_el1 >> 16) & 0xF) == 1)
		*hwcap |= HWCAP_CRC32;
	if (((id_aa64isar0_el1 >> 20) & 0xF) == 2)
		*hwcap |= HWCAP_ATOMICS;
	if (((id_aa64pfr0_el1 >> 16) & 0xF) == 1)
		*hwcap |= HWCAP_FPHP;
	if (((id_aa64pfr0_el1 >> 20) & 0xF) == 1)
		*hwcap |= HWCAP_ASIMDHP;
	/* XXX HWCAP_CPUID */
	if (((id_aa64isar0_el1 >> 28) & 0xF) == 1)
		*hwcap |= HWCAP_ASIMDRDM;
	if (((id_aa64isar1_el1 >> 12) & 0xF) == 1)
		*hwcap |= HWCAP_JSCVT;
	if (((id_aa64isar1_el1 >> 16) & 0xF) == 1)
		*hwcap |= HWCAP_FCMA;
	if (((id_aa64isar1_el1 >> 20) & 0xF) == 1)
		*hwcap |= HWCAP_LRCPC;
	if (((id_aa64isar1_el1 >> 0) & 0xF) == 1)
		*hwcap |= HWCAP_DCPOP;
	if (((id_aa64isar0_el1 >> 32) & 0xF) == 1)
		*hwcap |= HWCAP_SHA3;
	if (((id_aa64isar0_el1 >> 36) & 0xF) == 1)
		*hwcap |= HWCAP_SM3;
	if (((id_aa64isar0_el1 >> 40) & 0xF) == 1)
		*hwcap |= HWCAP_SM4;
	if (((id_aa64isar0_el1 >> 44) & 0xF) == 1)
		*hwcap |= HWCAP_ASIMDDP;
	if (((id_aa64isar0_el1 >> 12) & 0xF) == 2)
		*hwcap |= HWCAP_SHA512;
	if (((id_aa64pfr0_el1 >> 32) & 0xF) == 1)
		*hwcap |= HWCAP_SVE;
	if (((id_aa64isar0_el1 >> 48) & 0xF) == 1)
		*hwcap |= HWCAP_ASIMDFHM;
	if (((id_aa64pfr0_el1 >> 48) & 0xF) == 1)
		*hwcap |= HWCAP_DIT;
	if (((id_aa64mmfr2_el1 >> 32) & 0xF) == 1)
		*hwcap |= HWCAP_USCAT;
	if (((id_aa64isar1_el1 >> 20) & 0xF) == 2)
		*hwcap |= HWCAP_ILRCPC;
	if (((id_aa64isar0_el1 >> 52) & 0xF) == 1)
		*hwcap |= HWCAP_FLAGM;
	if (((id_aa64pfr1_el1 >> 4) & 0xF) == 2)
		*hwcap |= HWCAP_SSBS;
	if (((id_aa64isar1_el1 >> 36) & 0xF) == 1)
		*hwcap |= HWCAP_SB;
	if (((id_aa64isar1_el1 >> 4) & 0xF) == 1
	 || ((id_aa64isar1_el1 >> 8) & 0xF) == 1)
		*hwcap |= HWCAP_PACA;
	if (((id_aa64isar1_el1 >> 24) & 0xF) == 1
	 || ((id_aa64isar1_el1 >> 28) & 0xF) == 1)
		*hwcap |= HWCAP_PACG;
#endif
	auxv[12 * 2 + 0] = AT_HWCAP2;
	auxv[12 * 2 + 1] = 0;
#if defined(__aarch64__)
	size_t *hwcap2 = &auxv[11 * 2 + 1];
	if (((id_aa64isar1_el1 >> 0) & 0xF) == 2)
		*hwcap2 |= HWCAP2_DCPODP;
#if 0
	if (((id_aa64zfr0_el1 >> 0) & 0xF) == 1)
		*hwcap2 |= HWCAP2_SVE2;
	if (((id_aa64zfr0_el1 >> 4) & 0xF) == 1)
		*hwcap2 |= HWCAP2_SVEAES;
	if (((id_aa64zfr0_el1 >> 4) & 0xF) == 2)
		*hwcap2 |= HWCAP2_SVEPMULL;
	if (((id_aa64zfr0_el1 >> 16) & 0xF) == 1)
		*hwcap2 |= HWCAP2_SVEBITPERM;
	if (((id_aa64zfr0_el1 >> 32) & 0xF) == 1)
		*hwcap2 |= HWCAP2_SVESHA3;
	if (((id_aa64zfr0_el1 >> 40) & 0xF) == 1)
		*hwcap2 |= HWCAP2_SVESM4;
#endif
	if (((id_aa64isar0_el1 >> 52) & 0xF) == 2)
		*hwcap2 |= HWCAP2_FLAGM2;
	if (((id_aa64isar1_el1 >> 32) & 0xF) == 1)
		*hwcap2 |= HWCAP2_FRINT;
#if 0
	if (((id_aa64zfr0_el1 >> 44) & 0xF) == 1)
		*hwcap2 |= HWCAP2_SVEI8MM;
	if (((id_aa64zfr0_el1 >> 52) & 0xF) == 1)
		*hwcap2 |= HWCAP2_SVEF32MM;
	if (((id_aa64zfr0_el1 >> 56) & 0xF) == 1)
		*hwcap2 |= HWCAP2_SVEF64MM;
	if (((id_aa64zfr0_el1 >> 20) & 0xF) == 1)
		*hwcap2 |= HWCAP2_SVEBF16;
#endif
	if (((id_aa64isar1_el1 >> 52) & 0xF) == 1)
		*hwcap2 |= HWCAP2_I8MM;
	if (((id_aa64isar1_el1 >> 44) & 0xF) == 1)
		*hwcap2 |= HWCAP2_BF16;
	if (((id_aa64isar1_el1 >> 48) & 0xF) == 1)
		*hwcap2 |= HWCAP2_DGH;
	if (((id_aa64isar0_el1 >> 60) & 0xF) == 1)
		*hwcap2 |= HWCAP2_RNG;
	if (((id_aa64pfr1_el1 >> 0) & 0xF) == 1)
		*hwcap2 |= HWCAP2_BTI;
	if (((id_aa64pfr1_el1 >> 8) & 0xF) == 1)
		*hwcap2 |= HWCAP2_MTE;
	if (((id_aa64mmfr0_el1 >> 60) & 0xF) == 1)
		*hwcap2 |= HWCAP2_ECV;
	if (((id_aa64mmfr1_el1 >> 44) & 0xF) == 1)
		*hwcap2 |= HWCAP2_AFP;
	if (((id_aa64isar2_el1 >> 4) & 0xF) == 1)
		*hwcap2 |= HWCAP2_RPRES;
	if (((id_aa64pfr1_el1 >> 8) & 0xF) == 3)
		*hwcap2 |= HWCAP2_MTE3;
	if (((id_aa64pfr1_el1 >> 24) & 0xF) == 1)
		*hwcap2 |= HWCAP2_SME;
#if 0
	if (((id_aa64smfr0_el1 >> 52) & 0xF) == 0xF)
		*hwcap2 |= HWCAP2_SME_I16I64;
	if (((id_aa64smfr0_el1 >> 48) & 0x1) == 1)
		*hwcap2 |= HWCAP2_SME_F64F64;
	if (((id_aa64smfr0_el1 >> 26) & 0xF) == 1)
		*hwcap2 |= HWCAP2_SME_I8I32;
	if (((id_aa64smfr0_el1 >> 35) & 0x1) == 1)
		*hwcap2 |= HWCAP2_SME_F16F32;
	if (((id_aa64smfr0_el1 >> 34) & 0x1) == 1)
		*hwcap2 |= HWCAP2_SME_B16F32;
	if (((id_aa64smfr0_el1 >> 32) & 0x1) == 1)
		*hwcap2 |= HWCAP2_SME_F32F32;
	if (((id_aa64smfr0_el1 >> 63) & 0x1) == 1)
		*hwcap2 |= HWCAP2_SME_FA64;
#endif
	if (((id_aa64isar2_el1 >> 0) & 0xF) == 2)
		*hwcap2 |= HWCAP2_WFXT;
	if (((id_aa64isar1_el1 >> 44) & 0xF) == 2)
		*hwcap2 |= HWCAP2_EBF16;
#if 0
	if (((id_aa64zfr0_el1 >> 20) & 0xF) == 2)
		*hwcap2 |= HWCAP2_SVE_EBF16;
#endif
	if (((id_aa64isar2_el1 >> 52) & 0xF) == 2)
		*hwcap2 |= HWCAP2_CSSC;
	if (((id_aa64isar2_el1 >> 48) & 0xF) == 2)
		*hwcap2 |= HWCAP2_RPRFM;
#if 0
	if (((id_aa64zfr0_el1 >> 0) & 0xF) == 2)
		*hwcap2 |= HWCAP2_SVE2P1;
#endif
#if 0
	if (((id_aa64smfr0_el1 >> 56) & 0xF) == 1)
		*hwcap2 |= HWCAP2_SME2;
	if (((id_aa64smfr0_el1 >> 56) & 0xF) == 2)
		*hwcap2 |= HWCAP2_SME2P1;
	if (((id_aa64smfr0_el1 >> 44) & 0xF) == 5)
		*hwcap2 |= HWCAP2_SMEI16I32;
	if (((id_aa64smfr0_el1 >> 33) & 0x1) == 1)
		*hwcap2 |= HWCAP2_SMEBI32I32;
	if (((id_aa64smfr0_el1 >> 43) & 0x1) == 1)
		*hwcap2 |= HWCAP2_SMEB16B16;
	if (((id_aa64smfr0_el1 >> 42) & 0x1) == 1)
		*hwcap2 |= HWCAP2_SMEF16F16;
#endif
	if (((id_aa64isar2_el1 >> 16) & 0xF) == 1)
		*hwcap2 |= HWCAP2_MOPS;
	if (((id_aa64isar2_el1 >> 20) & 0xF) == 1)
		*hwcap2 |= HWCAP2_HBC;
#if 0
	if (((id_aa64zfr0_el1 >> 24) & 0xF) == 1)
		*hwcap2 |= HWCAP2_SVE_B16B16;
#endif
	if (((id_aa64isar1_el1 >> 20) & 0xF) == 3)
		*hwcap2 |= HWCAP2_LRCPC3;
	if (((id_aa64isar0_el1 >> 20) & 0xF) == 3)
		*hwcap2 |= HWCAP2_LSE128;
#if 0
	if (((id_aa64pfr2_el1 >> 32) & 0xF) == 1)
		*hwcap2 |= HWCAP2_FPMR;
#endif
	if (((id_aa64isar2_el1 >> 56) & 0xF) == 1)
		*hwcap2 |= HWCAP2_LUT;
	if (((id_aa64isar3_el1 >> 4) & 0xF) == 1)
		*hwcap2 |= HWCAP2_FAMINMAX;
#if 0
	if (((id_aa64fpfr0_el1 >> 31) & 0x1) == 1)
		*hwcap2 |= HWCAP2_F8CVT;
	if (((id_aa64fpfr0_el1 >> 30) & 0x1) == 1)
		*hwcap2 |= HWCAP2_F8FMA;
	if (((id_aa64fpfr0_el1 >> 29) & 0x1) == 1)
		*hwcap2 |= HWCAP2_F8DP4;
	if (((id_aa64fpfr0_el1 >> 28) & 0x1) == 1)
		*hwcap2 |= HWCAP2_F8DP2;
	if (((id_aa64fpfr0_el1 >> 1) & 0x1) == 1)
		*hwcap2 |= HWCAP2_F8E4M3;
	if (((id_aa64fpfr0_el1 >> 0) & 0x1) == 1)
		*hwcap2 |= HWCAP2_F8E5M2;
#endif
#if 0
	if (((id_aa64smfr0_el1 >> 60) & 0x1) == 1)
		*hwcap2 |= HWCAP2_SME_LUTV2;
	if (((id_aa64smfr0_el1 >> 41) & 0x1) == 1)
		*hwcap2 |= HWCAP2_SME_F8F16;
	if (((id_aa64smfr0_el1 >> 40) & 0x1) == 1)
		*hwcap2 |= HWCAP2_SME_F8F32;
	if (((id_aa64smfr0_el1 >> 30) & 0x1) == 1)
		*hwcap2 |= HWCAP2_SME_SF8FMA;
	if (((id_aa64smfr0_el1 >> 29) & 0x1) == 1)
		*hwcap2 |= HWCAP2_SME_SF8DP4;
	if (((id_aa64smfr0_el1 >> 28) & 0x1) == 1)
		*hwcap2 |= HWCAP2_SME_SF8DP2;
#endif
#endif
	auxv[13 * 2 + 0] = AT_NULL;
	auxv[13 * 2 + 1] = AT_NULL;
	return 0;
}

int kproc_create(const char *name, void *entry,
                 const char * const *argv,
                 const char * const *envp,
                 struct thread **threadp)
{
	struct vm_space *vm_space = vm_space_alloc();
	if (!vm_space)
		return -ENOMEM;
	struct thread *thread;
	int ret = proc_create(name, vm_space, entry, &thread);
	if (ret)
	{
		vm_space_free(vm_space);
		return ret;
	}
	arch_init_trapframe_kern(thread);
	ret = proc_push_init_args(thread, argv, envp, NULL, 0);
	if (ret)
	{
		/* XXX cleanup */
		return ret;
	}
	thread->pri = PRI_KERN;
	*threadp = thread;
	return 0;
}

static int uproc_create_elf(const char *name, struct file *file,
                            const char * const *argv,
                            const char * const *envp,
                            struct thread **threadp)
{
	struct vm_space *vm_space = vm_space_alloc();
	if (!vm_space)
		return -ENOMEM;
	size_t auxv[AUXV_SIZE * 2];
	struct elf_info info;
	int ret = elf_createctx(file, vm_space, 0, NULL, NULL, NULL, &info);
	if (ret)
	{
		TRACE("failed to create elf context");
		vm_space_free(vm_space);
		return ret;
	}
	struct thread *thread;
	ret = proc_create(name, vm_space, (void*)info.real_entry, &thread);
	if (ret)
	{
		TRACE("failed to create proc");
		vm_space_free(vm_space);
		return ret;
	}
	arch_init_trapframe_user(thread);
	ret = create_auxv(auxv, thread, &info);
	if (ret)
	{
		TRACE("failed to create auxv");
		proc_free(thread->proc);
		return ret;
	}
	ret = proc_push_init_args(thread, argv, envp, auxv,
	                          sizeof(auxv) / sizeof(*auxv) / 2);
	if (ret)
	{
		TRACE("failed to push argv / envp");
		proc_free(thread->proc);
		return ret;
	}
	thread->pri = PRI_USER;
	*threadp = thread;
	return 0;
}

static int proc_dup(struct proc *proc, int flags, struct proc **newprocp)
{
	struct proc *newp;
	int ret = proc_alloc(&newp);
	if (ret)
		return ret;
	newp->name = strdup(proc->name);
	if (!newp->name)
	{
		sma_free(&proc_sma, newp);
		return -ENOMEM;
	}
	if (flags & CLONE_VM)
	{
		refcount_inc(&proc->vm_space->refcount);
		newp->vm_space = proc->vm_space;
	}
	else
	{
		newp->vm_space = vm_space_dup(proc->vm_space);
		if (!newp->vm_space)
		{
			free(newp->name);
			sma_free(&proc_sma, newp);
			return -ENOMEM;
		}
	}
	newp->entrypoint = proc->entrypoint;
	newp->files = malloc(sizeof(*newp->files) * proc->files_nb, 0);
	if (!newp->files)
	{
		vm_space_free(newp->vm_space);
		free(newp->name);
		sma_free(&proc_sma, newp);
		return -ENOMEM;
	}
	for (size_t i = 0; i < proc->files_nb; ++i)
	{
		newp->files[i] = proc->files[i];
		if (newp->files[i].file)
			file_ref(newp->files[i].file);
	}
	newp->files_nb = proc->files_nb;
	newp->root = proc->root;
	newp->cwd = proc->cwd;
	node_ref(newp->root);
	node_ref(newp->cwd);
	newp->umask = proc->umask;
	newp->pid = __atomic_add_fetch(&g_pid, 1, __ATOMIC_SEQ_CST);
	newp->pgrp = proc->pgrp;
	pgrp_ref(newp->pgrp);
	pgrp_lock(newp->pgrp);
	TAILQ_INSERT_TAIL(&newp->pgrp->processes, newp, pgrp_chain);
	pgrp_unlock(newp->pgrp);
	newp->cred = proc->cred;
	newp->state = proc->state;
	newp->parent = proc;
	TAILQ_INSERT_TAIL(&proc->childs, newp, child_chain);
	for (size_t i = 0; i < SIGLAST; ++i)
		newp->sigactions[i] = proc->sigactions[i];
	*newprocp = newp;
	return 0;
}

static int thread_dup(struct thread *thread, struct thread **newthreadp)
{
	struct thread *newt;
	int ret = thread_alloc(&newt);
	if (ret)
		return ret;
	assert(thread->state == THREAD_RUNNING, "cloning thread isn't in running state");
	newt->state = THREAD_PAUSED;
	memcpy(&newt->tf_user, &thread->tf_user, sizeof(newt->tf_user));
	memcpy(&newt->tf_kern, &thread->tf_kern, sizeof(newt->tf_kern));
	newt->tf_nest_level = thread->tf_nest_level;
	newt->stack_size = thread->stack_size;
	newt->stack = thread->stack;
	newt->sigmask = thread->sigmask;
	newt->affinity = thread->affinity;
	newt->tls_addr = thread->tls_addr;
	newt->sigaltstack = thread->sigaltstack;
	newt->sigaltstack_nest = thread->sigaltstack_nest;
	newt->pri = thread->pri;
	*newthreadp = newt;
	return 0;
}

int uproc_clone(struct thread *thread, int flags, struct thread **newthreadp)
{
	struct proc *newp;
	int ret = proc_dup(thread->proc, flags, &newp);
	if (ret)
		return ret;
	struct thread *newt;
	ret = thread_dup(thread, &newt);
	if (ret)
	{
		/* XXX cleanup newp */
		return ret;
	}
	newt->proc = newp;
	newt->tid = newp->pid;
	TAILQ_INSERT_TAIL(&newp->threads, newt, thread_chain);
	spinlock_lock(&g_thread_list_lock);
	TAILQ_INSERT_TAIL(&g_thread_list, newt, chain);
	spinlock_unlock(&g_thread_list_lock);
	spinlock_lock(&g_proc_list_lock);
	TAILQ_INSERT_TAIL(&g_proc_list, newp, chain);
	spinlock_unlock(&g_proc_list_lock);
	*newthreadp = newt;
	return 0;
}

int uthread_clone(struct thread *thread, int flags, struct thread **newthreadp)
{
	(void)flags;
	struct thread *newt;
	int ret = thread_dup(thread, &newt);
	if (ret)
	{
		/* XXX cleanup newp */
		return ret;
	}
	newt->proc = thread->proc;
	newt->tid = __atomic_add_fetch(&g_pid, 1, __ATOMIC_SEQ_CST);
	TAILQ_INSERT_TAIL(&thread->proc->threads, newt, thread_chain);
	spinlock_lock(&g_thread_list_lock);
	TAILQ_INSERT_TAIL(&g_thread_list, newt, chain);
	spinlock_unlock(&g_thread_list_lock);
	*newthreadp = newt;
	return 0;
}

void proc_wakeup_vfork(struct proc *proc)
{
	spinlock_lock(&proc->vfork_waitq_lock);
	waitq_broadcast(&proc->vfork_waitq, 0);
	proc->vfork_rel->vfork_rel = NULL;
	proc->vfork_rel = NULL;
	spinlock_unlock(&proc->vfork_waitq_lock);
}

void proc_wakeup_wait(struct proc *proc, struct thread *source)
{
	spinlock_lock(&proc->wait_waitq_lock);
	source->waitable = 1;
	waitq_broadcast(&proc->wait_waitq, 0);
	spinlock_unlock(&proc->wait_waitq_lock);
	proc_signal(proc, SIGCHLD);
}

static void proc_itimer_cb(struct timer *timer)
{
	proc_test_itimer(timer->userdata, ITIMER_REAL);
}

void proc_test_itimer(struct proc *proc, int which)
{
	struct itimerval *itimer;
	struct timeval tv;

	switch (which)
	{
		case ITIMER_REAL:
			itimer = &proc->itimer_real;
			break;
		case ITIMER_VIRTUAL:
			itimer = &proc->itimer_virt;
			break;
		case ITIMER_PROF:
			itimer = &proc->itimer_prof;
			break;
		default:
			panic("invalid itimer\n");
	}
	if (!itimer->it_value.tv_sec && !itimer->it_value.tv_usec)
	{
		if (which == ITIMER_REAL && proc->itimer_timer.queued)
			timer_remove(&proc->itimer_timer);
		return;
	}
	switch (which)
	{
		case ITIMER_REAL:
		{
			struct timespec ts;
			if (clock_gettime(CLOCK_REALTIME, &ts))
				panic("failed to get realtime\n"); /* XXX */
			timeval_from_timespec(&tv, &ts);
			break;
		}
		case ITIMER_VIRTUAL:
			timeval_from_timespec(&tv, &proc->stats.utime);
			break;
		case ITIMER_PROF:
		{
			struct timeval tmp;
			timeval_from_timespec(&tv, &proc->stats.utime);
			timeval_from_timespec(&tmp, &proc->stats.stime);
			timeval_add(&tv, &tv, &tmp);
			break;
		}
		default:
			panic("invalid itimer\n");
	}
	if (timeval_cmp(&tv, &itimer->it_value) < 0)
	{
		if (which == ITIMER_REAL && !proc->itimer_timer.queued)
		{
			struct timespec ts;
			timespec_from_timeval(&ts, &itimer->it_value);
			timer_add(&proc->itimer_timer, CLOCK_REALTIME, ts,
			          proc_itimer_cb, proc);
		}
		return;
	}
	switch (which)
	{
		case ITIMER_REAL:
			proc_signal(proc, SIGALRM);
			break;
		case ITIMER_VIRTUAL:
			proc_signal(proc, SIGVTALRM);
			break;
		case ITIMER_PROF:
			proc_signal(proc, SIGPROF);
			break;
		default:
			panic("invalid itimer\n");
	}
	if (itimer->it_interval.tv_sec || itimer->it_interval.tv_usec)
	{
		struct timespec ts;
		timeval_add(&itimer->it_value, &itimer->it_value,
		            &itimer->it_interval);
		if (proc->itimer_timer.queued)
			timer_remove(&proc->itimer_timer);
		timespec_from_timeval(&ts, &itimer->it_value);
		timer_add(&proc->itimer_timer, CLOCK_REALTIME, ts,
		          proc_itimer_cb, proc);
	}
	else
	{
		itimer->it_value.tv_sec = 0;
		itimer->it_value.tv_usec = 0;
	}
}

static int exec_interp(struct file *file, struct vm_space *vm_space,
                       const char *path, char **pre,
                       const char * const **argv,
                       struct elf_info *info)
{
	char line[256];
	ssize_t ret = file_readseq(file, line, sizeof(line), 0);
	if (ret < 0)
		return ret;
	char *nl = memchr(line, '\n', ret);
	if (!nl)
		return -ENOEXEC;
	char *exec = &line[2];
	while (exec < nl && isspace(*exec))
		exec++;
	char *arg;
	if (exec == nl)
		return -ENOEXEC;
	arg = exec;
	while (arg < nl && !isspace(*arg))
		arg++;
	*arg = '\0';
	strlcpy(pre[0], exec, 256);
	int pre_nb = 1;
	char *tmp = arg + 1;
	while (tmp < nl && isspace(*tmp))
		tmp++;
	if (tmp < nl)
	{
		*tmp = '\0';
		strlcpy(pre[pre_nb++], arg, 256);
	}
	if (path)
	{
		strlcpy(pre[pre_nb++], path, 256);
		if (**argv)
			(*argv)++;
	}
	pre[pre_nb++] = NULL;
	struct node *interp_node;
	ret = vfs_getnode(NULL, exec, 0, &interp_node);
	if (ret)
		return ret;
	struct file *interp;
	ret = file_fromnode(interp_node, O_RDONLY, &interp);
	node_free(interp_node);
	if (ret)
		return ret;
	ret = elf_createctx(interp, vm_space, 0, NULL, NULL, NULL, info);
	file_free(interp);
	return ret;
}

int uproc_execve(struct thread *thread, struct file *file, const char *path,
                 const char * const *argv,
                 const char * const *envp)
{
	struct vm_space *vm_space = NULL;
	struct vm_zone *zone;
	char *newname = NULL;
	uint8_t magic[4];
	ssize_t ret;

	newname = strdup(argv[0]);
	if (!newname)
	{
		ret = -ENOMEM;
		goto err;
	}
	vm_space = vm_space_alloc();
	if (!vm_space)
	{
		ret = -ENOMEM;
		goto err;
	}
	ret = file_readseq(file, magic, sizeof(magic), 0);
	if (ret < 0)
		goto err;
	mutex_lock(&vm_space->mutex);
	ret = vm_alloc(vm_space,
	               vm_space->region.addr
	             + vm_space->region.size
	             - thread->stack_size,
	               0, thread->stack_size, 0, VM_PROT_RW, MAP_PRIVATE,
	               NULL, &zone);
	mutex_unlock(&vm_space->mutex);
	if (ret)
		goto err;
	void *stack = (void*)zone->addr;
	uintptr_t sp;
	int stack_argc;
	char **stack_argv;
	char **stack_envp;
	size_t *stack_auxv;
	struct elf_info info;
	size_t auxv[AUXV_SIZE * 2];
	if (magic[0] == '#' && magic[1] == '!')
	{
		char pre0[256];
		char pre1[256];
		char pre2[256];
		char *pre[] = {pre0, pre1, pre2, NULL};
		ret = exec_interp(file, vm_space, path, pre, &argv, &info);
		if (ret)
			goto err;
		ret = create_auxv(auxv, thread, &info);
		if (ret)
			goto err;
		ret = mem_push_init_args(vm_space, stack, thread->stack_size,
		                         (const char * const*)pre,
		                         argv, envp, auxv, AUXV_SIZE,
		                         &stack_argc, &stack_argv,
		                         &stack_envp, &stack_auxv,
		                         &sp);
		if (ret)
			goto err;
	}
	else
	{
		ret = elf_createctx(file, vm_space, 0, NULL, NULL, NULL, &info);
		if (ret)
			goto err;
		ret = create_auxv(auxv, thread, &info);
		if (ret)
			goto err;
		ret = mem_push_init_args(vm_space, stack, thread->stack_size, NULL,
		                         argv, envp, auxv, AUXV_SIZE,
		                         &stack_argc, &stack_argv,
		                         &stack_envp, &stack_auxv,
		                         &sp);
		if (ret)
			goto err;
	}
	free(thread->proc->name);
	thread->proc->name = newname;
	struct vm_space *oldctx = thread->proc->vm_space;
	thread->proc->vm_space = vm_space;
	thread->proc->entrypoint = (void*)info.real_entry;
	thread->stack = stack;
	thread->from_syscall = 0;
	arch_init_trapframe_user(thread);
	arch_set_stack_pointer(&thread->tf_user, sp);
#if ARCH_REGISTER_PARAMETERS >= 1
	arch_set_argument0(&thread->tf_user, (uintptr_t)stack_argc);
#endif
#if ARCH_REGISTER_PARAMETERS >= 2
	arch_set_argument1(&thread->tf_user, (uintptr_t)stack_argv);
#endif
#if ARCH_REGISTER_PARAMETERS >= 3
	arch_set_argument2(&thread->tf_user, (uintptr_t)stack_envp);
#endif
#if ARCH_REGISTER_PARAMETERS >= 4
	arch_set_argument3(&thread->tf_user, (uintptr_t)stack_auxv);
#endif
	arch_vm_setspace(thread->proc->vm_space); /* XXX move at another place */
	vm_space_free(oldctx);
	rwlock_wrlock(&thread->proc->files_lock);
	for (size_t i = 0; i < thread->proc->files_nb; ++i)
	{
		struct filedesc *fd = &thread->proc->files[i];
		if (!fd->file)
			continue;
		if (!fd->cloexec)
			continue;
		struct file *f = fd->file;
		fd->file = NULL;
		/* XXX don't do this inside the rwlock */
		file_free(f);
	}
	rwlock_unlock(&thread->proc->files_lock);
	for (size_t i = 0; i < SIGLAST; ++i)
		thread->proc->sigactions[i].sa_handler = (void*)SIG_DFL;
	return 0;

err:
	free(newname);
	vm_space_free(vm_space);
	return ret;
}

struct proc *getproc(pid_t pid)
{
	struct proc *proc;
	if (pid <= 0)
		return NULL;
	spinlock_lock(&g_proc_list_lock);
	TAILQ_FOREACH(proc, &g_proc_list, chain)
	{
		if (proc->pid == pid)
			break;
	}
	spinlock_unlock(&g_proc_list_lock);
	return proc;
}

struct thread *getthread(pid_t tid)
{
	struct thread *thread;
	struct proc *proc;
	if (tid <= 0)
		return NULL;
	spinlock_lock(&g_proc_list_lock);
	TAILQ_FOREACH(proc, &g_proc_list, chain)
	{
		thread = proc_getthread(proc, tid);
		if (thread)
			break;
	}
	spinlock_unlock(&g_proc_list_lock);
	return thread;
}

struct proc *proc_getchild(struct proc *proc, pid_t pid)
{
	struct proc *child;
	TAILQ_FOREACH(child, &proc->childs, child_chain)
	{
		if (child->pid == pid)
			break;
	}
	return child;
}

struct thread *proc_getthread(struct proc *proc, pid_t tid)
{
	struct thread *thread;
	TAILQ_FOREACH(thread, &proc->threads, thread_chain)
	{
		if (thread->tid == tid)
		{
			thread_ref(thread);
			break;
		}
	}
	return thread;
}

static int create_signal_stack(struct thread *thread, const siginfo_t *siginfo)
{
	struct sigaction *action = &thread->proc->sigactions[siginfo->si_signo];
	uintptr_t sp;
	if (!thread->sigaltstack_nest && (action->sa_flags & SA_ONSTACK))
	{
		sp = (uintptr_t)thread->sigaltstack.ss_sp;
		if (__builtin_add_overflow(sp, thread->sigaltstack.ss_size, &sp))
			return proc_exit(thread->proc, SIGSEGV);
	}
	else
	{
		sp = arch_get_stack_pointer(&thread->tf_user);
#if defined(__x86_64__)
		if (sp < 128)
			return proc_exit(thread->proc, SIGSEGV);
		sp -= 128; /* red zone */
#endif
	}
	size_t ctx_size = sizeof(struct trapframe) + sizeof(thread->sigmask);
	if (action->sa_flags & SA_SIGINFO)
		ctx_size += sizeof(siginfo_t);
	size_t call_stack_size = 0;
#if ARCH_REGISTER_PARAMETERS < 1
	ctx_size += sizeof(uintptr_t);
	call_stack_size += sizeof(uintptr_t);
#endif
	/* NB: we always push arguments because it's easier for sigreturn */
#if ARCH_REGISTER_PARAMETERS < 2
	ctx_size += sizeof(uintptr_t);
	call_stack_size += sizeof(uintptr_t);
#endif
#if ARCH_REGISTER_PARAMETERS < 3
	ctx_size += sizeof(uintptr_t);
	call_stack_size += sizeof(uintptr_t);
#endif
	ctx_size += ((uintptr_t)sp - ctx_size) % ARCH_STACK_ALIGNMENT;
#if ARCH_STACK_RETURN_ADDR
	ctx_size += sizeof(uintptr_t);
	call_stack_size += sizeof(uintptr_t);
#endif
	if (sp < ctx_size)
		return proc_exit(thread->proc, SIGSEGV);
	size_t map_base = (sp - ctx_size) & ~PAGE_MASK;
	size_t map_size = (((sp - 1) & ~PAGE_MASK) - map_base) + PAGE_SIZE;
	void *ptr;
	int ret = vm_map_user(thread->proc->vm_space, map_base,
	                      map_size, VM_PROT_RW, &ptr);
	if (ret == -EFAULT)
		return proc_exit(thread->proc, SIGSEGV);
	if (ret)
		return ret;
	uint8_t *dst = ptr;
	dst += sp - map_base;
	dst -= ctx_size;
#if ARCH_STACK_RETURN_ADDR
	*(uintptr_t*)dst = (uintptr_t)action->sa_restorer;
	dst += sizeof(uintptr_t);
#endif
#if ARCH_REGISTER_PARAMETERS < 1
	*(uintptr_t*)dst = (uintptr_t)siginfo->si_signo;
	dst += sizeof(uintptr_t);
#endif
	uintptr_t ucontext_addr = sp - ctx_size + call_stack_size;
	uintptr_t siginfo_addr = ucontext_addr
	                       + sizeof(struct trapframe)
	                       + sizeof(thread->sigmask);
	if (action->sa_flags & SA_SIGINFO)
	{
#if ARCH_REGISTER_PARAMETERS < 2
		*(uintptr_t*)dst = siginfo_addr;
		dst += sizeof(uintptr_t);
#endif
#if ARCH_REGISTER_PARAMETERS < 3
		*(uintptr_t*)dst = ucontext_addr;
		dst += sizeof(uintptr_t);
#endif
	}
	else
	{
#if ARCH_REGISTER_PARAMETERS < 2
		*(uintptr_t*)dst = 0;
		dst += sizeof(uintptr_t);
#endif
#if ARCH_REGISTER_PARAMETERS < 3
		*(uintptr_t*)dst = 0;
		dst += sizeof(uintptr_t);
#endif
	}
	memcpy(dst, &thread->tf_user, sizeof(struct trapframe));
	dst += sizeof(struct trapframe);
	le64enc(dst, thread->sigmask);
	dst += sizeof(thread->sigmask);
	arch_set_stack_pointer(&thread->tf_user, sp - ctx_size);
#if !ARCH_STACK_RETURN_ADDR
	arch_set_return_address(&thread->tf_user, (uintptr_t)action->sa_restorer);
#endif
#if ARCH_REGISTER_PARAMETERS >= 1
	arch_set_argument0(&thread->tf_user, siginfo->si_signo);
#endif
#if ARCH_REGISTER_PARAMETERS >= 2
	arch_set_argument1(&thread->tf_user, siginfo_addr);
#endif
#if ARCH_REGISTER_PARAMETERS >= 3
	arch_set_argument2(&thread->tf_user, ucontext_addr);
#endif
	if (action->sa_flags & SA_SIGINFO)
	{
		arch_set_instruction_pointer(&thread->tf_user,
		                             (uintptr_t)action->sa_sigaction);
		memcpy(dst, siginfo, sizeof(*siginfo));
		dst += sizeof(*siginfo);
	}
	else
	{
		arch_set_instruction_pointer(&thread->tf_user,
		                             (uintptr_t)action->sa_handler);
	}
	vm_unmap(ptr, map_size);
	return 0;
}

static int thread_handle_signal(struct thread *thread, int signum)
{
	struct sigaction *action = &thread->proc->sigactions[signum];
	if ((uintptr_t)action->sa_handler == SIG_IGN)
		return -EAGAIN;
	if ((uintptr_t)action->sa_handler == SIG_DFL)
	{
		switch (signum)
		{
			case SIGHUP:
			case SIGINT:
			case SIGKILL:
			case SIGUSR1:
			case SIGUSR2:
			case SIGPIPE:
			case SIGALRM:
			case SIGTERM:
			case SIGVTALRM:
			case SIGPROF:
			case SIGPOLL: /* terminate */
				return proc_exit(thread->proc, signum);
			case SIGQUIT:
			case SIGILL:
			case SIGTRAP:
			case SIGABRT:
			case SIGBUS:
			case SIGFPE:
			case SIGSEGV:
			case SIGXCPU:
			case SIGXFSZ: /* core */
				/* XXX dump core */
				return proc_exit(thread->proc, signum);
			case SIGCHLD:
			case SIGURG:
			case SIGWINCH: /* ignore */
				return -EAGAIN;
			case SIGCONT: /* continue */
				return proc_cont(thread->proc);
			case SIGSTOP:
			case SIGTSTP:
			case SIGTTIN:
			case SIGTTOU: /* stop */
				return proc_stop(thread->proc);
		}
	}
	siginfo_t siginfo; /* XXX siginfo should be queued inside the thread */
	memset(&siginfo, 0, sizeof(siginfo));
	siginfo.si_signo = signum;
	int ret = create_signal_stack(thread, &siginfo);
	/* XXX get a better way to handle SS_ONSTACK */
	if (!ret && (action->sa_flags & SA_ONSTACK))
	{
		thread->sigaltstack.ss_flags |= SS_ONSTACK;
		thread->sigaltstack_nest++;
	}
	thread->sigmask = le64dec(action->sa_mask.set);
	thread->sigmask &= ~(1 << SIGSTOP);
	thread->sigmask &= ~(1 << SIGKILL);
	if (!(action->sa_flags & SA_NODEFER))
		thread->sigmask |= 1 << signum;
	return ret;
}

int thread_handle_signals(struct thread *thread)
{
	if (thread->state == THREAD_ZOMBIE)
		return 0;
	if (!thread->sigqueue)
		return 0;
	for (size_t i = 1; i <= SIGLAST; ++i)
	{
		if (!(thread->sigqueue & (1 << i)))
			continue;
		if (thread->ptrace_state == PTRACE_ST_SYSCALL
		 || thread->ptrace_state == PTRACE_ST_RUNNING)
		{
			thread->sigqueue &= ~(1 << i);
			thread_ptrace_stop(thread, i);
			if (!(thread->sigqueue & (1 << i)))
				return thread_handle_signals(thread);
		}
		int ret = thread_handle_signal(thread, i);
		thread->sigqueue &= ~(1 << i);
		if (ret != -EAGAIN)
			return ret;
	}
	return 0;
}

int thread_signal(struct thread *thread, int signum)
{
	if (thread->sigmask & (1 << signum))
		return 0;
	thread->stats.nsignals++;
	thread->proc->stats.nsignals++;
	thread->sigqueue |= (1 << signum);
	if (thread->waitq)
	{
		/* XXX vfork & ptrace should put thread in STOPPED state instead of
		 * WAITING state
		 * it would easy differentiation between restartable and
		 * non restartable states, avoiding this kind of hacks
		 */
		if (thread->ptrace_state != PTRACE_ST_STOPPED
		 && (thread->state != THREAD_WAITING || !thread->proc->vfork_rel))
			waitq_wakeup_thread(thread->waitq, thread, -EINTR);
	}
	return 0;
}

int proc_signal(struct proc *proc, int signum)
{
	struct thread *thread;
	TAILQ_FOREACH(thread, &proc->threads, thread_chain)
	{
		if (thread->sigmask & (1 << signum))
			continue;
		return thread_signal(thread, signum);
	}
	return 0;
}

int thread_exit(struct thread *thread, int code)
{
	/* XXX is it really the case ? */
	struct thread *lead = TAILQ_FIRST(&thread->proc->threads);
	if (thread == lead)
		return proc_exit(thread->proc, code);
	thread->state = THREAD_ZOMBIE;
	thread_untrace(thread);
	proc_wakeup_wait(thread->proc, thread);
	if (curcpu()->thread == thread)
		sched_resched();
	return 0;
}

int proc_exit(struct proc *proc, int code)
{
	if (proc == g_initthread->proc)
		panic("init killed (%d)\n", code);
	TAILQ_FIRST(&proc->threads)->wstatus = code;
	proc->state = PROC_ZOMBIE; /* XXX really ? */
	cpumask_t cpu_sync_cpumask;
	CPUMASK_CLEAR(&cpu_sync_cpumask);
	int cpu_sync_required = 0;
	struct thread *thread;
	TAILQ_FOREACH(thread, &proc->threads, thread_chain)
	{
		if (thread != curcpu()->thread)
		{
			switch (thread->state)
			{
				case THREAD_PAUSED:
					break;
				case THREAD_RUNNING:
					CPUMASK_SET(&cpu_sync_cpumask, thread->running_cpuid, 1);
					cpu_sync_required = 1;
					break;
				case THREAD_WAITING:
					/* XXX
					 * if we unblock a waiting thread, we *MUST NOT*
					 * unschedule the thread because the thread must
					 * cleanup its kernel stack & stuff
					 *
					 * the thread should be effectively killed
					 * (i.e: migrated to zombie) on kernel-to-userland
					 * return (by detecting its proc is in zombie state)
					 */
					waitq_wakeup_thread(thread->waitq, thread, -EINTR); /* XXX another errno ? */
					break;
				case THREAD_STOPPED:
				case THREAD_ZOMBIE:
					break;
			}
		}
		thread->state = THREAD_ZOMBIE;
		thread_untrace(thread);
	}
	if (cpu_sync_required)
		cpu_sync(&cpu_sync_cpumask);
	struct thread *tracee;
	TAILQ_FOREACH(tracee, &proc->ptrace_tracees, ptrace_chain)
	{
		spinlock_lock(&tracee->ptrace_waitq_lock);
		if (tracee->waitq == &tracee->ptrace_waitq)
			waitq_signal(&tracee->ptrace_waitq, 0);
		thread_untrace(tracee);
		if (tracee->ptrace_options & PTRACE_O_EXITKILL)
			proc_signal(tracee->proc, SIGKILL);
		spinlock_unlock(&tracee->ptrace_waitq_lock);
	}
	if (proc->vfork_rel)
		proc_wakeup_vfork(proc->vfork_rel);
	rwlock_wrlock(&proc->files_lock);
	for (size_t i = 0; i < proc->files_nb; ++i)
	{
		struct filedesc *fd = &proc->files[i];
		if (!fd->file)
			continue;
		file_free(fd->file);
		fd->file = NULL;
	}
	rwlock_unlock(&proc->files_lock);
	if (refcount_get(&proc->vm_space->refcount) == 1) /* XXX make it non-racy */
		vm_space_cleanup(proc->vm_space);
	if (proc->parent)
		proc_wakeup_wait(proc->parent, TAILQ_FIRST(&proc->threads));
	/* XXX delete directly if no parent / parent ignore SIGCHLD ? */
	if (curcpu()->thread->proc == proc)
		sched_resched();
	return 0;
}

int proc_stop(struct proc *proc)
{
	if (proc->state != PROC_ALIVE)
		return 0;
	proc->state = PROC_STOPPED;
	cpumask_t cpu_sync_cpumask;
	CPUMASK_CLEAR(&cpu_sync_cpumask);
	int cpu_sync_required = 0;
	struct thread *thread;
	TAILQ_FOREACH(thread, &proc->threads, thread_chain)
	{
		if (thread == curcpu()->thread)
			continue;
		if (thread->state == THREAD_RUNNING)
		{
			CPUMASK_SET(&cpu_sync_cpumask, thread->running_cpuid, 1);
			cpu_sync_required = 1;
			thread->state = THREAD_STOPPED;
		}
	}
	if (cpu_sync_required)
		cpu_sync(&cpu_sync_cpumask);
	proc_wakeup_wait(proc->parent, TAILQ_FIRST(&proc->threads));
	return 0;
}

int proc_cont(struct proc *proc)
{
	if (proc->state != PROC_STOPPED)
		return 0;
	proc->state = PROC_ALIVE;
	struct thread *thread;
	TAILQ_FOREACH(thread, &proc->threads, thread_chain)
		thread_cont(thread);
	return 0;
}

void thread_ptrace_stop(struct thread *thread, int signum)
{
	spinlock_lock(&thread->ptrace_waitq_lock);
	thread->ptrace_state = PTRACE_ST_STOPPED;
	thread->wstatus = 0x7F | (signum << 8);
	/* XXX assert there is still a tracer present */
	proc_wakeup_wait(thread->ptrace_tracer, thread);
	if (waitq_wait_head(&thread->ptrace_waitq,
	                    &thread->ptrace_waitq_lock,
	                    NULL))
		panic("ptrace waitq errno\n");
	spinlock_unlock(&thread->ptrace_waitq_lock);
}

int thread_stop(struct thread *thread)
{
	if (thread->state == THREAD_STOPPED)
		return 0;
	thread->state = THREAD_STOPPED;
	sched_dequeue(thread);
	return 0;
}

int thread_cont(struct thread *thread)
{
	if (thread->state != THREAD_STOPPED)
		return 0;
	thread->state = THREAD_PAUSED;
	sched_run(thread);
	return 0;
}

int thread_sleep(const struct timespec *duration)
{
	struct waitq waitq;
	struct spinlock spinlock;
	spinlock_init(&spinlock);
	waitq_init(&waitq);
	spinlock_lock(&spinlock);
	int ret = waitq_wait_head(&waitq, &spinlock, duration);
	spinlock_unlock(&spinlock);
	waitq_destroy(&waitq);
	spinlock_destroy(&spinlock);
	return ret;
}

void thread_trace(struct proc *tracer, struct thread *tracee)
{
	tracee->ptrace_tracer = tracer;
	TAILQ_INSERT_TAIL(&tracer->ptrace_tracees, tracee, ptrace_chain);
}

void thread_untrace(struct thread *tracee)
{
	if (!tracee->ptrace_tracer)
		return;
	TAILQ_REMOVE(&tracee->ptrace_tracer->ptrace_tracees, tracee, ptrace_chain);
	tracee->ptrace_state = PTRACE_ST_NONE;
	tracee->ptrace_tracer = NULL;
}

void thread_fault(struct thread *thread, uintptr_t addr, uint32_t prot)
{
	thread->stats.faults++;
	thread->proc->stats.faults++;
	int ret = vm_fault(thread->proc->vm_space, addr, prot);
	if (!ret)
		return;
	if (0
#if PANIC_ON_PFR
	  || (prot & VM_PROT_R)
#endif
#if PANIC_ON_PFW
	  || (prot & VM_PROT_W)
#endif
#if PANIC_ON_PFX
	  || (prot & VM_PROT_X)
#endif
	)
	{
		arch_print_regs(&thread->tf_user.regs);
		thread_print_stack_trace(thread, NULL);
		panic("CPU %" PRIu32 " TID %" PRId32 " PID %" PRId32 " "
		      "[%s] %s page fault addr 0x%0*zx @ %0*zx: %s\n",
		      curcpu()->id,
		      thread->tid,
		      thread->proc->pid,
		      thread->proc->name,
		      (prot & VM_PROT_X) ? "exec" : ((prot & VM_PROT_W) ? "write" : "read"),
		      (int)(sizeof(size_t) * 2),
		      (size_t)addr,
		      (int)(sizeof(size_t) * 2),
		      (size_t)arch_get_instruction_pointer(&thread->tf_user),
		      strerror(ret));
	}
	thread_signal(thread, (prot & VM_PROT_W) ? SIGBUS : SIGSEGV);
}

void thread_illegal_instruction(struct thread *thread)
{
#if PANIC_ON_ILL
	arch_print_regs(&thread->tf_user.regs);
	thread_print_stack_trace(thread, NULL);
	panic("CPU %" PRIu32 " TID %" PRId32 " PID %" PRId32 " "
	      "[%s] illegal instruction @ 0x%0*zx\n",
	      curcpu()->id,
	      thread->tid,
	      thread->proc->pid,
	      thread->proc->name,
	      (int)(sizeof(size_t) * 2),
	      (size_t)arch_get_instruction_pointer(&thread->tf_user));
#endif
	thread_signal(thread, SIGILL);
}

void thread_print_stack_trace(struct thread *thread, struct uio *uio)
{
	size_t i = 1;
	uintptr_t fp = arch_get_frame_pointer(&thread->tf_user);
	uintptr_t pc = arch_get_instruction_pointer(&thread->tf_user);
	uprintf(uio, "[%3u] [0x%0*zx] 0x%0*zx\n",
	        0,
	        (int)(sizeof(uintptr_t) * 2), fp,
	        (int)(sizeof(uintptr_t) * 2), pc);
	while (pc)
	{
		uintptr_t v[2];
		int ret = vm_copyin(thread->proc->vm_space, v,
		                    (void*)(fp + ARCH_FRAME_POINTER_OFFSET),
		                    sizeof(v));
		if (ret)
		{
			uprintf(uio, "[%3zu] invalid frame 0x%0*zx\n",
			        i,
			        (int)(sizeof(uintptr_t) * 2), fp);
			break;
		}
		fp = v[0];
		pc = v[1];
		uprintf(uio, "[%3zu] [0x%0*zx] 0x%0*zx\n",
		        i,
		        (int)(sizeof(uintptr_t) * 2), fp,
		        (int)(sizeof(uintptr_t) * 2), pc);
		++i;
	}
}

int proc_getfile(struct proc *proc, int fd, struct file **file)
{
	rwlock_rdlock(&proc->files_lock);
	if (fd < 0 || (size_t)fd >= proc->files_nb)
	{
		rwlock_unlock(&proc->files_lock);
		return -EBADF;
	}
	*file = proc->files[fd].file;
	if (!*file)
	{
		rwlock_unlock(&proc->files_lock);
		return -EBADF;
	}
	file_ref(*file);
	rwlock_unlock(&proc->files_lock);
	return 0;
}

int proc_allocfd(struct proc *proc, struct file *file, int cloexec)
{
	rwlock_wrlock(&proc->files_lock);
	for (size_t i = 0; i < proc->files_nb; ++i)
	{
		if (proc->files[i].file)
			continue;
		proc->files[i].file = file;
		proc->files[i].cloexec = cloexec;
		file_ref(file);
		rwlock_unlock(&proc->files_lock);
		return i;
	}
	struct filedesc *files = realloc(proc->files,
	                                 sizeof(*files) * (proc->files_nb + 1),
	                                 0);
	if (!files)
	{
		rwlock_unlock(&proc->files_lock);
		return -ENOMEM;
	}
	proc->files = files;
	int fd = proc->files_nb;
	proc->files[fd].file = file;
	proc->files[fd].cloexec = cloexec;
	proc->files_nb++;
	file_ref(file);
	rwlock_unlock(&proc->files_lock);
	return fd;
}

int proc_freefd(struct proc *proc, int fd)
{
	struct file *file;
	rwlock_wrlock(&proc->files_lock);
	if (fd < 0 || (size_t)fd >= proc->files_nb)
	{
		rwlock_unlock(&proc->files_lock);
		return -EBADF;
	}
	file = proc->files[fd].file;
	proc->files[fd].file = NULL;
	rwlock_unlock(&proc->files_lock);
	if (!file)
		return -EBADF;
	file_free(file);
	return 0;
}

void proc_update_loadavg(void)
{
	struct cpu *cpu = curcpu();
	struct timespec ts;
	struct timespec diff;
	struct timespec loadavg_time;
	struct timespec loadavg_last_idle;
	struct timespec idlethread_utime;
	uint64_t delta;

	if (clock_gettime(CLOCK_MONOTONIC, &ts))
	{
		TRACE("loadavg: failed to get time");
		return;
	}
	loadavg_time = cpu->loadavg_time;
	timespec_sub(&diff, &ts, &loadavg_time);
	if (diff.tv_sec <= 0 && diff.tv_nsec < 1000000000)
		return;
	loadavg_time.tv_nsec += 1000000000;
	timespec_normalize(&loadavg_time);
	cpu->loadavg_time = loadavg_time;
	loadavg_last_idle = cpu->loadavg_last_idle;
	idlethread_utime = cpu->idlethread->stats.utime;
	timespec_sub(&diff, &idlethread_utime, &loadavg_last_idle);
	cpu->loadavg_last_idle = idlethread_utime;
	if (diff.tv_sec)
	{
		delta = 0;
	}
	else
	{
		delta = diff.tv_nsec / (1000000000 / 65536);
		if (delta > 65536)
			delta = 0;
		else
			delta = 65536 - delta;
	}
	/* 65536 / e ^ (1 / (60 * 1)) */
	cpu->loadavg[0] = ((cpu->loadavg[0] * 64452) + delta * 1084) >> 16;
	/* 65536 / e ^ (1 / (60 * 5)) */
	cpu->loadavg[1] = ((cpu->loadavg[1] * 65317) + delta * 219) >> 16;
	/* 65536 / e ^ (1 / (60 * 15)) */
	cpu->loadavg[2] = ((cpu->loadavg[2] * 65463) + delta * 73) >> 16;
}

static ssize_t loadavg_read(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	struct cpu *cpu;
	uint32_t loadavg_sum[3] = {0};

	CPU_FOREACH(cpu)
	{
		uprintf(uio, "cpu%" PRIu32 " %" PRIu32 ".%02" PRIu32 " %" PRIu32 ".%02" PRIu32 " %" PRIu32 ".%02" PRIu32 "\n",
		        cpu->id,
		        cpu->loadavg[0] / 65536, (cpu->loadavg[0] % 65536) / 655,
		        cpu->loadavg[1] / 65536, (cpu->loadavg[1] % 65536) / 655,
		        cpu->loadavg[2] / 65536, (cpu->loadavg[2] % 65536) / 655);
		loadavg_sum[0] += cpu->loadavg[0];
		loadavg_sum[1] += cpu->loadavg[1];
		loadavg_sum[2] += cpu->loadavg[2];
	}
	loadavg_sum[0] /= g_ncpus;
	loadavg_sum[1] /= g_ncpus;
	loadavg_sum[2] /= g_ncpus;
	uprintf(uio, "cpu %" PRIu32 ".%02" PRIu32 " %" PRIu32 ".%02" PRIu32 " %" PRIu32 ".%02" PRIu32 "\n",
	        loadavg_sum[0] / 65536, (loadavg_sum[0] % 65536) / 655,
	        loadavg_sum[1] / 65536, (loadavg_sum[1] % 65536) / 655,
	        loadavg_sum[2] / 65536, (loadavg_sum[2] % 65536) / 655);
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op loadavg_fop =
{
	.read = loadavg_read,
};

int loadavg_register_sysfs(void)
{
	return sysfs_mknode("loadavg", 0, 0, 0444, &loadavg_fop, NULL);
}

static ssize_t procinfo_read(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	struct proc *proc;
	spinlock_lock(&g_proc_list_lock);
	TAILQ_FOREACH(proc, &g_proc_list, chain)
	{
		const char *state;
		switch (proc->state)
		{
			case PROC_ALIVE:
				state = "ALIVE";
				break;
			case PROC_STOPPED:
				state = "STOPPED";
				break;
			case PROC_ZOMBIE:
				state = "ZOMBIE";
				break;
			default:
				state = "UNKNOWN";
				break;
		}
		uprintf(uio, "%5" PRId32 " %5" PRId32 " %10s %15.15s "
		        "%6" PRIu64 ".%03" PRIu64 "u "
		        "%6" PRIu64 ".%03" PRIu64 "s\n",
		        proc->pid,
		        proc->parent ? proc->parent->pid : 0,
		        state,
		        proc->name,
		        proc->stats.utime.tv_sec,
		        proc->stats.utime.tv_nsec / 1000000,
		        proc->stats.stime.tv_sec,
		        proc->stats.stime.tv_nsec / 1000000);
	}
	spinlock_unlock(&g_proc_list_lock);
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op procinfo_fop =
{
	.read = procinfo_read,
};

int procinfo_register_sysfs(void)
{
	return sysfs_mknode("procinfo", 0, 0, 0444, &procinfo_fop, NULL);
}

static ssize_t cpustat_read(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	struct timespec user_time = {0, 0};
	struct timespec sys_time = {0, 0};
	struct timespec idle_time = {0, 0};
	struct cpu *cpu;
	CPU_FOREACH(cpu)
	{
		uprintf(uio, "cpu%" PRIu32 " %" PRId64 ".%09" PRId64 " %" PRId64 ".%09" PRId64 " %" PRId64 ".%09" PRId64 "\n",
		        cpu->id,
		        cpu->user_time.tv_sec,
		        cpu->user_time.tv_nsec,
		        cpu->sys_time.tv_sec,
		        cpu->sys_time.tv_nsec,
		        cpu->idle_time.tv_sec,
		        cpu->idle_time.tv_nsec);
		timespec_add(&user_time, &user_time, &cpu->user_time);
		timespec_add(&sys_time, &sys_time, &cpu->sys_time);
		timespec_add(&idle_time, &idle_time, &cpu->idle_time);
	}
	uprintf(uio, "cpu %" PRId64 ".%09" PRId64 " %" PRId64 ".%09" PRId64 " %" PRId64 ".%09" PRId64 "\n",
	        user_time.tv_sec,
	        user_time.tv_nsec,
	        sys_time.tv_sec,
	        sys_time.tv_nsec,
	        idle_time.tv_sec,
	        idle_time.tv_nsec);
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op cpustat_fop =
{
	.read = cpustat_read,
};

int cpustat_register_sysfs(void)
{
	return sysfs_mknode("cpustat", 0, 0, 0444, &cpustat_fop, NULL);
}

int setup_idlethread(void)
{
	char name[64];
	uint32_t cpuid = curcpu()->id;
	snprintf(name, sizeof(name), "[idle%" PRIu32 "]", cpuid);
	const char *argv[] = {"idle", NULL};
	const char *envp[] = {NULL};
	struct thread *idlethread;
	int ret = kproc_create(name, idle_loop, argv, envp, &idlethread);
	if (ret)
		return ret;
	CPUMASK_CLEAR(&idlethread->affinity);
	CPUMASK_SET(&idlethread->affinity, cpuid, 1);
	idlethread->pri = 255;
	curcpu()->idlethread = idlethread;
	spinlock_lock(&g_thread_list_lock);
	TAILQ_INSERT_TAIL(&g_thread_list, idlethread, chain);
	spinlock_unlock(&g_thread_list_lock);
	idlethread->state = THREAD_RUNNING;
	return 0;
}

int setup_initthread(struct thread **thread)
{
	struct file *file;
	struct node *elf;
	int ret = vfs_getnode(NULL, "/bin/init", 0, &elf);
	if (ret)
		return ret;
	ret = file_fromnode(elf, 0, &file);
	node_free(elf);
	if (ret)
		return ret;
	const char *argv[] = {"/bin/init", NULL};
	const char *envp[] = {NULL};
	ret = uproc_create_elf("init", file, argv, envp, thread);
	if (ret)
	{
		file_free(file);
		return ret;
	}
	(*thread)->proc->parent = (*thread)->proc;
	spinlock_lock(&g_thread_list_lock);
	TAILQ_INSERT_TAIL(&g_thread_list, *thread, chain);
	spinlock_unlock(&g_thread_list_lock);
	(*thread)->tf_nest_level = 1;
	g_initthread = *thread;
	file_free(file);
	return 0;
}

struct timespec proc_time_diff(void)
{
	struct cpu *cpu = curcpu();
	struct timespec current;
	struct timespec diff;
	struct timespec last;

	clock_gettime(CLOCK_MONOTONIC, &current);
	last = cpu->last_proc_time;
	timespec_sub(&diff, &current, &last);
	cpu->last_proc_time = current;
	return diff;
}

void proc_add_time_enter(void)
{
	struct cpu *cpu = curcpu();
	struct thread *thread = cpu->thread;
	struct timespec diff = proc_time_diff();

	if (thread->tf_nest_level == 1)
	{
		timespec_add(&thread->proc->stats.utime,
		             &thread->proc->stats.utime, &diff);
		timespec_add(&thread->stats.utime,
		             &thread->stats.utime, &diff);
		if (thread == cpu->idlethread)
			timespec_add(&cpu->idle_time, &cpu->idle_time, &diff);
		else
			timespec_add(&cpu->user_time, &cpu->user_time, &diff);
	}
	else
	{
		timespec_add(&thread->proc->stats.stime,
		             &thread->proc->stats.stime, &diff);
		timespec_add(&thread->stats.stime,
		             &thread->stats.stime, &diff);
		timespec_add(&cpu->sys_time, &cpu->sys_time, &diff);
	}
}

void proc_add_time_leave(void)
{
	struct cpu *cpu = curcpu();
	struct thread *thread = cpu->thread;
	struct timespec diff = proc_time_diff();

	timespec_add(&thread->proc->stats.stime,
	             &thread->proc->stats.stime, &diff);
	timespec_add(&thread->stats.stime,
	             &thread->stats.stime, &diff);
	timespec_add(&cpu->sys_time, &cpu->sys_time, &diff);
	proc_test_itimer(thread->proc, ITIMER_VIRTUAL);
	proc_test_itimer(thread->proc, ITIMER_PROF);
}

void idle_loop(void)
{
	while (1)
	{
		arch_enable_interrupts();
		arch_wait_for_interrupt();
	}
}

void dead_loop(void)
{
	while (1)
	{
		arch_disable_interrupts();
		arch_wait_for_interrupt();
	}
}
