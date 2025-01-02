#include <ramfile.h>
#include <std.h>
#include <mem.h>

#define PTR_PER_BLK ((uint64_t)PAGE_SIZE / sizeof(void*))

#define BLK_IND1_IDX (11)
#define BLK_IND1_OFF (11)
#define BLK_IND1_LEN (PTR_PER_BLK)

#define BLK_IND2_IDX (12)
#define BLK_IND2_OFF (BLK_IND1_OFF + BLK_IND1_LEN)
#define BLK_IND2_LEN (BLK_IND1_LEN * PTR_PER_BLK)

#define BLK_IND3_IDX (13)
#define BLK_IND3_OFF (BLK_IND2_OFF + BLK_IND2_LEN)
#define BLK_IND3_LEN (BLK_IND2_LEN * PTR_PER_BLK)

#define BLK_IND4_IDX (14)
#define BLK_IND4_OFF (BLK_IND3_OFF + BLK_IND3_LEN)
#define BLK_IND4_LEN (BLK_IND3_LEN * PTR_PER_BLK)

#define BLK_IND5_IDX (15)
#define BLK_IND5_OFF (BLK_IND4_OFF + BLK_IND4_LEN)
#define BLK_IND5_LEN (BLK_IND4_LEN * PTR_PER_BLK)

#define BLK_IND6_OFF (BLK_IND5_OFF + BLK_IND5_LEN)

union ramfile_blk
{
	struct page *pages[PTR_PER_BLK];
	union ramfile_blk *blks[PTR_PER_BLK];
};

static void free_blocks(struct ramfile *file, union ramfile_blk **blk,
                        uint64_t off, uint64_t ind_level)
{
	if (!*blk)
		return;
	if (ind_level > 0)
	{
		uint64_t len;
		switch (ind_level)
		{
			case 1:
				len = 0;
				break;
			case 2:
				len = BLK_IND1_LEN;
				break;
			case 3:
				len = BLK_IND2_LEN;
				break;
			case 4:
				len = BLK_IND3_LEN;
				break;
			case 5:
				len = BLK_IND4_LEN;
				break;
			default:
				panic("invalid ind level: %d\n", (int)ind_level);
				return;
		}
		uint64_t sub = len ? off / len : 0;
		for (uint64_t n = sub; n < PTR_PER_BLK; ++n)
			free_blocks(file, &(*blk)->blks[n], len ? off % len : 0, ind_level - 1);
	}
	if (!off)
	{
		file->pages--;
		if (ind_level)
		{
			file->meta_pages--;
			vfree(*blk, PAGE_SIZE);
		}
		else
		{
			pm_free_page(*(struct page**)blk);
			file->data_pages--;
		}
		*blk = NULL;
	}
}

static void rm_page(struct ramfile *file, union ramfile_blk **blk,
                    uint64_t off, int rec)
{
	if (!*blk)
		return;
	switch (rec)
	{
		case 0:
		{
			struct page *page = (struct page*)*blk;
			pm_free_page(page);
			*blk = NULL;
			return;
		}
		case 1:
			rm_page(file, &(*blk)->blks[off], off, 0);
			break;
		case 2:
			rm_page(file, &(*blk)->blks[off / BLK_IND1_LEN],
			        off % BLK_IND1_LEN, 1);
			break;
		case 3:
			rm_page(file, &(*blk)->blks[off / BLK_IND2_LEN],
			        off % BLK_IND2_LEN, 2);
			break;
		case 4:
			rm_page(file, &(*blk)->blks[off / BLK_IND3_LEN],
			        off % BLK_IND3_LEN, 3);
			break;
		case 5:
			rm_page(file, &(*blk)->blks[off / BLK_IND4_LEN],
			        off % BLK_IND4_LEN, 4);
			break;
		default:
			panic("invalid ind level: %d\n", rec);
			break;
	}
}

static struct page *get_page(struct ramfile *file,
                             union ramfile_blk **blk, uint64_t off,
                             uint32_t flags, int rec)
{
	if (!*blk)
	{
		if (!(flags & RAMFILE_ALLOC))
			return NULL;
		if (rec)
		{
			*blk = vmalloc(PAGE_SIZE);
			if (!*blk)
				return NULL;
			memset(*blk, 0, PAGE_SIZE);
			file->meta_pages++;
		}
		else
		{
			int ret = pm_alloc_page((struct page**)blk);
			if (ret)
				return NULL;
			if (flags & RAMFILE_ZERO)
			{
				void *ptr = vm_map(*(struct page**)blk, PAGE_SIZE, VM_PROT_W);
				if (!ptr)
				{
					pm_free_page(*(struct page**)blk);
					return NULL;
				}
				memset(ptr, 0, PAGE_SIZE);
				vm_unmap(ptr, PAGE_SIZE);
			}
			file->data_pages++;
		}
		file->pages++;
	}
	switch (rec)
	{
		case 0:
		{
			struct page *page = (struct page*)*blk;
			pm_ref_page(page);
			return page;
		}
		case 1:
			return get_page(file, &(*blk)->blks[off],
			                off, flags, 0);
		case 2:
			return get_page(file, &(*blk)->blks[off / BLK_IND1_LEN],
			                off % BLK_IND1_LEN, flags, 1);
		case 3:
			return get_page(file, &(*blk)->blks[off / BLK_IND2_LEN],
			                off % BLK_IND2_LEN, flags, 2);
		case 4:
			return get_page(file, &(*blk)->blks[off / BLK_IND3_LEN],
			                off % BLK_IND3_LEN, flags, 3);
		case 5:
			return get_page(file, &(*blk)->blks[off / BLK_IND4_LEN],
			                off % BLK_IND4_LEN, flags, 4);
		default:
			panic("invalid ind level: %d\n", rec);
			break;
	}
	return NULL;
}

void ramfile_init(struct ramfile *file)
{
	memset(file, 0, sizeof(*file));
}

void ramfile_destroy(struct ramfile *file)
{
	for (size_t i = 0; i < sizeof(file->blk) / sizeof(*file->blk); ++i)
	{
		switch (i)
		{
			default:
				free_blocks(file, &file->blk[i], 0, 0);
				break;
			case BLK_IND1_IDX:
				free_blocks(file, &file->blk[i], 0, 1);
				break;
			case BLK_IND2_IDX:
				free_blocks(file, &file->blk[i], 0, 2);
				break;
			case BLK_IND3_IDX:
				free_blocks(file, &file->blk[i], 0, 3);
				break;
			case BLK_IND4_IDX:
				free_blocks(file, &file->blk[i], 0, 4);
				break;
			case BLK_IND5_IDX:
				free_blocks(file, &file->blk[i], 0, 5);
				break;
		}
	}
}

void ramfile_resize(struct ramfile *file, uint64_t size)
{
	if (size >= file->size)
	{
		file->size = size;
		return;
	}
	if (size >= BLK_IND5_OFF)
		free_blocks(file, &file->blk[BLK_IND5_IDX], size - BLK_IND5_OFF, 5);
	if (size >= BLK_IND4_OFF)
		free_blocks(file, &file->blk[BLK_IND4_IDX], size - BLK_IND4_OFF, 4);
	if (size >= BLK_IND3_OFF)
		free_blocks(file, &file->blk[BLK_IND3_IDX], size - BLK_IND3_OFF, 3);
	if (size >= BLK_IND2_OFF)
		free_blocks(file, &file->blk[BLK_IND2_IDX], size - BLK_IND2_OFF, 2);
	if (size >= BLK_IND1_OFF)
		free_blocks(file, &file->blk[BLK_IND1_IDX], size - BLK_IND1_OFF, 1);
	for (size_t i = size; i < BLK_IND1_IDX; ++i)
		free_blocks(file, &file->blk[i], i, 0);
}

void ramfile_rmpage(struct ramfile *file, uint64_t idx)
{
	if (idx > file->size)
		return;
	if (idx < BLK_IND1_OFF)
	{
		rm_page(file, &file->blk[idx], idx, 0);
		return;
	}
	if (idx < BLK_IND2_OFF)
	{
		rm_page(file, &file->blk[BLK_IND1_IDX], idx - BLK_IND1_OFF, 1);
		return;
	}
	if (idx < BLK_IND3_OFF)
	{
		rm_page(file, &file->blk[BLK_IND2_IDX], idx - BLK_IND2_OFF, 2);
		return;
	}
	if (idx < BLK_IND4_OFF)
	{
		rm_page(file, &file->blk[BLK_IND3_IDX], idx - BLK_IND3_OFF, 3);
		return;
	}
	if (idx < BLK_IND5_OFF)
	{
		rm_page(file, &file->blk[BLK_IND4_IDX], idx - BLK_IND4_OFF, 4);
		return;
	}
	if (idx < BLK_IND6_OFF)
	{
		rm_page(file, &file->blk[BLK_IND5_IDX], idx - BLK_IND5_OFF, 5);
		return;
	}
}

struct page *ramfile_getpage(struct ramfile *file, uint64_t idx, uint32_t flags)
{
	if (idx > file->size)
	{
		if (!(flags & RAMFILE_ALLOC))
			return NULL;
		file->size = idx + 1;
	}
	if (idx < BLK_IND1_OFF)
		return get_page(file, &file->blk[idx], idx, flags, 0);
	if (idx < BLK_IND2_OFF)
		return get_page(file, &file->blk[BLK_IND1_IDX],
		                idx - BLK_IND1_OFF, flags, 1);
	if (idx < BLK_IND3_OFF)
		return get_page(file, &file->blk[BLK_IND2_IDX],
		                idx - BLK_IND2_OFF, flags, 2);
	if (idx < BLK_IND4_OFF)
		return get_page(file, &file->blk[BLK_IND3_IDX],
		                idx - BLK_IND3_OFF, flags, 3);
	if (idx < BLK_IND5_OFF)
		return get_page(file, &file->blk[BLK_IND4_IDX],
		                idx - BLK_IND4_OFF, flags, 4);
	if (idx < BLK_IND6_OFF)
		return get_page(file, &file->blk[BLK_IND5_IDX],
		                idx - BLK_IND5_OFF, flags, 5);
	return NULL;
}
