#ifndef FILE_H
#define FILE_H

#include <refcount.h>
#include <types.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define O_RDONLY    0
#define O_WRONLY    1
#define O_RDWR      2
#define O_APPEND    (1 << 2)
#define O_ASYNC     (1 << 3)
#define O_CREAT     (1 << 4)
#define O_TRUNC     (1 << 5)
#define O_NOFOLLOW  (1 << 6)
#define O_DIRECTORY (1 << 7)
#define O_EXCL      (1 << 8)
#define O_NONBLOCK  (1 << 9)
#define O_CLOEXEC   (1 << 10)
#define O_NOCTTY    (1 << 11)

#define AT_FDCWD -100

#define AT_SYMLINK_NOFOLLOW (1 << 0)
#define AT_REMOVEDIR        (1 << 1)
#define AT_SYMLINK_FOLLOW   (1 << 2)
#define AT_EMPTY_PATH       (1 << 3)
#define AT_EACCESS          (1 << 4)

#define F_OK 0
#define X_OK (1 << 0)
#define W_OK (1 << 1)
#define R_OK (1 << 2)

#define F_DUPFD  0
#define F_GETFD  1
#define F_SETFD  2
#define F_GETFL  3
#define F_SETFL  4
#define F_SETLK  5
#define F_SETLKW 6
#define F_GETLK  7

#define F_RDLCK 0
#define F_WRLCK 1
#define F_UNLCK 2

struct poll_entry;
struct vm_space;
struct vm_zone;
struct file_op;
struct node;
struct sock;
struct uio;

struct flock
{
	int16_t l_type;
	int16_t l_whence;
	off_t l_start;
	off_t l_len;
	pid_t l_pid;
};

struct file
{
	const struct file_op *op;
	struct node *node; /* XXX union of node, sock, bdev, cdev, pipe */
	struct sock *sock;
	struct bdev *bdev;
	struct cdev *cdev;
	off_t off;
	refcount_t refcount;
	void *userdata;
	int flags;
};

struct file_op
{
	int (*release)(struct file *file);
	int (*open)(struct file *file, struct node *node);
	ssize_t (*write)(struct file *file, struct uio *uio);
	ssize_t (*read)(struct file *file, struct uio *uio);
	int (*ioctl)(struct file *file, unsigned long request, uintptr_t data);
	int (*mmap)(struct file *file, struct vm_zone *zone);
	off_t (*seek)(struct file *file, off_t off, int whence);
	int (*poll)(struct file *file, struct poll_entry *entry);
};

int file_fromnode(struct node *node, int flags, struct file **file);
int file_fromsock(struct sock *sock, int flags, struct file **file);
int file_frombdev(struct bdev *bdev, int flags, struct file **file);
int file_fromcdev(struct cdev *cdev, int flags, struct file **file);
void file_ref(struct file *file);
void file_free(struct file *file);

int file_release(struct file *file);
int file_open(struct file *file, struct node *node);
ssize_t file_write(struct file *file, struct uio *uio);
ssize_t file_read(struct file *file, struct uio *uio);
ssize_t file_readseq(struct file *file, void *data, size_t count, off_t off);
int file_ioctl(struct file *file, unsigned long request, uintptr_t data);
int file_mmap(struct file *file, struct vm_zone *zone);
int file_seek(struct file *file, off_t off, int whence);
int file_poll(struct file *file, struct poll_entry *entry);

#endif
