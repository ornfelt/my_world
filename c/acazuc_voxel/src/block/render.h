#ifndef BLOCK_RENDER_H
#define BLOCK_RENDER_H

#include "world/tessellator.h"

#include "shaders.h"
#include "log.h"

#include <stdlib.h>

static inline void add_face_indices(uint32_t *indices, uint32_t current_indice, const uint32_t *idx)
{
	indices[0] = current_indice + idx[0];
	indices[1] = current_indice + idx[1];
	indices[2] = current_indice + idx[2];
	indices[3] = current_indice + idx[3];
	indices[4] = current_indice + idx[4];
	indices[5] = current_indice + idx[5];
}

static inline void add_face_front(struct tessellator *tessellator,
                                  struct vec3f pos_org,
                                  struct vec3f pos_dst,
                                  struct vec2f tex_org,
                                  struct vec2f tex_dst,
                                  struct vec3f *colors,
                                  int flip)
{
	uint32_t current_indice = tessellator->vertexes.size;
	struct shader_blocks_vertex *vertexes = jks_array_grow(&tessellator->vertexes, 4);
	if (!vertexes)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	VEC3_SET(vertexes[0].position, pos_org.x, pos_org.y, pos_dst.z);
	VEC3_SET(vertexes[1].position, pos_org.x, pos_dst.y, pos_dst.z);
	VEC3_SET(vertexes[2].position, pos_dst.x, pos_dst.y, pos_dst.z);
	VEC3_SET(vertexes[3].position, pos_dst.x, pos_org.y, pos_dst.z);
	VEC3_CPY(vertexes[0].color, colors[0]);
	VEC3_CPY(vertexes[1].color, colors[1]);
	VEC3_CPY(vertexes[2].color, colors[2]);
	VEC3_CPY(vertexes[3].color, colors[3]);
	VEC2_SET(vertexes[0].uv, tex_org.x, tex_dst.y);
	VEC2_SET(vertexes[1].uv, tex_org.x, tex_org.y);
	VEC2_SET(vertexes[2].uv, tex_dst.x, tex_org.y);
	VEC2_SET(vertexes[3].uv, tex_dst.x, tex_dst.y);
	uint32_t *indices = jks_array_grow(&tessellator->indices, 6);
	if (!indices)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	if (flip)
	{
		static const uint32_t idx[] = {1, 0, 3, 2, 1, 3};
		add_face_indices(indices, current_indice, idx);
	}
	else
	{
		static const uint32_t idx[] = {2, 0, 3, 2, 1, 0};
		add_face_indices(indices, current_indice, idx);
	}
}

static inline void add_face_back(struct tessellator *tessellator,
                                 struct vec3f pos_org,
                                 struct vec3f pos_dst,
                                 struct vec2f tex_org,
                                 struct vec2f tex_dst,
                                 struct vec3f *colors,
                                 int flip)
{
	uint32_t current_indice = tessellator->vertexes.size;
	struct shader_blocks_vertex *vertexes = jks_array_grow(&tessellator->vertexes, 4);
	if (!vertexes)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	VEC3_SET(vertexes[0].position, pos_org.x, pos_org.y, pos_org.z);
	VEC3_SET(vertexes[1].position, pos_org.x, pos_dst.y, pos_org.z);
	VEC3_SET(vertexes[2].position, pos_dst.x, pos_dst.y, pos_org.z);
	VEC3_SET(vertexes[3].position, pos_dst.x, pos_org.y, pos_org.z);
	VEC3_CPY(vertexes[0].color, colors[0]);
	VEC3_CPY(vertexes[1].color, colors[1]);
	VEC3_CPY(vertexes[2].color, colors[2]);
	VEC3_CPY(vertexes[3].color, colors[3]);
	VEC2_SET(vertexes[0].uv, tex_dst.x, tex_dst.y);
	VEC2_SET(vertexes[1].uv, tex_dst.x, tex_org.y);
	VEC2_SET(vertexes[2].uv, tex_org.x, tex_org.y);
	VEC2_SET(vertexes[3].uv, tex_org.x, tex_dst.y);
	uint32_t *indices = jks_array_grow(&tessellator->indices, 6);
	if (!indices)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	if (flip)
	{
		static const uint32_t idx[] = {2, 3, 0, 1, 2, 0};
		add_face_indices(indices, current_indice, idx);
	}
	else
	{
		static const uint32_t idx[] = {1, 3, 0, 1, 2, 3};
		add_face_indices(indices, current_indice, idx);
	}
}

static inline void add_face_left(struct tessellator *tessellator,
                                 struct vec3f pos_org,
                                 struct vec3f pos_dst,
                                 struct vec2f tex_org,
                                 struct vec2f tex_dst,
                                 struct vec3f *colors,
                                 int flip)
{
	uint32_t current_indice = tessellator->vertexes.size;
	struct shader_blocks_vertex *vertexes = jks_array_grow(&tessellator->vertexes, 4);
	if (!vertexes)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	VEC3_SET(vertexes[0].position, pos_org.x, pos_org.y, pos_org.z);
	VEC3_SET(vertexes[1].position, pos_org.x, pos_dst.y, pos_org.z);
	VEC3_SET(vertexes[2].position, pos_org.x, pos_dst.y, pos_dst.z);
	VEC3_SET(vertexes[3].position, pos_org.x, pos_org.y, pos_dst.z);
	VEC3_CPY(vertexes[0].color, colors[0]);
	VEC3_CPY(vertexes[1].color, colors[1]);
	VEC3_CPY(vertexes[2].color, colors[2]);
	VEC3_CPY(vertexes[3].color, colors[3]);
	VEC2_SET(vertexes[0].uv, tex_org.x, tex_dst.y);
	VEC2_SET(vertexes[1].uv, tex_org.x, tex_org.y);
	VEC2_SET(vertexes[2].uv, tex_dst.x, tex_org.y);
	VEC2_SET(vertexes[3].uv, tex_dst.x, tex_dst.y);
	uint32_t *indices = jks_array_grow(&tessellator->indices, 6);
	if (!indices)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	if (flip)
	{
		static const uint32_t idx[] = {1, 0, 3, 2, 1, 3};
		add_face_indices(indices, current_indice, idx);
	}
	else
	{
		static const uint32_t idx[] = {2, 0, 3, 2, 1, 0};
		add_face_indices(indices, current_indice, idx);
	}
}

static inline void add_face_right(struct tessellator *tessellator,
                                  struct vec3f pos_org,
                                  struct vec3f pos_dst,
                                  struct vec2f tex_org,
                                  struct vec2f tex_dst,
                                  struct vec3f *colors,
                                  int flip)
{
	uint32_t current_indice = tessellator->vertexes.size;
	struct shader_blocks_vertex *vertexes = jks_array_grow(&tessellator->vertexes, 4);
	if (!vertexes)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	VEC3_SET(vertexes[0].position, pos_dst.x, pos_org.y, pos_org.z);
	VEC3_SET(vertexes[1].position, pos_dst.x, pos_dst.y, pos_org.z);
	VEC3_SET(vertexes[2].position, pos_dst.x, pos_dst.y, pos_dst.z);
	VEC3_SET(vertexes[3].position, pos_dst.x, pos_org.y, pos_dst.z);
	VEC3_CPY(vertexes[0].color, colors[0]);
	VEC3_CPY(vertexes[1].color, colors[1]);
	VEC3_CPY(vertexes[2].color, colors[2]);
	VEC3_CPY(vertexes[3].color, colors[3]);
	VEC2_SET(vertexes[0].uv, tex_dst.x, tex_dst.y);
	VEC2_SET(vertexes[1].uv, tex_dst.x, tex_org.y);
	VEC2_SET(vertexes[2].uv, tex_org.x, tex_org.y);
	VEC2_SET(vertexes[3].uv, tex_org.x, tex_dst.y);
	uint32_t *indices = jks_array_grow(&tessellator->indices, 6);
	if (!indices)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	if (flip)
	{
		static const uint32_t idx[] = {2, 3, 0, 1, 2, 0};
		add_face_indices(indices, current_indice, idx);
	}
	else
	{
		static const uint32_t idx[] = {1, 3, 0, 1, 2, 3};
		add_face_indices(indices, current_indice, idx);
	}
}

static inline void add_face_up(struct tessellator *tessellator,
                               struct vec3f pos_org,
                               struct vec3f pos_dst,
                               struct vec2f tex_org,
                               struct vec2f tex_dst,
                               struct vec3f *colors,
                               int flip)
{
	uint32_t current_indice = tessellator->vertexes.size;
	struct shader_blocks_vertex *vertexes = jks_array_grow(&tessellator->vertexes, 4);
	if (!vertexes)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	VEC3_SET(vertexes[0].position, pos_org.x, pos_dst.y, pos_dst.z);
	VEC3_SET(vertexes[1].position, pos_org.x, pos_dst.y, pos_org.z);
	VEC3_SET(vertexes[2].position, pos_dst.x, pos_dst.y, pos_org.z);
	VEC3_SET(vertexes[3].position, pos_dst.x, pos_dst.y, pos_dst.z);
	VEC3_CPY(vertexes[0].color, colors[0]);
	VEC3_CPY(vertexes[1].color, colors[1]);
	VEC3_CPY(vertexes[2].color, colors[2]);
	VEC3_CPY(vertexes[3].color, colors[3]);
	VEC2_SET(vertexes[0].uv, tex_org.x, tex_org.y);
	VEC2_SET(vertexes[1].uv, tex_org.x, tex_dst.y);
	VEC2_SET(vertexes[2].uv, tex_dst.x, tex_dst.y);
	VEC2_SET(vertexes[3].uv, tex_dst.x, tex_org.y);
	uint32_t *indices = jks_array_grow(&tessellator->indices, 6);
	if (!indices)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	if (flip)
	{
		static const uint32_t idx[] = {1, 0, 3, 2, 1, 3};
		add_face_indices(indices, current_indice, idx);
	}
	else
	{
		static const uint32_t idx[] = {2, 0, 3, 2, 1, 0};
		add_face_indices(indices, current_indice, idx);
	}
}

static inline void add_face_down(struct tessellator *tessellator,
                                 struct vec3f pos_org,
                                 struct vec3f pos_dst,
                                 struct vec2f tex_org,
                                 struct vec2f tex_dst,
                                 struct vec3f *colors,
                                 int flip)
{
	uint32_t current_indice = tessellator->vertexes.size;
	struct shader_blocks_vertex *vertexes = jks_array_grow(&tessellator->vertexes, 4);
	if (!vertexes)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	VEC3_SET(vertexes[0].position, pos_org.x, pos_org.y, pos_dst.z);
	VEC3_SET(vertexes[1].position, pos_org.x, pos_org.y, pos_org.z);
	VEC3_SET(vertexes[2].position, pos_dst.x, pos_org.y, pos_org.z);
	VEC3_SET(vertexes[3].position, pos_dst.x, pos_org.y, pos_dst.z);
	VEC3_CPY(vertexes[0].color, colors[0]);
	VEC3_CPY(vertexes[1].color, colors[1]);
	VEC3_CPY(vertexes[2].color, colors[2]);
	VEC3_CPY(vertexes[3].color, colors[3]);
	VEC2_SET(vertexes[0].uv, tex_org.x, tex_org.y);
	VEC2_SET(vertexes[1].uv, tex_org.x, tex_dst.y);
	VEC2_SET(vertexes[2].uv, tex_dst.x, tex_dst.y);
	VEC2_SET(vertexes[3].uv, tex_dst.x, tex_org.y);
	uint32_t *indices = jks_array_grow(&tessellator->indices, 6);
	if (!indices)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	if (flip)
	{
		static const uint32_t idx[] = {0, 1, 2, 3, 0, 2};
		add_face_indices(indices, current_indice, idx);
	}
	else
	{
		static const uint32_t idx[] = {3, 1, 2, 3, 0, 1};
		add_face_indices(indices, current_indice, idx);
	}
}

static inline void add_face_diag(struct tessellator *tessellator,
                                 struct vec3f pos_org,
                                 struct vec3f pos_dst,
                                 struct vec2f tex_org,
                                 struct vec2f tex_dst,
                                 struct vec3f *colors)
{
	uint32_t current_indice = tessellator->vertexes.size;
	struct shader_blocks_vertex *vertexes = jks_array_grow(&tessellator->vertexes, 4);
	if (!vertexes)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	VEC3_SET(vertexes[0].position, pos_org.x, pos_org.y, pos_org.z);
	VEC3_SET(vertexes[1].position, pos_org.x, pos_dst.y, pos_org.z);
	VEC3_SET(vertexes[2].position, pos_dst.x, pos_dst.y, pos_dst.z);
	VEC3_SET(vertexes[3].position, pos_dst.x, pos_org.y, pos_dst.z);
	VEC3_CPY(vertexes[0].color, colors[0]);
	VEC3_CPY(vertexes[1].color, colors[1]);
	VEC3_CPY(vertexes[2].color, colors[2]);
	VEC3_CPY(vertexes[3].color, colors[3]);
	VEC2_SET(vertexes[0].uv, tex_org.x, tex_dst.y);
	VEC2_SET(vertexes[1].uv, tex_org.x, tex_org.y);
	VEC2_SET(vertexes[2].uv, tex_dst.x, tex_org.y);
	VEC2_SET(vertexes[3].uv, tex_dst.x, tex_dst.y);
	uint32_t *indices = jks_array_grow(&tessellator->indices, 12);
	if (!indices)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	{
		static const uint32_t idx[] = {1, 0, 3, 2, 1, 3};
		add_face_indices(&indices[0], current_indice, idx);
	}
	{
		static const uint32_t idx[] = {3, 0, 1, 3, 1, 2};
		add_face_indices(&indices[6], current_indice, idx);
	}
}

#endif
