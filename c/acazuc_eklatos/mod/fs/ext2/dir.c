#include "ext2.h"

#include <vfs.h>
#include <uio.h>

int dir_lookup(struct node *node, const char *name, size_t name_len,
               struct node **childp)
{
	struct ext2_node *dir = (struct ext2_node*)node;
	VFS_HANDLE_DOT_DOTDOT_LOOKUP(node, dir->parent, name, name_len, childp);
	struct ext2_fs *fs = node->sb->private;
	uint8_t blk[EXT2_MAXBLKSZ_U8];
	uint32_t blkid = 0;
	int ret = read_node_block(fs, dir, blkid, blk);
	if (ret)
		return ret;
	uint32_t blkoff = 0;
	for (size_t i = 0; i < dir->inode.size;)
	{
		if (blkoff >= fs->blksz)
		{
			blkid += blkoff / fs->blksz;
			blkoff %= fs->blksz;
			ret = read_node_block(fs, dir, blkid, blk);
			if (ret)
				return ret;
		}
		struct ext2_dirent *dirent = (struct ext2_dirent*)&blk[blkoff];
		if (dirent->name_len == name_len
		 && !memcmp(dirent->name, name, name_len))
		{
			struct ext2_node *child;
			ret = get_node(fs, dirent->inode, &child);
			if (ret)
				return -ret;
			child->parent = node;
			*childp = &child->node;
			return 0;
		}
		i += dirent->rec_len;
		blkoff += dirent->rec_len;
	}
	return -ENOENT;
}

static int dt_from_ft(int type)
{
	switch (type)
	{
		case EXT2_FT_REG_FILE:
			return DT_REG;
		case EXT2_FT_DIR:
			return DT_DIR;
		case EXT2_FT_CHRDEV:
			return DT_CHR;
		case EXT2_FT_BLKDEV:
			return DT_BLK;
		case EXT2_FT_FIFO:
			return DT_FIFO;
		case EXT2_FT_SOCK:
			return DT_SOCK;
		case EXT2_FT_SYMLINK:
			return DT_LNK;
		default:
			return DT_UNKNOWN;
	}
}

static int ft_from_dt(int type)
{
	switch (type)
	{
		case DT_REG:
			return EXT2_FT_REG_FILE;
		case DT_DIR:
			return EXT2_FT_DIR;
		case DT_CHR:
			return EXT2_FT_CHRDEV;
		case DT_BLK:
			return EXT2_FT_BLKDEV;
		case DT_FIFO:
			return EXT2_FT_FIFO;
		case DT_SOCK:
			return EXT2_FT_SOCK;
		case DT_LNK:
			return EXT2_FT_SYMLINK;
		default:
			return -1;
	}
}

int dir_readdir(struct node *node, struct fs_readdir_ctx *ctx)
{
	int written = 0;
	struct ext2_node *dir = (struct ext2_node*)node;
	VFS_HANDLE_DOT_DOTDOT_READDIR(node, dir->parent, ctx, written);
	struct ext2_fs *fs = node->sb->private;
	uint8_t blk[EXT2_MAXBLKSZ_U8];
	int res;
	size_t n = 2;
	uint32_t blkid = 0;
	int ret = read_node_block(fs, dir, blkid, blk);
	if (ret)
		return ret;
	uint32_t blkoff = 0;
	for (size_t i = 0; i < dir->inode.size;)
	{
		if (blkoff >= fs->blksz)
		{
			blkid += blkoff / fs->blksz;
			blkoff %= fs->blksz;
			ret = read_node_block(fs, dir, blkid, blk);
			if (ret)
				return ret;
		}
		struct ext2_dirent *dirent = (struct ext2_dirent*)&blk[blkoff];
		if ((dirent->name_len == 1 && dirent->name[0] == '.')
		 || (dirent->name_len == 2 && dirent->name[0] == '.'
		                           && dirent->name[1] == '.')
		 || (dirent->file_type == EXT2_FT_UNKNOWN))
		{
			i += dirent->rec_len;
			blkoff += dirent->rec_len;
			continue;
		}
		if (n == (size_t)ctx->off)
		{
			res = ctx->fn(ctx, dirent->name, dirent->name_len, n,
			              dirent->inode,
			              dt_from_ft(dirent->file_type));
			if (res)
				return written;
			written++;
			ctx->off++;
		}
		i += dirent->rec_len;
		blkoff += dirent->rec_len;
		n++;
	}
	return written;
}

int dir_add_dirent(struct ext2_node *node, const char *name,
                   size_t name_len, ino_t ino, struct ext2_inode *inode)
{
	struct
	{
		struct ext2_dirent dirent;
		char name[256];
	} dirent;
	dirent.dirent.inode = ino;
	dirent.dirent.rec_len = sizeof(dirent.dirent) + name_len;
	dirent.dirent.name_len = name_len;
	dirent.dirent.file_type = ft_from_dt(inode->mode >> 12);
	memcpy(dirent.name, name, name_len);
	/* XXX find hole of sizeof(struct ext2_dirent) + name_len bytes */
	struct uio uio;
	struct iovec iov;
	uio_fromkbuf(&uio, &iov, &dirent, sizeof(dirent),
	            ((struct ext2_node*)node)->inode.size);
	return node_write((struct ext2_node*)node, &uio);
}

int dir_mknode(struct node *node, const char *name, size_t name_len,
               fs_attr_mask_t mask, const struct fs_attr *attr,
               dev_t rdev)
{
	struct ext2_fs *fs = node->sb->private;
	struct node *child;
	ino_t ino = 0;
	if (!(mask & FS_ATTR_MODE))
		return -EINVAL;
	if (name_len > 255)
		return -EINVAL;
	int ret = node_lookup(node, name, name_len, &child);
	if (!ret)
	{
		node_free(child);
		return -EEXIST;
	}
	if (ret != -ENOENT)
		return ret;
	ret = alloc_inode(fs, &ino);
	if (ret)
		return ret;
	switch (attr->mode & S_IFMT)
	{
		case 0:
		case S_IFREG:
		case S_IFIFO:
		case S_IFCHR:
		case S_IFBLK:
		case S_IFSOCK:
			ret = fs_mknode(fs, ino, mask, attr, rdev,
			                (struct ext2_node**)&child);
			if (ret)
				goto err;
			ret = write_inode(fs, ino,
			                  &((struct ext2_node*)child)->inode);
			break;
		case S_IFDIR:
			ret = fs_mknode(fs, ino, mask, attr, 0,
			                (struct ext2_node**)&child);
			if (ret)
				goto err;
			((struct ext2_node*)child)->parent = node;
			ret = write_inode(fs, ino,
			                  &((struct ext2_node*)child)->inode);
			if (ret)
				goto err;
			ret = dir_add_dirent((struct ext2_node*)child, ".", 1,
			                     ino, &((struct ext2_node*)child)->inode);
			if (ret)
				return ret;
			ret = dir_add_dirent((struct ext2_node*)child, "..", 2,
			                     node->ino, &((struct ext2_node*)node)->inode);
			break;
		default:
			ret = -EINVAL;
			break;
	}
	if (ret)
		goto err;
	ret = dir_add_dirent((struct ext2_node*)node, name, name_len,
	                     ino, &((struct ext2_node*)child)->inode);
	if (ret)
		goto err;
	return 0;

err:
	if (ino)
		free_inode(fs, ino); /* XXX best effort */
	if (child)
		node_free(child);
	return ret;
}
