#ifndef VIRGL_H
#define VIRGL_H

#include "virgl_protocol.h"
#include "mesa_pipe.h"
#include "virgl_hw.h"

#include <stddef.h>
#include <stdint.h>

struct env;
struct ctx;
struct res;

struct cmd_buf
{
	uint8_t *data;
	size_t size;
	size_t pos;
};

struct ctx
{
	struct env *env;
	uint32_t object_id;
	uint32_t surface;
	uint32_t ctx;
	uint32_t id;
	struct cmd_buf cmd_buf;
	uint32_t fb;
	int fd;
};

struct res
{
	struct ctx *ctx;
	uint32_t target;
	uint32_t format;
	uint32_t bind;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t array_size;
	uint32_t last_level;
	uint32_t nr_samples;
	uint32_t flags;
	uint32_t bytes;
	int fd;
	uint32_t id;
	void *data;
};

struct env
{
	const char *progname;
	int fd;
	uint32_t capset_id;
	uint32_t capset_version;
	uint32_t capset_size;
	struct virgl_caps_v1 caps_v1;
	struct ctx *ctx;
	struct res *index_buffer;
	struct res *vertex_buffer;
	struct res *uniform_buffer;
};

int vgl_flush(struct env *env);
int vgl_submit(struct ctx *ctx);
int vgl_create_buffer(struct ctx *ctx, enum virgl_formats format,
                      uint32_t bind, size_t bytes);
int vgl_create_context(struct ctx *ctx);
int vgl_get_capset_info(struct env *env, uint32_t *id, uint32_t *version,
                        uint32_t *size);
int vgl_get_capset(struct env *env, void *data, size_t size);
int vgl_create_resource(struct res *res);
int vgl_transfer_out(struct res *res,
                     uint32_t x, uint32_t y, uint32_t z,
                     uint32_t w, uint32_t h, uint32_t d,
                     uint64_t offset, uint32_t level, uint32_t stride,
                     uint32_t layer_stride);

#endif
