#ifndef GPU_H
#define GPU_H

#include <stdint.h>

#define GPU_WIDTH 160
#define GPU_HEIGHT 144

struct mem;

struct gpu
{
	uint8_t data[144 * 160 * 4];
	uint8_t priorities[160];
	uint8_t hasprinted[160];
	uint8_t sprites[10];
	uint8_t sprites_count;
	uint8_t windowlines;
	uint16_t line_cycle;
	uint8_t x;
	uint8_t y;
	struct mem *mem;
};

struct gpu *gpu_new(struct mem *mem);
void gpu_del(struct gpu *gpu);

void gpu_clock(struct gpu *gpu);

#endif
