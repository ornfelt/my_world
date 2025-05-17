#include "ext2.h"

#include <uio.h>

int read_inode(struct ext2_fs *fs, uint32_t ino, struct ext2_inode *inode)
{
	uint32_t group = (ino - 1) / fs->ext2sb.inodes_per_group;
	struct ext2_group_desc group_desc;
	int ret = read_group_desc(fs, group, &group_desc);
	if (ret)
		return ret;
#if 0
	print_group_desc(&group_desc);
#endif
	return read_disk_data(fs, inode, sizeof(*inode),
	                      group_desc.inode_table * fs->blksz
	                    + sizeof(*inode)
	                    * ((ino - 1) % fs->ext2sb.inodes_per_group));
}

int write_inode(struct ext2_fs *fs, uint32_t ino, struct ext2_inode *inode)
{
	uint32_t group = (ino - 1) / fs->ext2sb.inodes_per_group;
	struct ext2_group_desc group_desc;
	int ret = read_group_desc(fs, group, &group_desc);
	if (ret)
		return ret;
#if 0
	print_group_desc(&group_desc);
#endif
	return write_disk_data(fs, inode, sizeof(*inode),
	                       group_desc.inode_table * fs->blksz
	                     + sizeof(*inode)
	                     * ((ino - 1) % fs->ext2sb.inodes_per_group));
}

static int create_ind_inode_block(struct ext2_fs *fs, struct ext2_node *node,
                                  uint32_t *blkid)
{
	int ret = alloc_block_zero(fs, blkid);
	if (ret)
	{
		*blkid = 0;
		return ret;
	}
	ret = write_inode(fs, node->node.ino, &node->inode);
	if (ret)
	{
		*blkid = 0;
		return ret;
	}
	return 0;
}

static int create_ind_ind_block(struct ext2_fs *fs, uint32_t *parent,
                                uint32_t parentid, uint32_t *blkid)
{
	int ret = alloc_block_zero(fs, blkid);
	if (ret)
		return ret;
	ret = write_block(fs, parent, parentid);
	if (ret)
		return ret;
	return 0;
}

static int get_ind_block(struct ext2_fs *fs, struct ext2_node *node,
                         uint32_t id, int create, uint32_t *blkid,
                         uint32_t *offset)
{
	int ret;
	if (id < 12)
		return -EINVAL;
	uint32_t base = 12;
	uint32_t diff = fs->blk_per_blk;
	if (id < base + diff)
	{
		if (!node->inode.block[12])
		{
			if (!create)
			{
				*blkid = 0;
				return 0;
			}
			ret = create_ind_inode_block(fs, node,
			                             &node->inode.block[12]);
			if (ret)
				return ret;
		}
		*blkid = node->inode.block[12];
		*offset = id - base;
		return 0;
	}
	base += diff;
	diff *= fs->blk_per_blk;
	if (id < base + diff)
	{
		uint32_t blk[EXT2_MAXBLKSZ_U32];
		if (!node->inode.block[13])
		{
			if (!create)
			{
				*blkid = 0;
				return 0;
			}
			ret = create_ind_inode_block(fs, node,
			                             &node->inode.block[13]);
			if (ret)
				return ret;
		}
		ret = read_block(fs, blk, node->inode.block[13]);
		if (ret)
			return ret;
		uint32_t idx = base / fs->blk_per_blk;
		*offset = base % fs->blk_per_blk;
		*blkid = blk[idx];
		if (!*blkid)
		{
			if (!create)
				return 0;
			ret = create_ind_ind_block(fs, blk,
			                           node->inode.block[13],
			                           blkid);
			if (ret)
				return ret;
		}
		return 0;
	}
	base += diff;
	diff += fs->blk_per_blk;
	if (id < base + diff)
	{
		uint32_t blk[EXT2_MAXBLKSZ_U32];
		if (!node->inode.block[14])
		{
			if (!create)
			{
				*blkid = 0;
				return 0;
			}
			ret = create_ind_inode_block(fs, node,
			                             &node->inode.block[14]);
			if (ret)
				return ret;
		}
		ret = read_block(fs, blk, node->inode.block[14]);
		if (ret)
			return ret;
		uint32_t tmp = fs->blk_per_blk * fs->blk_per_blk;
		if (!blk[base / tmp])
		{
			if (!create)
			{
				*blkid = 0;
				return 0;
			}
			ret = create_ind_ind_block(fs, blk,
			                           node->inode.block[14],
			                           &blk[base / tmp]);
			if (ret)
				return ret;
		}
		uint32_t indid = blk[base / tmp];
		ret = read_block(fs, blk, indid);
		if (ret)
			return ret;
		*blkid = blk[(base % tmp) / fs->blk_per_blk];
		*offset = base % fs->blk_per_blk;
		if (!*blkid)
		{
			if (!create)
				return 0;
			ret = create_ind_ind_block(fs, blk,
			                           indid, blkid);
			if (ret)
				return ret;
		}
		return 0;
	}
	return -EINVAL;
}

static int get_node_block_id(struct ext2_fs *fs, struct ext2_node *node,
                             uint32_t id, uint32_t *blkid)
{
	if (id < 12)
	{
		*blkid = node->inode.block[id];
		return 0;
	}
	uint32_t blk[EXT2_MAXBLKSZ_U32];
	uint32_t indblk;
	uint32_t indoff;
	int ret = get_ind_block(fs, node, id, 0, &indblk, &indoff);
	if (ret)
		return ret;
	if (!indblk)
	{
		*blkid = 0;
		return 0;
	}
	ret = read_block(fs, blk, indblk);
	if (ret)
		return ret;
	*blkid = blk[indoff];
	return 0;
}

static int set_node_block_id(struct ext2_fs *fs, struct ext2_node *node,
                             uint32_t id, uint32_t blkid)
{
	if (id < 12)
	{
		uint32_t tmp = node->inode.block[id];
		node->inode.block[id] = blkid;
		int ret = write_inode(fs, node->node.ino, &node->inode);
		if (ret)
		{
			node->inode.block[id] = tmp;
			return ret;
		}
		return 0;
	}
	uint32_t blk[EXT2_MAXBLKSZ_U32];
	uint32_t indblk;
	uint32_t indoff;
	int ret = get_ind_block(fs, node, id, 1, &indblk, &indoff);
	if (ret)
		return ret;
	ret = read_block(fs, blk, indblk);
	if (ret)
		return ret;
	blk[indoff] = blkid;
	return write_block(fs, blk, indblk);
}

int update_node_inode(struct ext2_node *node)
{
	/* XXX
	 * we shouldn't update fs_node->attr and forward it to inode:
	 * we should update inode, and if it doesn't fail, we should update node->attr accordingly
	 */
	struct ext2_fs *fs = node->node.sb->private;
	node->inode.atime = node->node.attr.atime.tv_sec;
	node->inode.mtime = node->node.attr.mtime.tv_sec;
	node->inode.ctime = node->node.attr.ctime.tv_sec;
	node->inode.uid = node->node.attr.uid;
	node->inode.gid = node->node.attr.gid;
	node->inode.mode = node->node.attr.mode;
	node->inode.size = node->node.attr.size;
	int ret = write_inode(fs, node->node.ino, &node->inode);
	if (ret)
		return ret;
	return 0;
}

int node_truncate(struct ext2_node *node, off_t size)
{
	off_t tmp = node->node.attr.size;
	if (size == tmp)
		return 0;
	node->node.attr.size = size;
	int ret = update_node_inode(node);
	if (ret)
	{
		node->node.attr.size = tmp;
		return ret;
	}
	if (size < tmp)
	{
		/* XXX release blocks */
	}
	else
	{
		/* XXX allocate blocks */
	}
	return 0;
}

int read_node_block(struct ext2_fs *fs, struct ext2_node *node,
                    uint32_t id, void *data)
{
	uint32_t blkid;
	int ret = get_node_block_id(fs, node, id, &blkid);
	if (ret)
		return ret;
	if (!blkid) /* sparse file */
	{
		memset(data, 0, fs->blksz);
		return 0;
	}
	return read_block(fs, data, blkid);
}

ssize_t node_read(struct ext2_node *node, struct uio *uio)
{
	struct ext2_fs *fs = node->node.sb->private;
	ssize_t ret;
	size_t count;
	if (uio->off + uio->count > node->inode.size)
		count = node->inode.size - uio->off;
	else
		count = uio->count;
	if (!count)
		return 0;
	size_t org = count;
	off_t align = uio->off % fs->blksz;
	if (align)
	{
		uint8_t tmp[EXT2_MAXBLKSZ_U8];
		ret = read_node_block(fs, node, uio->off / fs->blksz, tmp);
		if (ret)
			return ret;
		size_t diff = fs->blksz - align;
		if (diff >= count)
			return uio_copyin(uio, &tmp[align], count);
		ret = uio_copyin(uio, &tmp[align], diff);
		if (ret < 0)
			return ret;
		count -= ret;
	}
	while (count >= fs->blksz)
	{
		uint8_t tmp[EXT2_MAXBLKSZ_U8];
		ret = read_node_block(fs, node, uio->off / fs->blksz, tmp);
		if (ret)
			return ret;
		ret = uio_copyin(uio, tmp, fs->blksz);
		if (ret < 0)
			return ret;
		count -= ret;
	}
	if (count)
	{
		uint8_t tmp[EXT2_MAXBLKSZ_U8];
		ret = read_node_block(fs, node, uio->off / fs->blksz, tmp);
		if (ret)
			return ret;
		ret = uio_copyin(uio, tmp, count);
		if (ret < 0)
			return ret;
	}
	return org;
}

static int write_node_block(struct ext2_fs *fs, struct ext2_node *node,
                            uint32_t id, const void *data)
{
	uint32_t blkid;
	int ret = get_node_block_id(fs, node, id, &blkid);
	if (ret)
		return ret;
	if (!blkid)
	{
		ret = alloc_block(fs, &blkid);
		if (ret)
			return ret;
		ret = set_node_block_id(fs, node, id, blkid);
		if (ret)
		{
			free_block(fs, blkid); /* best effort... */
			return ret;
		}
	}
	return write_block(fs, data, blkid);
}

ssize_t node_write(struct ext2_node *node, struct uio *uio)
{
	struct ext2_fs *fs = node->node.sb->private;
	ssize_t ret;
	if (!uio->count)
		return 0;
	size_t org = uio->count;
	off_t align = uio->off % fs->blksz;
	int dirty_inode = 0;
	if (align)
	{
		uint8_t tmp[EXT2_MAXBLKSZ_U8];
		ret = read_node_block(fs, node, uio->off / fs->blksz, tmp);
		if (ret)
			return ret;
		size_t diff = fs->blksz - align;
		if (diff >= uio->count)
			diff = uio->count;
		ret = uio_copyout(&tmp[align], uio, diff);
		if (ret < 0)
			return ret;
		ret = write_node_block(fs, node, uio->off / fs->blksz, tmp);
		if (ret)
			return ret;
		if (uio->off > node->node.attr.size)
		{
			node->node.attr.size = uio->off;
			dirty_inode = 1;
		}
	}
	while (uio->count >= fs->blksz)
	{
		uint8_t tmp[EXT2_MAXBLKSZ_U8];
		ret = uio_copyout(tmp, uio, fs->blksz);
		if (ret < 0)
			return ret;
		ret = write_node_block(fs, node, uio->off / fs->blksz, tmp);
		if (ret)
			return ret;
		if (uio->off > node->node.attr.size)
		{
			node->node.attr.size = uio->off;
			dirty_inode = 1;
		}
	}
	if (uio->count)
	{
		uint8_t tmp[EXT2_MAXBLKSZ_U8];
		ret = read_node_block(fs, node, uio->off / fs->blksz, tmp);
		if (ret)
			return ret;
		ret = uio_copyout(tmp, uio, uio->count);
		if (ret < 0)
			return ret;
		ret = write_node_block(fs, node, uio->off / fs->blksz, tmp);
		if (ret)
			return ret;
		if (uio->off > node->node.attr.size)
		{
			node->node.attr.size = uio->off;
			dirty_inode = 1;
		}
	}
	if (dirty_inode)
	{
		ret = update_node_inode(node);
		if (ret)
			return ret;
	}
	return org;
}

int alloc_inode(struct ext2_fs *fs, ino_t *ino)
{
	if (!fs->ext2sb.free_inodes_count)
		return -ENOMEM;
	for (size_t i = 0; i < fs->groups_count; ++i)
	{
		int ret = group_alloc_inode(fs, i, ino);
		if (ret)
			return ret;
		if (!*ino)
			continue;
		fs->ext2sb.free_inodes_count--;
		ret = write_sb(fs);
		if (ret)
			panic("failed to write ext2 sb\n"); /* XXX */
		return 0;
	}
	return -ENOMEM;
}

int free_inode(struct ext2_fs *fs, ino_t ino)
{
	uint32_t grpid = ino / fs->ext2sb.inodes_per_group;
	struct ext2_group_desc group_desc;
	int ret = read_group_desc(fs, grpid, &group_desc);
	if (ret)
		return ret;
	if (group_desc.free_inodes_count >= fs->ext2sb.inodes_per_group)
		return -EINVAL; /* XXX assert */
	uint8_t blk[EXT2_MAXBLKSZ_U8];
	ret = read_block(fs, blk, group_desc.inode_bitmap);
	if (ret)
		return ret;
	uint32_t idx = ino % fs->ext2sb.inodes_per_group;
	if (!(blk[idx / 8] & (1 << (idx % 8))))
		return -EINVAL; /* XXX assert */
	blk[idx / 8] &= ~(1 << (idx % 8));
	ret = write_block(fs, blk, group_desc.inode_bitmap);
	if (ret)
		return ret;
	group_desc.free_inodes_count++;
	ret = write_group_desc(fs, grpid, &group_desc);
	if (ret)
		panic("failed to write group desc\n"); /* XXX */
	return 0;
}

