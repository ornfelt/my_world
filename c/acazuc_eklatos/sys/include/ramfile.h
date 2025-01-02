#ifndef RAMFILE_H
#define RAMFILE_H

#include <types.h>

/*
 * 32 bits maximum file size: 4 EB
 * 64 bits maximum file size: 144 PB
 */

#define RAMFILE_ALLOC (1 << 0)
#define RAMFILE_ZERO  (1 << 1)

union ramfile_blk;

struct ramfile
{
	union ramfile_blk *blk[16];
	uint64_t meta_pages;
	uint64_t data_pages;
	uint64_t pages;
	uint64_t size; /* in pages */
};

void ramfile_init(struct ramfile *file);
void ramfile_destroy(struct ramfile *file);
void ramfile_resize(struct ramfile *file, uint64_t size);
void ramfile_rmpage(struct ramfile *file, uint64_t idx);
struct page *ramfile_getpage(struct ramfile *file, uint64_t idx, uint32_t flags);

#endif
