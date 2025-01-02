#ifndef UNISTD_H
#define UNISTD_H

#include <sys/param.h>
#include <sys/types.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

#define F_OK 0
#define X_OK (1 << 0)
#define W_OK (1 << 1)
#define R_OK (1 << 2)

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define _PC_LINK_MAX         0
#define _PC_MAX_CANON        1
#define _PC_MAX_INPUT        2
#define _PC_NAME_MAX         3
#define _PC_PATH_MAX         4
#define _PC_PIPE_BUF         5
#define _PC_CHOWN_RESTRICTED 6
#define _PC_NO_TRUNC         7
#define _PC_VDISABLE         8

#define _SC_ARG_MAX        1
#define _SC_CHILD_MAX      2
#define _SC_HOST_NAME_MAX  3
#define _SC_LOGIN_NAME_MAX 4
#define _SC_NGROUPS_MAX    5
#define _SC_CLK_TCK        6
#define _SC_OPEN_MAX       7
#define _SC_PAGESIZE       8
#define _SC_PAGE_SIZE      9
#define _SC_RE_DUP_MAX     10
#define _SC_STREAM_MAX     11
#define _SC_SYMLOOP_MAX    12
#define _SC_TTY_NAME_MAX   13
#define _SC_TZNAME_MAX     14
#define _SC_VERSION        15

struct stat;
struct sys_dirent;

typedef unsigned useconds_t;

ssize_t write(int fd, const void *buffer, size_t count);
ssize_t read(int fd, void *buffer, size_t count);
off_t lseek(int fd, off_t offset, int whence);
void _exit(int status) __attribute__((noreturn));
void exit_group(int status) __attribute__((noreturn));
int close(int fd);
int dup(int oldfd);
int dup2(int oldfd, int newfd);
int dup3(int oldfd, int newfd, int flags);
int pipe(int pipefd[2]);
int pipe2(int pipefd[2], int flags);

int link(const char *oldpath, const char *newpath);
int linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath,
           int flags);
int symlink(const char *target, const char *linkpath);
int symlinkat(const char *target, int newdirfd, const char *linkpath);
ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);
ssize_t readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz);
int unlink(const char *pathname);
int unlinkat(int dirfd, const char *pathname, int flags);
int rmdir(const char *pathname);
int truncate(const char *pathname, off_t length);
int ftruncate(int fd, off_t length);
int ftruncateat(int dirfd, const char *pathname, off_t length, int flags);
int fsync(int fd);
int fdatasync(int fd);

int getdents(int fd, struct sys_dirent *dirp, unsigned long count);

pid_t getpid(void);
pid_t getppid(void);
pid_t fork(void);
pid_t vfork(void);
pid_t gettid(void);

uid_t getuid(void);
uid_t geteuid(void);
gid_t getgid(void);
gid_t getegid(void);
int setreuid(uid_t ruid, uid_t euid);
int setregid(gid_t rgid, gid_t egid);
int setuid(uid_t uid);
int setgid(gid_t gid);
int seteuid(uid_t euid);
int setegid(gid_t egid);

int setpgid(pid_t pid, pid_t pgid);
pid_t getpgid(pid_t pid);
pid_t getpgrp(void);
pid_t setsid(void);
pid_t getsid(pid_t pid);

int getgroups(int size, gid_t list[]);

int settls(void *addr);
void *gettls(void);

int fexecve(int fd, char * const argv[], char * const envp[]);
int execveat(int dirfd, const char *pathname, char * const argv[],
             char * const envp[], int flags);
int execve(const char *pathname, char * const argv[], char * const envp[]);
int execl(const char *pathname, const char *arg, ...);
int execlp(const char *file, const char *arg, ...);
int execle(const char *pathname, const char *arg, ...);
int execv(const char *pathname, char * const argv[]);
int execvp(const char *file, char * const argv[]);

int getopt(int argc, char * const argv[], const char *optstring);

unsigned int sleep(unsigned int seconds);
int usleep(useconds_t usec);

int chdir(const char *path);
int fchdir(int fd);
char *getcwd(char *buf, size_t size);
int chroot(const char *path);

int chown(const char *pathname, uid_t uid, gid_t gid);
int fchown(int fd, uid_t uid, gid_t gid);
int lchown(const char *pathname, uid_t uid, gid_t gid);
int fchownat(int dirfd, const char *pathname, uid_t uid, gid_t gid, int flags);

int access(const char *pathname, int mode);
int faccessat(int dirfd, const char *pathname, int mode, int flags);

int getpagesize(void);

char *getpass(const char *prompt);

long fpathconf(int fd, int name);
long pathconf(const char *path, int name);

int isatty(int fd);

int daemon(int nochdir, int noclose);

unsigned alarm(unsigned seconds);

char *getlogin(void);

long sysconf(int name);

pid_t tcgetpgrp(int fd);
int tcsetpgrp(int fd, pid_t pgrp);

int pause(void);

int nice(int inc);

ssize_t syscall(size_t id, ...);

extern char *optarg;
extern int optind;
extern int opterr;
extern int optopt;

#ifdef __cplusplus
}
#endif

#endif
