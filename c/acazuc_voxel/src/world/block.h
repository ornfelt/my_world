#ifndef BLOCK_H
#define BLOCK_H

#include <jks/vec3.h>

#include <stdint.h>

#define BLOCK_SIZE 1

#define FRONT_COLOR_FACTOR  0.787
#define BACK_COLOR_FACTOR   0.787
#define LEFT_COLOR_FACTOR   0.590
#define RIGHT_COLOR_FACTOR  0.590
#define TOP_COLOR_FACTOR    0.985
#define BOTTOM_COLOR_FACTOR 0.496

struct tessellator;
struct chunk;

enum block_faces
{
	BLOCK_FACE_FRONT  = 0x01,
	BLOCK_FACE_BACK   = 0x02,
	BLOCK_FACE_LEFT   = 0x04,
	BLOCK_FACE_RIGHT  = 0x08,
	BLOCK_FACE_TOP    = 0x10,
	BLOCK_FACE_BOTTOM = 0x20,
};

enum block_face_points
{
	F1P1,
	F1P2,
	F1P3,
	F1P4,
	F2P1,
	F2P2,
	F2P3,
	F2P4,
	F3P1,
	F3P2,
	F3P3,
	F3P4,
	F4P1,
	F4P2,
	F4P3,
	F4P4,
	F5P1,
	F5P2,
	F5P3,
	F5P4,
	F6P1,
	F6P2,
	F6P3,
	F6P4,
};

struct block_lights_levels
{
	uint8_t f1p1 : 4;
	uint8_t f1p2 : 4;
	uint8_t f1p3 : 4;
	uint8_t f1p4 : 4;
	uint8_t f2p1 : 4;
	uint8_t f2p2 : 4;
	uint8_t f2p3 : 4;
	uint8_t f2p4 : 4;
	uint8_t f3p1 : 4;
	uint8_t f3p2 : 4;
	uint8_t f3p3 : 4;
	uint8_t f3p4 : 4;
	uint8_t f4p1 : 4;
	uint8_t f4p2 : 4;
	uint8_t f4p3 : 4;
	uint8_t f4p4 : 4;
	uint8_t f5p1 : 4;
	uint8_t f5p2 : 4;
	uint8_t f5p3 : 4;
	uint8_t f5p4 : 4;
	uint8_t f6p1 : 4;
	uint8_t f6p2 : 4;
	uint8_t f6p3 : 4;
	uint8_t f6p4 : 4;
};

struct block
{
	uint16_t type;
	uint8_t data;
};

void block_fill_buffers(struct block block, struct chunk *chunk, struct vec3f pos, struct tessellator *tessellator, uint8_t layer);

#endif
