#ifndef BLOCKS_H
#define BLOCKS_H

struct block_def;

struct blocks_def
{
	struct block_def *blocks[4096];
};

struct blocks_def *blocks_def_new(void);
void blocks_def_delete(struct blocks_def *blocks);

#endif
