#ifndef WOW_COMMON_H
#define WOW_COMMON_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct wow_memory
{
	void *(*malloc)(size_t size);
	void *(*realloc)(void *ptr, size_t size);
	void (*free)(void *ptr);
};

#define WOW_MALLOC(size) (wow_memory.malloc ? wow_memory.malloc(size) : malloc(size))
#define WOW_REALLOC(ptr, size) (wow_memory.realloc ? wow_memory.realloc(ptr, size) : realloc(ptr, size))
#define WOW_FREE(ptr) (wow_memory.free ? wow_memory.free(ptr) : free(ptr))

extern struct wow_memory wow_memory;

struct wow_vec2f
{
	float x;
	float y;
};

struct wow_vec3f
{
	float x;
	float y;
	float z;
};

struct wow_vec3s
{
	int16_t x;
	int16_t y;
	int16_t z;
};

struct wow_vec4f
{
	float x;
	float y;
	float z;
	float w;
};

struct wow_vec4b
{
	uint8_t x;
	uint8_t y;
	uint8_t z;
	uint8_t w;
};

struct wow_quatf
{
	float x;
	float y;
	float z;
	float w;
};

struct wow_quats
{
	int16_t x;
	int16_t y;
	int16_t z;
	int16_t w;
};

struct wow_chunk_header
{
	uint32_t magic;
	uint32_t size;
};

struct wow_mver
{
	struct wow_chunk_header header;
	uint32_t version;
};

struct wow_mwmo
{
	struct wow_chunk_header header;
	char *data;
	uint32_t size;
};

#define WOW_MODF_FLAGS_DESTROYABLE 0x1
#define WOW_MODF_FLAGS_LOD         0x2
#define WOW_MODF_FLAGS_UNKNOWN     0x4
#define WOW_MODF_FLAGS_FILEDATA    0x8

struct wow_modf_data
{
	uint32_t name_id;
	uint32_t unique_id;
	struct wow_vec3f position;
	struct wow_vec3f rotation;
	struct wow_vec3f aabb0;
	struct wow_vec3f aabb1;
	uint16_t flags;
	uint16_t doodad_set;
	uint16_t name_set;
	uint16_t unk;
};

struct wow_modf
{
	struct wow_chunk_header header;
	struct wow_modf_data *data;
	uint32_t size;
};

#ifdef __cplusplus
}
#endif

#endif
