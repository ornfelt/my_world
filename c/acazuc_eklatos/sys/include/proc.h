#ifndef PROC_H
#define PROC_H

#include <refcount.h>
#include <signal.h>
#include <rwlock.h>
#include <mutex.h>
#include <queue.h>
#include <types.h>
#include <timer.h>
#include <time.h>
#include <cpu.h>

#define PRI_KERN 50
#define PRI_USER 100

#define FD_CLOEXEC (1 << 0)

#define CLONE_VFORK  (1 << 0)
#define CLONE_VM     (1 << 1)
#define CLONE_THREAD (1 << 2)

#define ELF_KMOD   (1 << 0)
#define ELF_INTERP (1 << 1)

struct vm_space;
struct filedesc;
struct node;
struct proc;
struct runq;

TAILQ_HEAD(sess_head, sess);
TAILQ_HEAD(pgrp_head, pgrp);
TAILQ_HEAD(proc_head, proc);
TAILQ_HEAD(thread_head, thread);

enum proc_state
{
	PROC_ALIVE,
	PROC_STOPPED,
	PROC_ZOMBIE,
};

struct sess
{
	refcount_t refcount;
	struct mutex mutex;
	pid_t id;
	struct pgrp_head groups;
	TAILQ_ENTRY(sess) chain;
};

struct pgrp
{
	refcount_t refcount;
	struct mutex mutex;
	struct sess *sess;
	pid_t id;
	struct proc_head processes;
	TAILQ_ENTRY(pgrp) chain;
};

struct filedesc
{
	struct file *file;
	int cloexec;
};

struct procstat
{
	struct timespec utime;
	struct timespec stime;
	uint64_t faults;
	uint64_t msgsnd;
	uint64_t msgrcv;
	uint64_t nsignals;
	uint64_t nctxsw;
};

struct cred
{
	gid_t *groups;
	size_t groups_nb;
	uid_t uid;
	uid_t euid;
	uid_t suid;
	gid_t gid;
	gid_t egid;
	gid_t sgid;
};

/* about waitpid:
 * any thread of a given process can wait any child of the process
 * any thread of a given process can wait any thread of the process
 * if the leader thread dies, the process it belongs to dies
 * if a non-leader thread dies, it becomes a ZOMBIE until it is waited on
 * if a process dies, all the child threads are killed except the leader
 *   one, the thread and the process become ZOMBIE until its is waited on
 */

struct proc
{
	struct vm_space *vm_space;
	enum proc_state state;
	char *name;
	void *entrypoint;
	struct filedesc *files;
	size_t files_nb;
	struct rwlock files_lock;
	struct node *root;
	struct node *cwd;
	struct proc *parent;
	struct mutex mutex;
	struct procstat stats;
	struct procstat cstats;
	struct waitq wait_waitq; /* waitq for waitpid */
	struct spinlock wait_waitq_lock;
	struct proc *vfork_rel;
	struct waitq vfork_waitq;
	struct spinlock vfork_waitq_lock;
	struct sigaction sigactions[SIGLAST + 1];
	mode_t umask;
	pid_t pid;
	struct pgrp *pgrp;
	struct cred cred;
	struct thread_head ptrace_tracees;
	struct thread_head threads;
	struct proc_head childs;
	struct itimerval itimer_real; /* XXX cache if timer are enabled */
	struct itimerval itimer_virt;
	struct itimerval itimer_prof;
	struct timer itimer_timer;
	TAILQ_ENTRY(proc) chain;
	TAILQ_ENTRY(proc) child_chain;
	TAILQ_ENTRY(proc) pgrp_chain;
};

enum ptrace_state
{
	PTRACE_ST_NONE,
	PTRACE_ST_SYSCALL, /* running, will stop at next syscall */
	PTRACE_ST_STOPPED, /* stopped because of ptrace event (waiting on ptrace_waitq) */
	PTRACE_ST_RUNNING, /* running without syscall trap */
	PTRACE_ST_ONESTEP, /* single step trap */
};

enum thread_state
{
	THREAD_RUNNING, /* currently executed */
	THREAD_PAUSED,  /* paused by scheduler */
	THREAD_WAITING, /* waiting for lock */
	THREAD_STOPPED, /* SIGSTOP / ptrace */
	THREAD_ZOMBIE,  /* dead */
};

struct thread
{
	struct proc *proc;
	enum thread_state state;
	int from_syscall;
	struct trapframe tf_user;
	struct trapframe tf_kern;
	size_t tf_nest_level;
	size_t stack_size;
	uint8_t *stack;
	size_t int_stack_size;
	uint8_t *int_stack;
	uintptr_t tls_addr;
	struct timespec wait_timeout; /* currently monotonic */
	struct waitq *waitq; /* current waitq sleeping on */
	size_t wait_cpuid;
	int wstatus;
	int waitable;
	enum ptrace_state ptrace_state;
	struct waitq ptrace_waitq; /* waitq for child to wait on when ptrace sigtrap */
	struct spinlock ptrace_waitq_lock;
	struct proc *ptrace_tracer;
	uint32_t ptrace_options;
	uint64_t sigqueue;
	uint64_t sigmask;
	stack_t sigaltstack;
	size_t sigaltstack_nest;
	ssize_t running_cpuid;
	struct procstat stats;
	int waitq_ret;
	int *futex_addr;
	cpumask_t affinity;
	struct mutex mutex;
	pid_t tid;
	pri_t pri;
	struct runq *runq;
	refcount_t refcount;
	TAILQ_ENTRY(thread) chain;
	TAILQ_ENTRY(thread) thread_chain;
	TAILQ_ENTRY(thread) runq_chain;
	TAILQ_ENTRY(thread) waitq_chain;
	TAILQ_ENTRY(thread) waitq_timeout_chain;
	TAILQ_ENTRY(thread) ptrace_chain;
};

/* XXX this should be moved somewhere else */
struct elf_info
{
	size_t base_addr; /* the virtual 0 of the elf file */
	size_t map_base; /* the address of the first map (base_addr + min_addr) */
	size_t map_size; /* max_addr - min_addr */
	size_t min_addr; /* min PT_LOAD addr (base_addr relative) */
	size_t max_addr; /* max PT_LOAD addr (base_addr relative) */
	size_t phaddr;
	size_t phnum;
	size_t phent;
	size_t entry;
	size_t real_entry; /* entry point of the interpreter if any, of program otherwise */
};

extern struct spinlock g_proc_list_lock;
extern struct proc_head g_proc_list;
extern struct spinlock g_thread_list_lock;
extern struct thread_head g_thread_list;

void arch_init_trapframe_kern(struct thread *thread);
void arch_init_trapframe_user(struct thread *thread);
void arch_set_stack_pointer(struct trapframe *tf, uintptr_t sp);
uintptr_t arch_get_stack_pointer(struct trapframe *tf);
void arch_set_instruction_pointer(struct trapframe *tf, uintptr_t ip);
uintptr_t arch_get_instruction_pointer(struct trapframe *tf);
void arch_set_frame_pointer(struct trapframe *tf, uintptr_t fp);
uintptr_t arch_get_frame_pointer(struct trapframe *tf);
void arch_set_syscall_retval(struct trapframe *tf, uintptr_t val);
uintptr_t arch_get_syscall_retval(struct trapframe *tf);
#if ARCH_REGISTER_PARAMETERS >= 1
void arch_set_argument0(struct trapframe *tf, uintptr_t val);
uintptr_t arch_get_argument0(struct trapframe *tf);
#endif
#if ARCH_REGISTER_PARAMETERS >= 2
void arch_set_argument1(struct trapframe *tf, uintptr_t val);
uintptr_t arch_get_argument1(struct trapframe *tf);
#endif
#if ARCH_REGISTER_PARAMETERS >= 3
void arch_set_argument2(struct trapframe *tf, uintptr_t val);
uintptr_t arch_get_argument2(struct trapframe *tf);
#endif
#if ARCH_REGISTER_PARAMETERS >= 4
void arch_set_argument3(struct trapframe *tf, uintptr_t val);
uintptr_t arch_get_argument3(struct trapframe *tf);
#endif
#if !ARCH_STACK_RETURN_ADDR
void arch_set_return_address(struct trapframe *tf, uintptr_t val);
uintptr_t arch_get_return_address(struct trapframe *tf);
#endif
int arch_validate_user_trapframe(struct trapframe *tf);
void arch_save_fpu(void *dst);
void arch_load_fpu(const void *src);
void arch_set_tls_addr(uintptr_t addr);
void arch_set_singlestep(struct thread *thread);

struct sess *sess_alloc(pid_t id);
void sess_free(struct sess *sess);
void sess_ref(struct sess *sess);
void sess_lock(struct sess *sess);
void sess_unlock(struct sess *sess);

struct pgrp *pgrp_alloc(pid_t id, struct sess *sess);
void pgrp_free(struct pgrp *pgrp);
void pgrp_ref(struct pgrp *pgrp);
struct pgrp *getpgrp(pid_t id);
void pgrp_lock(struct pgrp *pgrp);
void pgrp_unlock(struct pgrp *pgrp);

void proc_setpgrp(struct proc *proc, struct pgrp *pgrp);

int kproc_create(const char *name, void *entry, const char * const *argv,
                 const char * const *envp, struct thread **thread);

int uproc_clone(struct thread *thread, int flags, struct thread **newthreadp);
int uthread_clone(struct thread *thread, int flags, struct thread **newthreadp);

int uproc_execve(struct thread *thread, struct file *file, const char *path,
                 const char * const *argv, const char * const *envp);

typedef int (*elf_dep_handler_t)(const char *name, void *userdata);
typedef void *(*elf_sym_resolver_t)(const char *name, int type, void *userdata);

int elf_createctx(struct file *file, struct vm_space *vm_space, int flags,
                  elf_dep_handler_t dep_handler,
                  elf_sym_resolver_t sym_resolver,
                  void *userdata, struct elf_info *info);

void proc_free(struct proc *proc);
void thread_free(struct thread *thread);

void thread_ref(struct thread *thread);

struct proc *getproc(pid_t pid);
struct thread *getthread(pid_t tid);
struct proc *proc_getchild(struct proc *proc, pid_t pid);
struct thread *proc_getthread(struct proc *proc, pid_t tid);

int thread_handle_signals(struct thread *thread);
int thread_signal(struct thread *thread, int signum);
int proc_signal(struct proc *proc, int signum);
int thread_exit(struct thread *thread, int code);
int proc_exit(struct proc *proc, int code);
int proc_stop(struct proc *proc);
int proc_cont(struct proc *proc);

void thread_ptrace_stop(struct thread *thread, int signum);
int thread_stop(struct thread *thread);
int thread_cont(struct thread *thread);
int thread_sleep(const struct timespec *duration);

void thread_trace(struct proc *tracer, struct thread *tracee);
void thread_untrace(struct thread *thread);

void thread_fault(struct thread *thread, uintptr_t addr, uint32_t prot);
void thread_illegal_instruction(struct thread *thread);

int proc_getfile(struct proc *proc, int fd, struct file **file);
int proc_allocfd(struct proc *proc, struct file *file, int cloexec);
int proc_freefd(struct proc *proc, int fd);

void thread_print_stack_trace(struct thread *thread, struct uio *uio);

void proc_update_loadavg(void);

void proc_wakeup_vfork(struct proc *proc);
void proc_wakeup_wait(struct proc *proc, struct thread *source);

void proc_test_itimer(struct proc *proc, int itimer);

int setup_idlethread(void);
int setup_initthread(struct thread **thread);

struct timespec proc_time_diff(void);
void proc_add_time_enter(void);
void proc_add_time_leave(void);

int procinfo_register_sysfs(void);
int loadavg_register_sysfs(void);
int cpustat_register_sysfs(void);

uintptr_t syscall(uintptr_t id, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3,
                  uintptr_t arg4, uintptr_t arg5, uintptr_t arg6);

void idle_loop(void) __attribute__((noreturn));
void dead_loop(void) __attribute__((noreturn));

#endif
