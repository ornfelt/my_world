#include <errno.h>
#include <disk.h>
#include <file.h>
#include <vfs.h>
#include <uio.h>
#include <std.h>

static struct spinlock disks_lock = SPINLOCK_INITIALIZER(); /* XXX rwlock */
static TAILQ_HEAD(, disk) disks = TAILQ_HEAD_INITIALIZER(disks);

static ssize_t disk_fread(struct file *file, struct uio *uio);
static ssize_t disk_fwrite(struct file *file, struct uio *uio);
static off_t disk_fseek(struct file *file, off_t off, int whence);

static ssize_t partition_fread(struct file *file, struct uio *uio);
static ssize_t partition_fwrite(struct file *file, struct uio *uio);
static off_t partition_fseek(struct file *file, off_t off, int whence);

static const struct file_op disk_fop =
{
	.read = disk_fread,
	.write = disk_fwrite,
	.seek = disk_fseek,
};

static const struct file_op partition_fop =
{
	.read = partition_fread,
	.write = partition_fwrite,
	.seek = partition_fseek,
};

int disk_new(const char *name, dev_t rdev, off_t size, const struct disk_op *op,
             struct disk **diskp)
{
	struct disk *disk = malloc(sizeof(*disk), M_ZERO);
	if (!disk)
		return -ENOMEM;
	for (size_t i = 0; ; ++i)
	{
		if (i == 4096)
			return -ENOMEM;
		snprintf(disk->name, sizeof(disk->name), "%s%zu", name, i);
		struct disk *tmp;
		spinlock_lock(&disks_lock);
		TAILQ_FOREACH(tmp, &disks, chain)
		{
			if (!strcmp(tmp->name, disk->name))
				break;
		}
		spinlock_unlock(&disks_lock);
		if (!tmp)
			break;
	}
	int ret = bdev_alloc(disk->name, 0, 0, 0600, rdev, &disk_fop, &disk->bdev);
	if (ret)
	{
		free(disk);
		return ret;
	}
	disk->blksz = 512;
	disk->size = size;
	disk->op = op;
	disk->bdev->userdata = disk;
	*diskp = disk;
	spinlock_lock(&disks_lock);
	TAILQ_INSERT_TAIL(&disks, disk, chain);
	spinlock_unlock(&disks_lock);
	return 0;
}

int disk_load(struct disk *disk)
{
	int ret = gpt_parse(disk);
	if (!ret || ret != -ENOENT)
		return ret;
	ret = mbr_parse(disk);
	if (ret)
		return ret;
	return 0;
}

static struct disk *getdisk(struct file *file)
{
	if (file->bdev)
		return file->bdev->userdata;
	if (file->node && S_ISBLK(file->node->attr.mode))
		return file->node->bdev->userdata;
	return NULL;
}

static ssize_t disk_fread(struct file *file, struct uio *uio)
{
	struct disk *disk = getdisk(file);
	if (!disk)
		return -EINVAL;
	return disk_read(disk, uio);
}

static ssize_t disk_fwrite(struct file *file, struct uio *uio)
{
	struct disk *disk = getdisk(file);
	if (!disk)
		return -EINVAL;
	return disk_write(disk, uio);
}

static off_t disk_fseek(struct file *file, off_t off, int whence)
{
	struct disk *disk = getdisk(file);
	if (!disk)
		return -EINVAL;
	switch (whence)
	{
		case SEEK_SET:
			if (off < 0)
				return -EINVAL;
			file->off = off;
			return file->off;
		case SEEK_CUR:
			if (off < 0 && off < -file->off)
				return -EINVAL;
			file->off += off;
			return file->off;
		case SEEK_END:
			if (off < -(ssize_t)disk->size)
				return -EINVAL;
			file->off = disk->size + off;
			return file->off;
		default:
			return -EINVAL;
	}
}

ssize_t disk_read(struct disk *disk, struct uio *uio)
{
	if (!disk->op || !disk->op->read)
		return -EINVAL;
	size_t rd = 0;
	ssize_t ret;
	size_t align = uio->off % disk->blksz;
	if (align)
	{
		uint8_t buf[4096]; /* XXX malloc */
		struct uio pad_uio;
		struct iovec pad_iov;
		assert(disk->blksz <= sizeof(buf), "invalid blksz\n");
		uio_fromkbuf(&pad_uio, &pad_iov, buf, disk->blksz, uio->off - align);
		ret = disk->op->read(disk, &pad_uio);
		if (ret < 0)
			return ret;
		if ((size_t)ret != disk->blksz)
			return rd;
		size_t pad = disk->blksz - align;
		if (pad > uio->count)
			pad = uio->count;
		ret = uio_copyin(uio, &buf[align], pad);
		if (ret < 0)
			return ret;
		uio->off += ret;
		rd += ret;
	}
	if (uio->count >= disk->blksz)
	{
		size_t addend = uio->count % disk->blksz;
		uio->count -= addend;
		ret = disk->op->read(disk, uio);
		if (ret < 0)
			return ret;
		rd += ret;
		uio->count = addend;
	}
	if (uio->count)
	{
		uint8_t buf[4096]; /* XXX malloc */
		struct uio pad_uio;
		struct iovec pad_iov;
		assert(disk->blksz <= sizeof(buf), "invalid blksz\n");
		uio_fromkbuf(&pad_uio, &pad_iov, buf, disk->blksz, uio->off);
		ret = disk->op->read(disk, &pad_uio);
		if (ret < 0)
			return ret;
		if ((size_t)ret != disk->blksz)
			return rd;
		ret = uio_copyin(uio, buf, uio->count);
		if (ret < 0)
			return ret;
		uio->off += ret;
		rd += ret;
	}
	return rd;
}

ssize_t disk_write(struct disk *disk, struct uio *uio)
{
	if (!disk->op || !disk->op->write)
		return -EINVAL;
	size_t wr = 0;
	ssize_t ret;
	size_t align = uio->off % disk->blksz;
	if (align)
	{
		uint8_t buf[4096]; /* XXX malloc */
		struct uio pad_uio;
		struct iovec pad_iov;
		assert(disk->blksz <= sizeof(buf), "invalid blksz\n");
		uio_fromkbuf(&pad_uio, &pad_iov, buf, disk->blksz, uio->off - align);
		ret = disk->op->read(disk, &pad_uio);
		if (ret < 0)
			return ret;
		if ((size_t)ret != disk->blksz)
			return wr;
		size_t pad = disk->blksz - align;
		if (pad > uio->count)
			pad = uio->count;
		ret = uio_copyout(&buf[align], uio, pad);
		if (ret < 0)
			return ret;
		uio->off += ret;
		wr += ret;
		uio_fromkbuf(&pad_uio, &pad_iov, buf, disk->blksz, uio->off - align);
		ret = disk->op->write(disk, &pad_uio);
		if (ret < 0)
			return ret;
		if ((size_t)ret != disk->blksz)
			return wr;
	}
	if (uio->count >= disk->blksz)
	{
		size_t addend = uio->count % disk->blksz;
		uio->count -= addend;
		ret = disk->op->write(disk, uio);
		if (ret < 0)
			return ret;
		wr += ret;
		uio->count = addend;
	}
	if (uio->count)
	{
		uint8_t buf[4096]; /* XXX malloc */
		struct uio pad_uio;
		struct iovec pad_iov;
		assert(disk->blksz <= sizeof(buf), "invalid blksz\n");
		uio_fromkbuf(&pad_uio, &pad_iov, buf, disk->blksz, uio->off);
		ret = disk->op->read(disk, &pad_uio);
		if (ret < 0)
			return ret;
		if ((size_t)ret != disk->blksz)
			return wr;
		ret = uio_copyout(buf, uio, uio->count);
		if (ret < 0)
			return ret;
		uio->off += ret;
		wr += ret;
		uio_fromkbuf(&pad_uio, &pad_iov, buf, disk->blksz, uio->off);
		ret = disk->op->write(disk, &pad_uio);
		if (ret < 0)
			return ret;
		if ((size_t)ret != disk->blksz)
			return wr;
	}
	return wr;
}

int partition_new(struct disk *disk, size_t id, off_t offset, off_t size,
                  struct partition **partitionp)
{
	struct partition *partition = malloc(sizeof(*partition), M_ZERO);
	if (!partition)
		return -ENOMEM;
	if (snprintf(partition->name, sizeof(partition->name), "%sp%u", disk->name,
	             (unsigned)id) >= (int)sizeof(partition->name))
	{
		free(partition);
		return -EINVAL;
	}
	partition->id = id;
	partition->disk = disk;
	partition->offset = offset;
	partition->size = size;
	int ret = bdev_alloc(partition->name, 0, 0, 0600, disk->bdev->rdev + 1,
	                     &partition_fop,
	                     &partition->bdev);
	if (ret)
	{
		free(partition);
		return ret;
	}
	partition->bdev->userdata = partition;
	*partitionp = partition;
	return 0;
}

static struct partition *getpartition(struct file *file)
{
	if (file->bdev)
		return file->bdev->userdata;
	if (file->node && S_ISBLK(file->node->attr.mode))
		return file->node->bdev->userdata;
	return NULL;
}

static ssize_t partition_fread(struct file *file, struct uio *uio)
{
	struct partition *partition = getpartition(file);
	if (!partition)
		return -EINVAL;
	return partition_read(partition, uio);
}

static ssize_t partition_fwrite(struct file *file, struct uio *uio)
{
	struct partition *partition = getpartition(file);
	if (!partition)
		return -EINVAL;
	return partition_write(partition, uio);
}

static off_t partition_fseek(struct file *file, off_t off, int whence)
{
	struct partition *partition = getpartition(file);
	if (!partition)
		return -EINVAL;
	switch (whence)
	{
		case SEEK_SET:
			if (off < 0)
				return -EINVAL;
			file->off = off;
			return file->off;
		case SEEK_CUR:
			if (off < 0 && off < -file->off)
				return -EINVAL;
			file->off += off;
			return file->off;
		case SEEK_END:
			if (off < -(ssize_t)partition->size)
				return -EINVAL;
			file->off = partition->size + off;
			return file->off;
		default:
			return -EINVAL;
	}
}

ssize_t partition_read(struct partition *partition, struct uio *uio)
{
	if (uio->off < 0)
		return -EINVAL;
	off_t foff;
	if (__builtin_add_overflow(partition->offset, uio->off, &foff))
		return -EINVAL;
	if (foff >= partition->size)
		return 0;
	off_t count = uio->count;
	off_t last;
	if (__builtin_add_overflow(foff, count, &last))
		return -EINVAL;
	if (last >= partition->size)
		count = partition->size - foff;
	off_t tmpoff = uio->off;
	size_t tmpcount = uio->count;
	uio->off = foff;
	uio->count = count;
	ssize_t ret = disk_read(partition->disk, uio);
	uio->off = tmpoff + (uio->off - foff);
	uio->count = tmpcount - (count - uio->count);
	return ret;
}

ssize_t partition_write(struct partition *partition, struct uio *uio)
{
	if (uio->off < 0)
		return -EINVAL;
	off_t foff;
	if (__builtin_add_overflow(partition->offset, uio->off, &foff))
		return -EINVAL;
	if (foff >= partition->size)
		return 0;
	off_t count = uio->count;
	off_t last;
	if (__builtin_add_overflow(foff, count, &last))
		return -EINVAL;
	if (last >= partition->size)
		count = partition->size - foff;
	off_t tmpoff = uio->off;
	size_t tmpcount = uio->count;
	uio->off = foff;
	uio->count = count;
	ssize_t ret = disk_write(partition->disk, uio);
	uio->off = tmpoff + (uio->off - foff);
	uio->count = tmpcount - (count - uio->count);
	return ret;
}
