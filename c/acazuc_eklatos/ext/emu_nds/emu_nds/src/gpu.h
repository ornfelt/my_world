#ifndef GPU_H
#define GPU_H

#include <stdint.h>

struct mem;

struct gpu_eng
{
	uint8_t *data;
	uint32_t pitch;
	uint8_t  (*get_vram_bg8 )(struct mem *mem, uint32_t addr);
	uint16_t (*get_vram_bg16)(struct mem *mem, uint32_t addr);
	uint32_t (*get_vram_bg32)(struct mem *mem, uint32_t addr);
	uint8_t  (*get_vram_obj8 )(struct mem *mem, uint32_t addr);
	uint16_t (*get_vram_obj16)(struct mem *mem, uint32_t addr);
	uint32_t (*get_vram_obj32)(struct mem *mem, uint32_t addr);
	uint32_t reg_base;
	uint32_t pal_base;
	uint32_t oam_base;
	int32_t bg2x;
	int32_t bg2y;
	int32_t bg3x;
	int32_t bg3y;
	int engb;
};

struct vec4
{
	int32_t x;
	int32_t y;
	int32_t z;
	int32_t w;
};

struct vec3
{
	int32_t x;
	int32_t y;
	int32_t z;
};

struct vec2
{
	int32_t x;
	int32_t y;
};

struct matrix
{
	struct vec4 x;
	struct vec4 y;
	struct vec4 z;
	struct vec4 w;
};

struct vertex
{
	struct vec4 position;
	struct vec3 color;
	struct vec2 texcoord;
	int32_t screen_x;
	int32_t screen_y;
};

struct polygon
{
	uint8_t quad;
	uint32_t attr;
	uint32_t texture;
	uint32_t pltt_base;
	uint16_t vertexes[4];
};

struct gpu_g3d_buf
{
	uint8_t data[256 * 192 * 4];
	int32_t zbuf[256 * 192];
	struct vertex vertexes[6144 * 10]; /* XXX don't do this :D */
	struct polygon polygons[2048 * 10];
	uint16_t vertexes_nb;
	uint16_t polygons_nb;
};

struct light
{
	struct vec3 dir;
	struct vec3 halfdir;
	struct vec3 color;
};

struct gpu_g3d
{
	struct gpu_g3d_buf bufs[2];
	struct gpu_g3d_buf *front;
	struct gpu_g3d_buf *back;
	struct matrix proj_stack[2];
	struct matrix pos_stack[32];
	struct matrix dir_stack[32];
	struct matrix tex_stack[2];
	struct matrix proj_matrix;
	struct matrix pos_matrix;
	struct matrix dir_matrix;
	struct matrix tex_matrix;
	struct matrix clip_matrix;
	uint8_t shininess[128];
	struct light lights[4];
	uint16_t toon[32];
	uint8_t fog[32];
	struct vec3 diffuse;
	struct vec3 ambient;
	struct vec3 specular;
	struct vec3 emission;
	uint8_t specular_table;
	uint8_t matrix_mode;
	uint8_t proj_stack_pos;
	uint8_t pos_stack_pos;
	uint8_t tex_stack_pos;
	struct vec4 position;
	struct vec3 color;
	struct vec2 texcoord;
	uint32_t polygon_attr;
	uint32_t commit_polygon_attr;
	uint8_t primitive;
	uint8_t tmp_vertex;
	uint8_t swap_buffers;
	uint8_t viewport_left;
	uint8_t viewport_right;
	uint8_t viewport_top;
	uint8_t viewport_bottom;
	uint32_t texture;
	uint32_t pltt_base;
};

struct gpu
{
	struct gpu_eng enga;
	struct gpu_eng engb;
	struct gpu_g3d g3d;
	struct mem *mem;
	int capture;
};

struct gpu *gpu_new(struct mem *mem);
void gpu_del(struct gpu *gpu);

void gpu_draw(struct gpu *gpu, uint8_t y);
void gpu_commit_bgpos(struct gpu *gpu);
void gpu_g3d_draw(struct gpu *gpu);
void gpu_g3d_swap_buffers(struct gpu *gpu);

void gpu_gx_cmd(struct gpu *gpu, uint8_t cmd, uint32_t *params);

#endif
