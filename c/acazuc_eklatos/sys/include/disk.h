#ifndef DISK_H
#define DISK_H

#include <queue.h>
#include <types.h>

struct partition;
struct disk;
struct bdev;
struct file;
struct uio;

struct disk_op
{
	ssize_t (*read)(struct disk *disk, struct uio *uio);
	ssize_t (*write)(struct disk *disk, struct uio *uio);
};

struct disk
{
	const struct disk_op *op;
	struct bdev *bdev;
	struct partition **partitions;
	size_t partitions_nb;
	char name[64];
	off_t size;
	void *userdata;
	size_t blksz;
	TAILQ_ENTRY(disk) chain;
};

struct partition
{
	struct disk *disk;
	struct bdev *bdev;
	char name[64];
	size_t id;
	off_t offset;
	off_t size;
};

int disk_new(const char *name, dev_t rdev, off_t size, const struct disk_op *op,
             struct disk **diskp);
int disk_load(struct disk *disk);
ssize_t disk_read(struct disk *disk, struct uio *uio);
ssize_t disk_write(struct disk *disk, struct uio *uio);

int partition_new(struct disk *disk, size_t id, off_t offset, off_t size,
                  struct partition **partitionp);
ssize_t partition_read(struct partition *partition, struct uio *uio);
ssize_t partition_write(struct partition *partition, struct uio *uio);

int mbr_parse(struct disk *disk);
int gpt_parse(struct disk *disk);

#endif
